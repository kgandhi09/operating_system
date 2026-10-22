#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QPointer>
#include <QTimer>

class KNotification;

// jk-update gui --watch, started with the desktop (/etc/xdg/autostart): no
// window, only KDE's notification once jk-update-watch has found a new
// release (/run/jk-update/state). Install opens the window; Later (or
// closing the notification) shows it again a day later, or at once for a
// newer release than the one put off.
class Notifier : public QObject {
    Q_OBJECT
public:
    explicit Notifier(QObject *parent = nullptr);

private:
    void look();
    // <version>: the release's key (version/build), for Later.
    void notify(const QString &version, const QString &title, qint64 size);
    void later(const QString &version);

    QFileSystemWatcher watcher_;
    QTimer daily_;
    QPointer<KNotification> shown_;
    QString shownVersion_;
};
