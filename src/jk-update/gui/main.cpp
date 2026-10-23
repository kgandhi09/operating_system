// jk-update-gui: Software Update, jk-update's window: what is installed, the
// newest release on this system's channel, installing it in the background
// (the download's progress, then Restart now / Later).
//
//   jk-update gui              the window; it checks when it opens
//   jk-update gui --install    the window, installing at once (the
//                              notification's Install button)
//   jk-update gui --watch      no window: KDE's notification when
//                              jk-update-watch finds a new release (started
//                              with the desktop, /etc/xdg/autostart)
// As root (sudo jk-update gui) it installs and restarts at once; as a user
// it asks for the password in KDE's dialog first.
// For tests: --screenshot <png> saves a picture of the window and quits.
#include "notifier.h"

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
    app.setApplicationName("jk-update-gui");
    app.setApplicationDisplayName("Software Update");
    app.setOrganizationName("J.K. Robotics");
    app.setDesktopFileName("jk-update-gui");
    const QStringList args = app.arguments();

    if (args.contains("--watch")) {
        app.setQuitOnLastWindowClosed(false);   // the password dialog, if any
        Notifier notifier;
        return app.exec();
    }

    // Dark, like jk-viz and the settings windows, whatever the desktop's theme.
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
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); },
                     Qt::QueuedConnection);
    engine.setInitialProperties({{"installAtStart", args.contains("--install")}});
    engine.loadFromModule("JkUpdateGui", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;

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
