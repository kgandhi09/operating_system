// jk-timedated: org.freedesktop.timedate1 on the system bus (see
// timedate.h). Started at boot by /etc/init.d/S19timedate (jk_os's D-Bus
// has no service activation); SIGTERM saves the clock and stops ntpd.
//
// For tests: JK_TIMEDATED_BUS=session and JK_TIMEDATED_ROOT=<dir> (files
// under <dir> instead of /etc, /var).
#include "timedate.h"

#include <QCoreApplication>
#include <QSocketNotifier>

#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

namespace {
int sigFd[2];
void onSignal(int)
{
    const char c = 1;
    [[maybe_unused]] ssize_t n = write(sigFd[0], &c, 1);
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("jk-timedated");

    // SIGTERM/SIGINT through a socket pair: quit from the event loop.
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sigFd) == 0) {
        auto *sn = new QSocketNotifier(sigFd[1], QSocketNotifier::Read, &app);
        QObject::connect(sn, &QSocketNotifier::activated, &app, [] { QCoreApplication::quit(); });
        std::signal(SIGTERM, onSignal);
        std::signal(SIGINT, onSignal);
    }

    TimeDate td;
    td.restoreClock();

    QDBusConnection bus = timedateBus();
    if (!bus.isConnected()) {
        qCritical("no D-Bus bus: %s", qPrintable(bus.lastError().message()));
        return 1;
    }
    if (!bus.registerObject("/org/freedesktop/timedate1", &td,
                            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties)
        || !bus.registerService("org.freedesktop.timedate1")) {
        qCritical("cannot take org.freedesktop.timedate1: %s", qPrintable(bus.lastError().message()));
        return 1;
    }

    if (td.ntp())
        td.startNtp();
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [&td] {
        td.stopNtp();
        td.saveClock();
    });
    return app.exec();
}
