#include "updater.h"

#include "adminpassword.h"

#include <QFile>
#include <unistd.h>

namespace {

// key=value files (/etc/os-release, /etc/jk_os/target, /run/jk-update/state).
QString valueIn(const QString &path, const QString &key)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    const QByteArray prefix = key.toUtf8() + '=';
    for (const QByteArray &line : file.readAll().split('\n')) {
        if (line.startsWith(prefix)) {
            QString value = QString::fromUtf8(line.mid(prefix.size())).trimmed();
            if (value.size() >= 2 && value.startsWith('"') && value.endsWith('"'))
                value = value.mid(1, value.size() - 2);
            return value;
        }
    }
    return {};
}

const QString kPurpose = QObject::tr("Software Update installs a new jk_os release and restarts the system.");

} // namespace

Updater::Updater(QObject *parent)
    : QObject(parent), program_(qEnvironmentVariable("JK_UPDATE_CLI", "/usr/sbin/jk-update"))
{
    installedVersion_ = valueIn("/etc/os-release", "VERSION_ID");
    installedBuild_ = valueIn("/etc/jk_os/target", "JK_BUILD");
    channel_ = valueIn("/etc/jk_os/target", "JK_CHANNEL");
    // What the last check (jk-update-watch) found, until this window's own.
    if (valueIn("/run/jk-update/state", "STATE") == "available") {
        state_ = "available";
        newVersion_ = valueIn("/run/jk-update/state", "VERSION");
        newBuild_ = valueIn("/run/jk-update/state", "BUILD");
        totalBytes_ = valueIn("/run/jk-update/state", "SIZE").toLongLong();
    }
    process_.setProcessChannelMode(QProcess::MergedChannels);
    connect(&process_, &QProcess::readyReadStandardOutput, this, &Updater::readLines);
    connect(&process_, &QProcess::finished, this, &Updater::finished);
}

bool Updater::admin()
{
    if (geteuid() == 0 || askAdminPassword(kPurpose))
        return true;
    error_ = tr("Not done: no administrator password given.");
    emit changed();
    return false;
}

void Updater::run(const QStringList &args, bool asRoot)
{
    if (busy())
        return;
    error_.clear();
    pending_.clear();
    if (asRoot && geteuid() != 0)
        process_.start("sudo", QStringList{"-n", program_} + args);
    else
        process_.start(program_, args);
    emit changed();
}

void Updater::check()
{
    log_.clear();
    shadowed_.clear();
    state_ = "checking";
    run({"--check", "--machine"}, false);
}

void Updater::install()
{
    if (!admin())
        return;
    log_.clear();
    shadowed_.clear();
    doneBytes_ = 0;
    state_ = "downloading";
    run({"--machine"}, true);
}

void Updater::restart()
{
    if (!admin())
        return;
    const bool ok = geteuid() == 0 ? QProcess::startDetached("reboot", {})
                                   : QProcess::startDetached("sudo", {"-n", "reboot"});
    if (!ok) {
        error_ = tr("The system could not be restarted: run sudo reboot.");
        emit changed();
    }
}

void Updater::readLines()
{
    pending_ += process_.readAllStandardOutput();
    qsizetype end;
    while ((end = pending_.indexOf('\n')) >= 0) {
        line(QString::fromUtf8(pending_.left(end)));
        pending_.remove(0, end + 1);
    }
}

void Updater::line(const QString &text)
{
    const QStringList f = text.split('\t');
    const QString kind = f.value(0);
    if (kind == "msg") {
        message_ = f.value(1).trimmed();
        log_.append(message_);
    } else if (kind == "error") {
        error_ = f.value(1);
        log_.append(error_);
    } else if (kind == "release") {
        newVersion_ = f.value(1);
        totalBytes_ = f.value(3).toLongLong();
        releaseDate_ = f.value(4);
        commit_ = f.value(5);
        newBuild_ = f.value(6);
    } else if (kind == "status") {
        state_ = f.value(1);
        if (state_ == "pending" || state_ == "installed")
            newVersion_ = f.value(2);
    } else if (kind == "progress") {
        doneBytes_ = f.value(1).toLongLong();
        totalBytes_ = f.value(2).toLongLong();
    } else if (kind == "shadowed") {
        shadowed_.append(f.value(1));
    } else if (!text.trimmed().isEmpty()) {
        // Not jk-update's own line (sudo's, say): keep it for the log.
        log_.append(text.trimmed());
        if (text.contains("password is required"))
            error_ = tr("sudo asked for the password again: try once more.");
    }
    emit changed();
}

void Updater::finished(int exitCode, QProcess::ExitStatus status)
{
    readLines();
    if (!pending_.isEmpty())
        line(QString::fromUtf8(pending_));
    pending_.clear();
    if (status != QProcess::NormalExit || exitCode != 0) {
        if (error_.isEmpty())
            error_ = tr("jk-update stopped (exit code %1).").arg(exitCode);
        state_ = "error";
    }
    emit changed();
}
