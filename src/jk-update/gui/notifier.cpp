#include "notifier.h"

#include <KNotification>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>

namespace {

const QString kRunDir = "/run/jk-update";
const QString kState = kRunDir + "/state";

QString stateValue(const QByteArray &text, const QByteArray &key)
{
    for (const QByteArray &line : text.split('\n'))
        if (line.startsWith(key + '='))
            return QString::fromUtf8(line.mid(key.size() + 1)).trimmed();
    return {};
}

} // namespace

Notifier::Notifier(QObject *parent) : QObject(parent)
{
    // The state file is replaced (renamed into place): watch its directory,
    // and /run until jk-update-watch makes that.
    QDir().exists(kRunDir) ? watcher_.addPath(kRunDir) : watcher_.addPath("/run");
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, [this] {
        if (!watcher_.directories().contains(kRunDir) && QDir().exists(kRunDir))
            watcher_.addPath(kRunDir);
        look();
    });
    // A release put off with Later comes back a day later.
    daily_.setInterval(60 * 60 * 1000);
    connect(&daily_, &QTimer::timeout, this, &Notifier::look);
    daily_.start();
    look();
}

void Notifier::look()
{
    QFile file(kState);
    if (!file.open(QIODevice::ReadOnly))
        return;
    const QByteArray text = file.readAll();
    if (stateValue(text, "STATE") != "available") {
        if (shown_)
            shown_->close();
        return;
    }
    // A release is its version and build: a re-publish of the same version
    // (scripts/publish.sh --force) is another one.
    const QString version = stateValue(text, "VERSION");
    const QString build = stateValue(text, "BUILD");
    const QString key = version + '/' + build;
    if (shown_ && shownVersion_ == key)
        return;
    QSettings put("jk_os", "jk-update-notify");
    const QDateTime until = put.value("laterUntil").toDateTime();
    if (put.value("laterVersion").toString() == key && until.isValid()
        && QDateTime::currentDateTime() < until)
        return;
    const bool sameVersion = version == stateValue(text, "CURRENT");
    notify(key, sameVersion ? tr("A new build of jk_os %1 is available").arg(version)
                            : tr("jk_os %1 is available").arg(version),
           stateValue(text, "SIZE").toLongLong());
}

void Notifier::notify(const QString &version, const QString &title, qint64 size)
{
    if (shown_)
        shown_->close();
    auto *n = new KNotification("updateAvailable", KNotification::Persistent, this);
    n->setComponentName("jk-update");
    n->setIconName("system-software-update");
    n->setTitle(title);
    n->setText(size > 0 ? tr("%1 MB to download. It installs in the background; restart when it's done.")
                              .arg(size / (1024 * 1024))
                        : tr("Install it in the background; restart when it's done."));
    KNotificationAction *installAction = n->addAction(tr("Install…"));
    KNotificationAction *laterAction = n->addAction(tr("Later"));
    connect(installAction, &KNotificationAction::activated, this, [n] {
        QProcess::startDetached(QCoreApplication::applicationFilePath(), {"--install"});
        n->close();
    });
    connect(laterAction, &KNotificationAction::activated, this, [this, n, version] {
        later(version);
        n->close();
    });
    // Dismissed without an answer: the same as Later.
    connect(n, &KNotification::closed, this, [this, version] {
        QSettings put("jk_os", "jk-update-notify");
        if (put.value("laterVersion").toString() != version)
            later(version);
    });
    shown_ = n;
    shownVersion_ = version;
    n->sendEvent();
}

void Notifier::later(const QString &version)
{
    QSettings put("jk_os", "jk-update-notify");
    put.setValue("laterVersion", version);
    put.setValue("laterUntil", QDateTime::currentDateTime().addDays(1));
}
