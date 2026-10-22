#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

// jk-update for the window: runs it with --machine (as root under sudo -n,
// the password asked once in KDE's dialog) and follows its lines: what is
// installed and available, the download's progress, the install.
class Updater : public QObject {
    Q_OBJECT
    QML_ELEMENT
    // idle, checking, uptodate, available, pending, downloading, installing,
    // installed, error
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(QString installedVersion READ installedVersion CONSTANT)
    Q_PROPERTY(QString installedBuild READ installedBuild CONSTANT)
    Q_PROPERTY(QString channel READ channel CONSTANT)
    Q_PROPERTY(QString newVersion READ newVersion NOTIFY changed)
    // Its build ID: told apart from the running build when the version is
    // the same (scripts/publish.sh --force).
    Q_PROPERTY(QString newBuild READ newBuild NOTIFY changed)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY changed)
    Q_PROPERTY(qint64 doneBytes READ doneBytes NOTIFY changed)
    Q_PROPERTY(QString releaseDate READ releaseDate NOTIFY changed)
    Q_PROPERTY(QString commit READ commit NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QStringList log READ log NOTIFY changed)
    Q_PROPERTY(QStringList shadowed READ shadowed NOTIFY changed)

public:
    explicit Updater(QObject *parent = nullptr);

    QString state() const { return state_; }
    bool busy() const { return process_.state() != QProcess::NotRunning; }
    QString installedVersion() const { return installedVersion_; }
    QString installedBuild() const { return installedBuild_; }
    QString channel() const { return channel_; }
    QString newVersion() const { return newVersion_; }
    QString newBuild() const { return newBuild_; }
    qint64 totalBytes() const { return totalBytes_; }
    qint64 doneBytes() const { return doneBytes_; }
    QString releaseDate() const { return releaseDate_; }
    QString commit() const { return commit_; }
    QString message() const { return message_; }
    QString error() const { return error_; }
    QStringList log() const { return log_; }
    QStringList shadowed() const { return shadowed_; }

    Q_INVOKABLE void check();
    Q_INVOKABLE void install();
    Q_INVOKABLE void restart();

signals:
    void changed();

private:
    void run(const QStringList &args, bool asRoot);
    void readLines();
    void finished(int exitCode, QProcess::ExitStatus status);
    void line(const QString &text);
    bool admin();

    QProcess process_;
    QByteArray pending_;
    QString program_;
    QString state_ = "idle";
    QString installedVersion_;
    QString installedBuild_;
    QString channel_;
    QString newVersion_;
    QString newBuild_;
    qint64 totalBytes_ = 0;
    qint64 doneBytes_ = 0;
    QString releaseDate_;
    QString commit_;
    QString message_;
    QString error_;
    QStringList log_;
    QStringList shadowed_;
};
