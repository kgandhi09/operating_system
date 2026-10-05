// jk-timedated: see timedate.h.
#include "timedate.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <grp.h>
#include <pwd.h>
#include <sys/timex.h>
#include <unistd.h>
#include <vector>

namespace {

const char *kPath = "/org/freedesktop/timedate1";
const char *kIface = "org.freedesktop.timedate1";
const char *kConfig = "/etc/jk_os/time";
const char *kSavedClock = "/var/lib/jk_os/clock";
const char *kZoneinfo = "/usr/share/zoneinfo";
const char *kNtpd = "/usr/sbin/ntpd";
const char *kHwclock = "/sbin/hwclock";
const QStringList kDefaultServers = {"0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org", "3.pool.ntp.org"};

qint64 nowUsec()
{
    timespec ts {};
    clock_gettime(CLOCK_REALTIME, &ts);
    return qint64(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
}

bool setClockUsec(qint64 usec)
{
    timespec ts {};
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;
    if (clock_settime(CLOCK_REALTIME, &ts) == 0)
        return true;
    qWarning("cannot set the clock: %s", strerror(errno));
    return false;
}

} // namespace

QDBusConnection timedateBus()
{
    // JK_TIMEDATED_BUS=session: on the session bus (tests).
    return qEnvironmentVariable("JK_TIMEDATED_BUS") == "session" ? QDBusConnection::sessionBus()
                                                                  : QDBusConnection::systemBus();
}

TimeDate::TimeDate(QObject *parent) : QObject(parent)
{
    root_ = qEnvironmentVariable("JK_TIMEDATED_ROOT");   // (tests)
    loadConfig();

    saveTimer_.setInterval(15 * 60 * 1000);
    connect(&saveTimer_, &QTimer::timeout, this, &TimeDate::saveClock);
    saveTimer_.start();

    restartNtp_.setSingleShot(true);
    restartNtp_.setInterval(30 * 1000);
    connect(&restartNtp_, &QTimer::timeout, this, &TimeDate::startNtp);
    ntpd_.setProcessChannelMode(QProcess::ForwardedChannels);
    connect(&ntpd_, &QProcess::finished, this, [this](int code) {
        if (stopping_ || !ntp_)
            return;
        qWarning("ntpd exited (%d), restarting it in 30 s", code);
        restartNtp_.start();
    });
}

TimeDate::~TimeDate()
{
    stopNtp();
}

// /etc/jk_os/time: NTP=yes|no, NTP_SERVERS="host ...", RTC_LOCAL=yes|no.
void TimeDate::loadConfig()
{
    ntp_ = true;
    localRtc_ = false;
    servers_ = kDefaultServers;
    QFile f(path(kConfig));
    if (!f.open(QIODevice::ReadOnly))
        return;
    for (const QByteArray &raw : f.readAll().split('\n')) {
        const QString line = QString::fromUtf8(raw).trimmed();
        const qsizetype eq = line.indexOf('=');
        if (line.startsWith('#') || eq <= 0)
            continue;
        const QString key = line.left(eq);
        QString value = line.mid(eq + 1).trimmed();
        if (value.size() >= 2 && (value.startsWith('"') || value.startsWith('\'')))
            value = value.mid(1, value.size() - 2);
        if (key == "NTP")
            ntp_ = value != "no";
        else if (key == "RTC_LOCAL")
            localRtc_ = value == "yes";
        else if (key == "NTP_SERVERS" && !value.isEmpty())
            servers_ = value.split(' ', Qt::SkipEmptyParts);
    }
}

bool TimeDate::saveConfig() const
{
    QDir().mkpath(QFileInfo(path(kConfig)).path());
    QFile f(path(kConfig));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    f.write(QStringLiteral("# The clock (jk-timedated; KDE's Date & Time settings change it).\n"
                           "NTP=%1\nNTP_SERVERS=\"%2\"\nRTC_LOCAL=%3\n")
                .arg(ntp_ ? "yes" : "no", servers_.join(' '), localRtc_ ? "yes" : "no")
                .toUtf8());
    return f.flush();
}

// Root and the administrators (group wheel, as sudo has them) may change the
// clock; anyone may read it.
bool TimeDate::authorized()
{
    if (!calledFromDBus())
        return true;
    const auto uidReply = connection().interface()->serviceUid(message().service());
    if (uidReply.isValid()) {
        const uint uid = uidReply.value();
        if (uid == 0)
            return true;
        const passwd *pw = getpwuid(uid);
        // JK_TIMEDATED_GROUP: another group than wheel (tests).
        const group *wheel = getgrnam(qEnvironmentVariable("JK_TIMEDATED_GROUP", "wheel").toLocal8Bit().constData());
        if (pw && wheel) {
            int n = 32;
            std::vector<gid_t> groups(n);
            if (getgrouplist(pw->pw_name, pw->pw_gid, groups.data(), &n) < 0) {
                groups.resize(n);
                getgrouplist(pw->pw_name, pw->pw_gid, groups.data(), &n);
            }
            if (std::find(groups.begin(), groups.begin() + n, wheel->gr_gid) != groups.begin() + n)
                return true;
        }
    }
    sendErrorReply(QDBusError::AccessDenied, "Only administrators (group wheel) can change the system clock");
    return false;
}

void TimeDate::changed(const QVariantMap &props)
{
    QDBusMessage sig = QDBusMessage::createSignal(kPath, "org.freedesktop.DBus.Properties", "PropertiesChanged");
    sig << QString(kIface) << props << QStringList();
    timedateBus().send(sig);
}

// ---- properties ----

QString TimeDate::timezone() const
{
    const QString target = QFileInfo(path("/etc/localtime")).symLinkTarget();
    const qsizetype at = target.indexOf("/zoneinfo/");
    if (at >= 0)
        return target.mid(at + 10);
    QFile f(path("/etc/timezone"));
    if (f.open(QIODevice::ReadOnly)) {
        const QString tz = QString::fromUtf8(f.readLine()).trimmed();
        if (!tz.isEmpty())
            return tz;
    }
    return "UTC";
}

bool TimeDate::canNtp() const
{
    return QFileInfo(kNtpd).isExecutable();
}

// The kernel's view: ntpd clears STA_UNSYNC once it disciplines the clock.
bool TimeDate::ntpSynchronized() const
{
    timex tx {};
    if (adjtimex(&tx) < 0)
        return false;
    return !(tx.status & STA_UNSYNC);
}

quint64 TimeDate::timeUSec() const
{
    return quint64(nowUsec());
}

quint64 TimeDate::rtcTimeUSec() const
{
    QFile f("/sys/class/rtc/rtc0/since_epoch");
    if (!f.open(QIODevice::ReadOnly))
        return 0;
    return f.readAll().trimmed().toULongLong() * 1000000;
}

// ---- the clock ----

void TimeDate::writeRtc() const
{
    if (!root_.isEmpty() || !QFile::exists("/dev/rtc0"))
        return;
    // Some RTCs (the tablet's PMIC one) can't be set: then the saved clock
    // (saveClock) is what survives a reboot.
    QProcess::execute(kHwclock, {"-w", localRtc_ ? "-l" : "-u"});
}

void TimeDate::saveClock() const
{
    QDir().mkpath(QFileInfo(path(kSavedClock)).path());
    QFile f(path(kSavedClock));
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QByteArray::number(nowUsec() / 1000000) + '\n');
}

void TimeDate::restoreClock()
{
    if (localRtc_ && root_.isEmpty() && QFile::exists("/dev/rtc0"))
        QProcess::execute(kHwclock, {"-s", "-l"});
    QFile f(path(kSavedClock));
    if (!f.open(QIODevice::ReadOnly))
        return;
    const qint64 saved = f.readAll().trimmed().toLongLong();
    if (saved > 0 && nowUsec() / 1000000 < saved) {
        qInfo("clock behind the saved time, set to %s",
              qPrintable(QDateTime::fromSecsSinceEpoch(saved).toUTC().toString(Qt::ISODate)));
        setClockUsec(saved * 1000000);
    }
}

void TimeDate::startNtp()
{
    if (!ntp_ || !canNtp() || ntpd_.state() != QProcess::NotRunning)
        return;
    // -n: in the foreground, as our child; it steps the clock when far off.
    QStringList args {"-n"};
    for (const QString &s : servers_)
        args << "-p" << s;
    stopping_ = false;
    ntpd_.start(kNtpd, args);
    qInfo("network time: ntpd %s", qPrintable(args.join(' ')));
}

void TimeDate::stopNtp()
{
    restartNtp_.stop();
    if (ntpd_.state() == QProcess::NotRunning)
        return;
    stopping_ = true;
    ntpd_.terminate();
    if (!ntpd_.waitForFinished(3000))
        ntpd_.kill();
    ntpd_.waitForFinished(1000);
}

// ---- methods ----

void TimeDate::SetTime(qlonglong usec, bool relative, bool)
{
    if (!authorized())
        return;
    if (ntp_) {
        sendErrorReply("org.freedesktop.timedate1.AutomaticTimeSyncEnabled",
                       "Network time is on: turn it off to set the time by hand");
        return;
    }
    const qint64 target = relative ? nowUsec() + usec : usec;
    if (target <= 0 || !setClockUsec(target)) {
        sendErrorReply(QDBusError::Failed, QString("cannot set the clock: %1").arg(strerror(errno)));
        return;
    }
    qInfo("clock set to %s", qPrintable(QDateTime::fromMSecsSinceEpoch(target / 1000).toUTC().toString(Qt::ISODate)));
    writeRtc();
    saveClock();
}

bool TimeDate::validZone(const QString &tz) const
{
    static const QRegularExpression name("^[A-Za-z0-9_+-]+(/[A-Za-z0-9_+-]+)*$");
    if (!name.match(tz).hasMatch())
        return false;
    QFile f(path(QString(kZoneinfo) + "/" + tz));
    return f.open(QIODevice::ReadOnly) && f.read(4) == "TZif";
}

void TimeDate::SetTimezone(const QString &tz, bool)
{
    if (!authorized())
        return;
    if (!validZone(tz)) {
        sendErrorReply(QDBusError::InvalidArgs, "Unknown time zone: " + tz);
        return;
    }
    if (tz == timezone())
        return;
    // A new link next to the old, renamed over it: never without one.
    const QString link = path("/etc/localtime"), tmp = link + ".jk-new";
    QFile::remove(tmp);
    if (!QFile::link(QString(kZoneinfo) + "/" + tz, tmp) || ::rename(QFile::encodeName(tmp), QFile::encodeName(link)) != 0) {
        QFile::remove(tmp);
        sendErrorReply(QDBusError::Failed, "cannot write /etc/localtime");
        return;
    }
    QFile f(path("/etc/timezone"));
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(tz.toUtf8() + '\n');
    qInfo("time zone: %s", qPrintable(tz));
    // A local-time RTC follows the zone.
    if (localRtc_)
        writeRtc();
    changed({{"Timezone", tz}});
}

void TimeDate::SetLocalRTC(bool localRtc, bool fixSystem, bool)
{
    if (!authorized())
        return;
    if (localRtc == localRtc_)
        return;
    localRtc_ = localRtc;
    if (!saveConfig()) {
        sendErrorReply(QDBusError::Failed, QString("cannot write %1").arg(kConfig));
        return;
    }
    if (fixSystem && root_.isEmpty() && QFile::exists("/dev/rtc0"))
        QProcess::execute(kHwclock, {"-s", localRtc_ ? "-l" : "-u"});
    else
        writeRtc();
    changed({{"LocalRTC", localRtc_}});
}

void TimeDate::SetNTP(bool useNtp, bool)
{
    if (!authorized())
        return;
    if (useNtp && !canNtp()) {
        sendErrorReply(QDBusError::NotSupported, "no ntpd in this system");
        return;
    }
    if (useNtp == ntp_)
        return;
    ntp_ = useNtp;
    if (!saveConfig()) {
        sendErrorReply(QDBusError::Failed, QString("cannot write %1").arg(kConfig));
        return;
    }
    if (ntp_)
        startNtp();
    else
        stopNtp();
    qInfo("network time %s", ntp_ ? "on" : "off");
    changed({{"NTP", ntp_}});
}

// The zones of tzdata's zone1970.tab (the ones to choose from), and UTC.
QStringList TimeDate::ListTimezones() const
{
    QStringList zones {"UTC"};
    QFile f(path(QString(kZoneinfo) + "/zone1970.tab"));
    if (f.open(QIODevice::ReadOnly)) {
        for (const QByteArray &line : f.readAll().split('\n')) {
            if (line.isEmpty() || line.startsWith('#'))
                continue;
            const QList<QByteArray> cols = line.split('\t');
            if (cols.size() >= 3)
                zones << QString::fromUtf8(cols[2]);
        }
    }
    zones.sort();
    zones.removeDuplicates();
    return zones;
}
