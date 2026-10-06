#include <QGuiApplication>
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("jk-power-gui");
    app.setApplicationDisplayName("Power Settings");
    app.setDesktopFileName("jk-power-gui");
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
    QGuiApplication::setPalette(pal);

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
