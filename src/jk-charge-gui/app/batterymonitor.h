// jk-charge-gui: the battery and its charger, read from the kernel's
// power supplies (/sys/class/power_supply) every second.
//
// Works with the two ways drivers report a battery: by charge (µAh, µA, as
// the tablet's SM5714 does) or by energy (µWh, µW, as laptops' ACPI does).
// Current and power are signed here: positive into the battery, negative
// out of it.
#pragma once
#include <QHash>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class BatteryMonitor : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool available READ available NOTIFY updated)
    Q_PROPERTY(QString batteryName MEMBER batteryName_ NOTIFY updated)
    Q_PROPERTY(QString model MEMBER model_ NOTIFY updated)
    Q_PROPERTY(QString status MEMBER status_ NOTIFY updated)
    Q_PROPERTY(QString technology MEMBER technology_ NOTIFY updated)
    Q_PROPERTY(QString health MEMBER health_ NOTIFY updated)
    Q_PROPERTY(int capacity MEMBER capacity_ NOTIFY updated)
    Q_PROPERTY(qreal voltage MEMBER voltage_ NOTIFY updated)
    Q_PROPERTY(qreal voltageOcv MEMBER voltageOcv_ NOTIFY updated)
    Q_PROPERTY(qreal current MEMBER current_ NOTIFY updated)
    Q_PROPERTY(qreal currentAvg MEMBER currentAvg_ NOTIFY updated)
    Q_PROPERTY(qreal power MEMBER power_ NOTIFY updated)
    Q_PROPERTY(qreal powerAvg MEMBER powerAvg_ NOTIFY updated)
    Q_PROPERTY(qreal temperature MEMBER temperature_ NOTIFY updated)
    Q_PROPERTY(bool energyUnits MEMBER energyUnits_ NOTIFY updated)
    Q_PROPERTY(qreal designFull MEMBER designFull_ NOTIFY updated)
    Q_PROPERTY(qreal full MEMBER full_ NOTIFY updated)
    Q_PROPERTY(qreal now MEMBER now_ NOTIFY updated)
    Q_PROPERTY(qreal healthPercent MEMBER healthPercent_ NOTIFY updated)
    Q_PROPERTY(int cycleCount MEMBER cycleCount_ NOTIFY updated)
    Q_PROPERTY(qreal ratePerHour MEMBER ratePerHour_ NOTIFY updated)
    Q_PROPERTY(int secondsLeft MEMBER secondsLeft_ NOTIFY updated)
    Q_PROPERTY(int careStart MEMBER careStart_ NOTIFY updated)
    Q_PROPERTY(int careEnd MEMBER careEnd_ NOTIFY updated)
    Q_PROPERTY(bool chargerPresent MEMBER chargerPresent_ NOTIFY updated)
    Q_PROPERTY(bool chargerOnline MEMBER chargerOnline_ NOTIFY updated)
    Q_PROPERTY(bool directActive MEMBER directActive_ NOTIFY updated)
    Q_PROPERTY(QString chargerName MEMBER chargerName_ NOTIFY updated)
    Q_PROPERTY(QString chargerHealth MEMBER chargerHealth_ NOTIFY updated)
    Q_PROPERTY(QString chargeType MEMBER chargeType_ NOTIFY updated)
    Q_PROPERTY(qreal inputLimit MEMBER inputLimit_ NOTIFY updated)
    Q_PROPERTY(qreal chargeCurrentSet MEMBER chargeCurrentSet_ NOTIFY updated)
    Q_PROPERTY(qreal chargeCurrentMax MEMBER chargeCurrentMax_ NOTIFY updated)
    Q_PROPERTY(qreal chargeVoltageSet MEMBER chargeVoltageSet_ NOTIFY updated)
    Q_PROPERTY(QVariantList powerHistory READ powerHistory NOTIFY updated)
    Q_PROPERTY(QVariantList capacityHistory READ capacityHistory NOTIFY updated)
    Q_PROPERTY(int historySeconds READ historySeconds CONSTANT)

public:
    explicit BatteryMonitor(QObject *parent = nullptr);
    bool available() const { return !batteryDir_.isEmpty(); }
    QVariantList powerHistory() const;
    QVariantList capacityHistory() const;
    int historySeconds() const { return kHistory; }

signals:
    void updated();

private:
    static constexpr int kHistory = 3600;   // samples, one per second
    static constexpr int kAverage = 30;     // seconds, for the rate and time left

    QString root_, batteryDir_, chargerDir_;
    QTimer timer_;

    QString batteryName_, model_, status_, technology_, health_;
    int capacity_ = -1, cycleCount_ = -1, secondsLeft_ = -1, careStart_ = -1, careEnd_ = -1;
    qreal voltage_, voltageOcv_, current_, currentAvg_, power_, powerAvg_, temperature_;
    bool energyUnits_ = false;
    qreal designFull_, full_, now_, healthPercent_, ratePerHour_;
    bool chargerPresent_ = false, chargerOnline_ = false, directActive_ = false;
    QString chargerName_, chargerHealth_, chargeType_;
    qreal inputLimit_, chargeCurrentSet_, chargeCurrentMax_, chargeVoltageSet_;

    QList<qreal> powerHist_, capacityHist_, flowHist_;   // flow: A or W, for the averages

    void findSupplies();
    void poll();
    void reset();
};
