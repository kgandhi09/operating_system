// jk-power-gui: the mode and every setting jk-power offers on this machine.
//
//   jk-power gui [--ask-password]   (jk-power starts this program)
// As root (sudo jk-power gui) changes apply at once; as a user, through
// sudo, with KDE's password dialog when sudo needs the password, and with
// --ask-password (the desktop's menu entry) at the start, cancel quitting.
// For tests: --screenshot <png> saves a picture of the window and quits.
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
    app.setApplicationName("jk-power-gui");
    app.setApplicationDisplayName("Power Settings");
    app.setDesktopFileName("jk-power-gui");
    if (app.arguments().contains("--ask-password")
        && !askAdminPassword(QObject::tr("Power Settings changes how hard this machine runs.")))
        return 0;
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
    QApplication::setPalette(pal);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("JkPowerGui", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;

    const QStringList args = app.arguments();
    const qsizetype shotArg = args.indexOf("--screenshot");
    if (shotArg >= 0 && shotArg + 1 < args.size()) {
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        const QString path = args[shotArg + 1];
        QTimer::singleShot(2000, &app, [window, path] {
            if (window)
                window->grabWindow().save(path);
            QCoreApplication::quit();
        });
    }
    return app.exec();
}
