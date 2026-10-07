// jk-charge-gui: see batterymonitor.h.
#include "batterymonitor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <climits>
#include <cmath>

namespace {

const qreal NaN = std::nan("");

// The POWER_SUPPLY_* lines of a supply's uevent, keys without the prefix.
QHash<QString, QString> readUevent(const QString &dir)
{
    QHash<QString, QString> m;
    QFile f(dir + "/uevent");
    if (f.open(QIODevice::ReadOnly)) {
        for (const QByteArray &line : f.readAll().split('\n')) {
            if (!line.startsWith("POWER_SUPPLY_"))
                continue;
            const qsizetype eq = line.indexOf('=');
            if (eq > 0)
                m.insert(QString::fromUtf8(line.mid(13, eq - 13)), QString::fromUtf8(line.mid(eq + 1)).trimmed());
        }
    }
    // One failed ADC property can make the kernel's whole uevent read fail.
    // Read the individual attributes then, so status and the other live
    // values keep updating while a charger switches paths.
    static const char *const attrs[] = {
        "type", "scope", "online", "status", "manufacturer", "model_name",
        "technology", "health", "capacity", "cycle_count", "temp",
        "charge_control_start_threshold", "charge_control_end_threshold",
        "voltage_now", "voltage_ocv", "current_now", "current_avg", "power_now",
        "energy_full_design", "energy_full", "energy_now",
        "charge_full_design", "charge_full", "charge_now", "charge_type",
        "input_current_limit", "constant_charge_current",
        "constant_charge_current_max", "constant_charge_voltage"
    };
    for (const char *attr : attrs) {
        const QString key = QString::fromLatin1(attr).toUpper();
        if (m.contains(key))
            continue;
        QFile value(dir + "/" + QString::fromLatin1(attr));
        if (value.open(QIODevice::ReadOnly))
            m.insert(key, QString::fromUtf8(value.readAll()).trimmed());
    }
    return m;
}

// A value in micro-units as a plain one (µA -> A), or NaN.
qreal micro(const QHash<QString, QString> &m, const char *key)
{
    bool ok = false;
    const qreal v = m.value(key).toDouble(&ok);
    return ok ? v / 1e6 : NaN;
}

int integer(const QHash<QString, QString> &m, const char *key, int fallback = -1)
{
    bool ok = false;
    const int v = m.value(key).toInt(&ok);
    return ok ? v : fallback;
}

qreal average(const QList<qreal> &l, int n)
{
    qreal sum = 0;
    int count = 0;
    for (qsizetype i = l.size() - 1; i >= 0 && count < n; i--, count++)
        sum += l[i];
    return count ? sum / count : NaN;
}

QVariantList toVariant(const QList<qreal> &l)
{
    QVariantList v;
    v.reserve(l.size());
    for (qreal x : l)
        v.append(x);
    return v;
}

} // namespace

BatteryMonitor::BatteryMonitor(QObject *parent) : QObject(parent)
{
    // JK_POWER_SUPPLY_DIR: another tree laid out like /sys/class/power_supply
    // (tests).
    root_ = qEnvironmentVariable("JK_POWER_SUPPLY_DIR", "/sys/class/power_supply");
    reset();
    connect(&timer_, &QTimer::timeout, this, &BatteryMonitor::poll);
    timer_.start(1000);
    QTimer::singleShot(0, this, &BatteryMonitor::poll);
}

void BatteryMonitor::reset()
{
    voltage_ = voltageOcv_ = current_ = currentAvg_ = power_ = powerAvg_ = temperature_ = NaN;
    designFull_ = full_ = now_ = healthPercent_ = ratePerHour_ = NaN;
    inputLimit_ = chargeCurrentSet_ = chargeCurrentMax_ = chargeVoltageSet_ = NaN;
    directVoltage_ = directCurrent_ = directPower_ = directTemperature_ = directInputLimit_ = NaN;
}

// The system battery (not a mouse's or a headset's: scope Device), and the
// charger: the one online, else the first one.
void BatteryMonitor::findSupplies()
{
    batteryDir_.clear();
    chargerDir_.clear();
    QString firstCharger;
    const QStringList names = QDir(root_).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &n : names) {
        const QString dir = root_ + "/" + n;
        const auto m = readUevent(dir);
        const QString type = m.value("TYPE");
        if (type == "Battery") {
            if (batteryDir_.isEmpty() && m.value("SCOPE") != "Device")
                batteryDir_ = dir;
        } else if (type == "Mains" || type.startsWith("USB")) {
            if (firstCharger.isEmpty())
                firstCharger = dir;
            if (chargerDir_.isEmpty() && m.value("ONLINE") == "1")
                chargerDir_ = dir;
        }
    }
    if (chargerDir_.isEmpty())
        chargerDir_ = firstCharger;
}

void BatteryMonitor::poll()
{
    findSupplies();
    reset();
    if (batteryDir_.isEmpty()) {
        powerHist_.clear();
        directPowerHist_.clear();
        capacityHist_.clear();
        flowHist_.clear();
        emit updated();
        return;
    }

    const auto b = readUevent(batteryDir_);
    const auto direct = readUevent(root_ + "/sm5440-direct");
    directActive_ = direct.value("ONLINE") == "1";
    if (directActive_) {
        directVoltage_ = micro(direct, "VOLTAGE_NOW");
        directCurrent_ = micro(direct, "CURRENT_NOW");
        directInputLimit_ = micro(direct, "INPUT_CURRENT_LIMIT");
        const int dieTemp = integer(direct, "TEMP", INT_MIN);
        directTemperature_ = dieTemp == INT_MIN ? NaN : dieTemp / 10.0;
        if (!std::isnan(directVoltage_) && !std::isnan(directCurrent_))
            directPower_ = directVoltage_ * directCurrent_;
    }
    batteryName_ = QFileInfo(batteryDir_).fileName();
    model_ = (b.value("MANUFACTURER") + " " + b.value("MODEL_NAME")).trimmed();
    status_ = b.value("STATUS", "Unknown");
    technology_ = b.value("TECHNOLOGY");
    health_ = b.value("HEALTH");
    capacity_ = integer(b, "CAPACITY");
    cycleCount_ = integer(b, "CYCLE_COUNT");
    careStart_ = integer(b, "CHARGE_CONTROL_START_THRESHOLD");
    careEnd_ = integer(b, "CHARGE_CONTROL_END_THRESHOLD");
    voltage_ = micro(b, "VOLTAGE_NOW");
    voltageOcv_ = micro(b, "VOLTAGE_OCV");
    const int t = integer(b, "TEMP", INT_MIN);
    temperature_ = t == INT_MIN ? NaN : t / 10.0;

    // Signed flow: drivers like the SM5714's report a negative current while
    // discharging; ACPI reports magnitudes, and the status gives the sign.
    const bool discharging = status_ == "Discharging";
    auto sign = [discharging](qreal v) { return (discharging && v > 0) ? -v : v; };
    current_ = sign(micro(b, "CURRENT_NOW"));
    currentAvg_ = sign(micro(b, "CURRENT_AVG"));
    power_ = sign(micro(b, "POWER_NOW"));
    if (std::isnan(power_) && !std::isnan(current_) && !std::isnan(voltage_))
        power_ = current_ * voltage_;
    if (std::isnan(current_) && !std::isnan(power_) && voltage_ > 0)
        current_ = power_ / voltage_;

    energyUnits_ = b.contains("ENERGY_FULL_DESIGN") || b.contains("ENERGY_NOW");
    const char *pre = energyUnits_ ? "ENERGY_" : "CHARGE_";
    designFull_ = micro(b, QByteArray(pre).append("FULL_DESIGN").constData());
    full_ = micro(b, QByteArray(pre).append("FULL").constData());
    now_ = micro(b, QByteArray(pre).append("NOW").constData());
    if (!std::isnan(full_) && designFull_ > 0)
        healthPercent_ = full_ / designFull_ * 100;

    // History and averages (power in W; the flow in the battery's own unit).
    const qreal flow = energyUnits_ ? power_ : current_;
    auto push = [](QList<qreal> &l, qreal v) {
        l.append(v);
        if (l.size() > kHistory)
            l.removeFirst();
    };
    push(powerHist_, std::isnan(power_) ? 0 : power_);
    push(directPowerHist_, directActive_ && !std::isnan(directPower_) ? directPower_ : 0);
    push(capacityHist_, capacity_ < 0 ? 0 : capacity_);
    if (!std::isnan(flow))
        push(flowHist_, flow);
    powerAvg_ = average(powerHist_, kAverage);
    const qreal flowAvg = average(flowHist_, kAverage);

    // %/h and time to full (or to the battery care limit) or to empty.
    const qreal size = full_ > 0 ? full_ : designFull_;
    if (size > 0 && !std::isnan(flowAvg)) {
        ratePerHour_ = flowAvg / size * 100;
        const qreal have = !std::isnan(now_) ? now_ : size * qMax(capacity_, 0) / 100.0;
        const int target = (careEnd_ > 0 && careEnd_ < 100) ? careEnd_ : 100;
        qreal hours = -1;
        if (flowAvg > 0.005 * size && capacity_ < target)
            hours = (size * target / 100.0 - have) / flowAvg;
        else if (flowAvg < -0.005 * size)
            hours = have / -flowAvg;
        if (hours >= 0 && hours < 1000)
            secondsLeft_ = int(hours * 3600);
        else
            secondsLeft_ = -1;
    } else {
        secondsLeft_ = -1;
    }

    chargerPresent_ = !chargerDir_.isEmpty();
    chargerOnline_ = false;
    chargerName_.clear();
    chargerHealth_.clear();
    chargeType_.clear();
    if (chargerPresent_) {
        const auto c = readUevent(chargerDir_);
        chargerName_ = QFileInfo(chargerDir_).fileName();
        chargerOnline_ = c.value("ONLINE") == "1";
        chargerHealth_ = c.value("HEALTH");
        chargeType_ = c.value("CHARGE_TYPE");
        inputLimit_ = micro(c, "INPUT_CURRENT_LIMIT");
        chargeCurrentSet_ = micro(c, "CONSTANT_CHARGE_CURRENT");
        chargeCurrentMax_ = micro(c, "CONSTANT_CHARGE_CURRENT_MAX");
        chargeVoltageSet_ = micro(c, "CONSTANT_CHARGE_VOLTAGE");
    }
    emit updated();
}

QVariantList BatteryMonitor::powerHistory() const { return toVariant(powerHist_); }
QVariantList BatteryMonitor::directPowerHistory() const { return toVariant(directPowerHist_); }
QVariantList BatteryMonitor::capacityHistory() const { return toVariant(capacityHist_); }
