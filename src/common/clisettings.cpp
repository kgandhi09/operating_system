#include "clisettings.h"

#include <QProcess>
#include <QVariantMap>

bool CliSettings::command(const QString &program, const QStringList &args,
                          const QByteArray &input, QByteArray &output)
{
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(program, args);
    if (!process.waitForStarted(5000)) {
        output = process.errorString().toUtf8();
        return false;
    }
    if (!input.isEmpty())
        process.write(input);
    process.closeWriteChannel();
    if (!process.waitForFinished(30000)) {
        process.kill();
        process.waitForFinished();
        output = "The command timed out";
        return false;
    }
    output = process.readAll();
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

void CliSettings::refresh()
{
    if (program_.isEmpty())
        return;
    QByteArray output;
    const QStringList args = charge_ ? QStringList{"--machine"}
                                     : QStringList{"list", "--machine"};
    if (!command(program_, args, {}, output)) {
        error_ = QString::fromUtf8(output).trimmed();
        emit changed();
        return;
    }

    QVariantList next;
    for (const QByteArray &line : output.split('\n')) {
        if (line.isEmpty())
            continue;
        const QList<QByteArray> cells = line.split('\t');
        QVariantMap row;
        if (charge_) {
            if (cells.size() != 6)
                continue;
            row.insert("key", QString::fromUtf8(cells[0]));
            row.insert("type", "number");
            row.insert("current", QString::fromUtf8(cells[1]));
            row.insert("saved", QString::fromUtf8(cells[2]));
            row.insert("choices", QString::fromUtf8(cells[3]) + "-" + QString::fromUtf8(cells[4]));
            row.insert("unit", QString::fromUtf8(cells[5]));
            row.insert("group", "Charging");
            row.insert("label", QString::fromUtf8(cells[0]));
        } else {
            if (cells.size() != 9)
                continue;
            static const char *names[] = {"key", "type", "current", "saved", "source",
                                          "choices", "unit", "group", "label"};
            for (int i = 0; i < 9; ++i)
                row.insert(names[i], QString::fromUtf8(cells[i]));
        }
        next.append(row);
    }
    rows_ = next;

    if (!charge_) {
        QByteArray modesOutput;
        if (command(program_, {"modes"}, {}, modesOutput)) {
            modes_.clear();
            mode_.clear();
            for (const QByteArray &line : modesOutput.split('\n')) {
                if (line.size() < 3)
                    continue;
                const QString name = QString::fromUtf8(line.mid(2)).trimmed();
                modes_.append(name);
                if (line.startsWith("* "))
                    mode_ = name;
            }
        }
    }
    error_.clear();
    emit changed();
}

bool CliSettings::invoke(const QStringList &args, const QString &password)
{
    if (program_.isEmpty() || args.isEmpty())
        return false;
    QByteArray output;
    // sudo -S reads the password from a pipe. It is never placed in an
    // argument or stored on disk. A cached sudo ticket also works with an
    // empty field.
    QStringList sudoArgs{"-S", "-p", "", program_};
    sudoArgs.append(args);
    const bool ok = command("sudo", sudoArgs, password.toUtf8() + '\n', output);
    message_ = ok ? QString::fromUtf8(output).trimmed() : QString();
    error_ = ok ? QString() : QString::fromUtf8(output).trimmed();
    if (ok) {
        refresh();
        // refresh clears error, but keep the command's result visible.
        message_ = QString::fromUtf8(output).trimmed();
    }
    emit changed();
    return ok;
}
