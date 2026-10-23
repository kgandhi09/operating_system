// jk-charge-gui: the battery and its charger in real time: level,
// charging or discharging and how fast, power, voltage, temperature, health,
// time left, the charger's settings (jk-charge) and the last hour as graphs.
//
//   jk-charge gui [--ask-password]   (jk-charge starts this program)
// As root (sudo jk-charge gui) changes apply at once; as a user, through
// sudo, with KDE's password dialog when sudo needs the password, and with
// --ask-password (the desktop's menu entry) at the start, cancel quitting.
// For tests: --screenshot <png> [--delay <ms>] saves a picture of the window
// after <ms> and quits. JK_POWER_SUPPLY_DIR reads another tree than
// /sys/class/power_supply.
#include "adminpassword.h"

#include <QApplication>
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

int main(int argc, char **argv)
{
    // QApplication (widgets): KDE's password dialog.
    QApplication app(argc, argv);
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
    if (args.contains("--ask-password")
        && !askAdminPassword(QObject::tr("Charge Settings changes how the battery charges.")))
        return 0;

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
    QApplication::setPalette(pal);

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
