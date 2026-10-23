// jk-link gui: the mesh console. Shows what jk-linkd knows (this machine,
// the machines around it, the links between them) and pairs, accepts and
// unpairs (as an administrator: group wheel). Started by `jk-link gui`.
//
//   jk-link gui [<machine>]                   open on that machine (name or ID)
//   --screenshot <file.png> [--after <ms>]   save the window and quit (tests)
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("jk-link"));
    app.setApplicationDisplayName(QStringLiteral("jk-link"));
    app.setDesktopFileName(QStringLiteral("jk-link"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("network-workgroup")));

    QString shot, select;
    int after = 4000;
    const QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == QLatin1String("--screenshot") && i + 1 < args.size())
            shot = args[++i];
        else if (args[i] == QLatin1String("--after") && i + 1 < args.size())
            after = args[++i].toInt();
        else if (!args[i].startsWith(QLatin1Char('-')))
            select = args[i];
    }

    QQmlApplicationEngine engine;
    engine.setInitialProperties({{QStringLiteral("initialSelect"), select}});
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("JkLink", "Main");

    if (!shot.isEmpty() && !engine.rootObjects().isEmpty()) {
        auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QTimer::singleShot(after, &app, [win, shot] {
            bool ok = win && win->grabWindow().save(shot);
            QCoreApplication::exit(ok ? 0 : 1);
        });
    }
    return app.exec();
}
