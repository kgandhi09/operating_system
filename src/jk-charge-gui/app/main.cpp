// jk-charge-gui: the battery and its charger in real time: level,
// charging or discharging and how fast, power, voltage, temperature, health,
// time left, the charger's settings (jk-charge) and the last hour as graphs.
//
//   jk-charge-gui
// For tests: --screenshot <png> [--delay <ms>] saves a picture of the window
// after <ms> and quits. JK_POWER_SUPPLY_DIR reads another tree than
// /sys/class/power_supply.
#include <QGuiApplication>
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <cstdio>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (geteuid() != 0) {
        std::fprintf(stderr, "jk-charge-gui: run as root to change charging settings\n");
        return 1;
    }
    QGuiApplication app(argc, argv);
    app.setApplicationName("jk-charge-gui");
    app.setApplicationDisplayName("Charge Settings");
    app.setOrganizationName("J.K. Robotics");
    app.setDesktopFileName("jk-charge-gui");

    QString shot;
    int delay = 2500;
    const QStringList args = app.arguments();
    for (qsizetype i = 1; i < args.size(); i++) {
        if (args[i] == "--screenshot" && i + 1 < args.size())
            shot = args[++i];
        else if (args[i] == "--delay" && i + 1 < args.size())
            delay = args[++i].toInt();
    }

    // Dark, like jk-viz, whatever the desktop's theme.
    QQuickStyle::setStyle("Fusion");
    QPalette pal;
    pal.setColor(QPalette::Window, QColor("#14181e"));
    pal.setColor(QPalette::WindowText, QColor("#e6e9ee"));
    pal.setColor(QPalette::Base, QColor("#0e1116"));
    pal.setColor(QPalette::Text, QColor("#e6e9ee"));
    pal.setColor(QPalette::Button, QColor("#232a34"));
    pal.setColor(QPalette::ButtonText, QColor("#e6e9ee"));
    pal.setColor(QPalette::Highlight, QColor("#3d7fd6"));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::ToolTipBase, QColor("#191e26"));
    pal.setColor(QPalette::ToolTipText, QColor("#e6e9ee"));
    QGuiApplication::setPalette(pal);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); },
                     Qt::QueuedConnection);
    engine.loadFromModule("JkChargeGui", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;

    if (!shot.isEmpty()) {
        auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QTimer::singleShot(delay, &app, [win, shot] {
            win->grabWindow().save(shot);
            QCoreApplication::quit();
        });
    }
    return app.exec();
}
