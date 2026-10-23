// jk-timedated: org.freedesktop.timedate1, the system clock service that
// KDE's Date & Time settings (and anything written for systemd-timedated)
// talks to: the time zone, setting the time, and network time (NTP, with
// BusyBox's ntpd).
#pragma once
#include <QDBusConnection>
#include <QDBusContext>
#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QTimer>

// The bus it serves on: the system bus (the session bus for tests).
QDBusConnection timedateBus();

class TimeDate : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.timedate1")
    Q_PROPERTY(QString Timezone READ timezone)
    Q_PROPERTY(bool LocalRTC READ localRtc)
    Q_PROPERTY(bool CanNTP READ canNtp)
    Q_PROPERTY(bool NTP READ ntp)
    Q_PROPERTY(bool NTPSynchronized READ ntpSynchronized)
    Q_PROPERTY(quint64 TimeUSec READ timeUSec)
    Q_PROPERTY(quint64 RTCTimeUSec READ rtcTimeUSec)

public:
    explicit TimeDate(QObject *parent = nullptr);
    ~TimeDate() override;

    QString timezone() const;
    bool localRtc() const { return localRtc_; }
    bool canNtp() const;
    bool ntp() const { return ntp_; }
    bool ntpSynchronized() const;
    quint64 timeUSec() const;
    quint64 rtcTimeUSec() const;

    // Before the bus: put the clock back to the last time saved if it is
    // behind it (a tablet's RTC can't be set), set it from a local-time RTC.
    void restoreClock();
    void saveClock() const;
    void startNtp();
    void stopNtp();

public slots:
    void SetTime(qlonglong usec, bool relative, bool interactive);
    void SetTimezone(const QString &tz, bool interactive);
    void SetLocalRTC(bool localRtc, bool fixSystem, bool interactive);
    void SetNTP(bool useNtp, bool interactive);
    QStringList ListTimezones() const;

private:
    QString root_;          // "" normally; another tree for tests
    bool ntp_ = true;
    bool localRtc_ = false;
    QStringList servers_;
    QProcess ntpd_;
    QTimer saveTimer_, restartNtp_;
    bool stopping_ = false;

    QString path(const QString &p) const { return root_ + p; }
    void loadConfig();
    bool saveConfig() const;
    bool authorized();
    void changed(const QVariantMap &props);
    bool validZone(const QString &tz) const;
    void writeRtc() const;
};
