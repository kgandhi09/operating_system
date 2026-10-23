// jk-viz: every process and what it consumes, out of 100% of each resource,
// as layers from the silicon (top) to the applications (bottom). Live, from
// jk-vizd.
//
//   jk-viz [--file <snapshot.json>]
// For tests: --screenshot <png> [--delay <ms>] [--fit] [--find <text>]
// [--resource <0-6>] saves a picture of the window after <ms> and quits.
#include <QGuiApplication>
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("jk-viz");
    app.setApplicationDisplayName("jk-viz");
    app.setOrganizationName("J.K. Robotics");
    app.setDesktopFileName("jk-viz");

    QString shot, find;
    int delay = 2500, resource = -1;
    bool fit = false;
    const QStringList args = app.arguments();
    for (qsizetype i = 1; i < args.size(); i++) {
        if (args[i] == "--screenshot" && i + 1 < args.size())
            shot = args[++i];
        else if (args[i] == "--delay" && i + 1 < args.size())
            delay = args[++i].toInt();
        else if (args[i] == "--fit")
            fit = true;
        else if (args[i] == "--find" && i + 1 < args.size())
            find = args[++i];
        else if (args[i] == "--resource" && i + 1 < args.size())
            resource = args[++i].toInt();
        else if (args[i] == "--file" && i + 1 < args.size())
            qputenv("JK_VIZ_FILE", args[++i].toLocal8Bit());
    }

    // Dark, whatever the desktop's theme (the graph is drawn on dark).
    QQuickStyle::setStyle("Fusion");
    QPalette pal;
    pal.setColor(QPalette::Window, QColor("#14181e"));
    pal.setColor(QPalette::WindowText, QColor("#e6e9ee"));
    pal.setColor(QPalette::Base, QColor("#0e1116"));
    pal.setColor(QPalette::AlternateBase, QColor("#191e26"));
    pal.setColor(QPalette::Text, QColor("#e6e9ee"));
    pal.setColor(QPalette::Button, QColor("#232a34"));
    pal.setColor(QPalette::ButtonText, QColor("#e6e9ee"));
    pal.setColor(QPalette::Highlight, QColor("#3d7fd6"));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::PlaceholderText, QColor("#7d8694"));
    pal.setColor(QPalette::Mid, QColor("#2c3440"));
    pal.setColor(QPalette::Dark, QColor("#0b0d11"));
    pal.setColor(QPalette::Light, QColor("#3a4452"));
    pal.setColor(QPalette::ToolTipBase, QColor("#191e26"));
    pal.setColor(QPalette::ToolTipText, QColor("#e6e9ee"));
    QGuiApplication::setPalette(pal);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); },
                     Qt::QueuedConnection);
    engine.loadFromModule("JkViz", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;

    if (!shot.isEmpty()) {
        // (Tests: a picture of the window, then quit.)
        auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QTimer::singleShot(std::max(0, delay - 800), &app, [win, fit, find, resource] {
            auto *graph = win->findChild<QObject *>("graph");
            auto *view = win->findChild<QObject *>("view");
            if (graph && resource >= 0)
                graph->setProperty("resource", resource);
            if (graph && view && !find.isEmpty()) {
                graph->setProperty("search", find);
                QString id;
                QMetaObject::invokeMethod(graph, "nextMatch", Q_RETURN_ARG(QString, id));
                QMetaObject::invokeMethod(view, "centerOnNode", Q_ARG(QString, id));
            }
            if (view && fit)
                QMetaObject::invokeMethod(view, "fit");
        });
        QTimer::singleShot(delay, &app, [win, shot] {
            win->grabWindow().save(shot);
            QCoreApplication::quit();
        });
    }
    return app.exec();
}
