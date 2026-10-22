#include "adminpassword.h"

#include <KPasswordDialog>
#include <QGuiApplication>
#include <QIcon>
#include <QProcess>
#include <unistd.h>

namespace {

// KPasswordDialog keeps itself open, with the error shown, while
// checkPassword() says no.
class SudoPasswordDialog : public KPasswordDialog {
public:
    using KPasswordDialog::KPasswordDialog;

protected:
    bool checkPassword() override
    {
        // -v: only check the password (and refresh the time stamp); -S: read
        // it from stdin; -p "": no prompt of sudo's own.
        QProcess sudo;
        sudo.setProcessChannelMode(QProcess::MergedChannels);
        sudo.start("sudo", {"-S", "-p", "", "-v"});
        if (!sudo.waitForStarted(5000)) {
            showErrorMessage(tr("sudo could not be started: %1").arg(sudo.errorString()), FatalError);
            return false;
        }
        sudo.write(password().toUtf8() + '\n');
        sudo.closeWriteChannel();
        if (!sudo.waitForFinished(30000)) {
            sudo.kill();
            sudo.waitForFinished();
            showErrorMessage(tr("sudo did not answer."), UnknownError);
            return false;
        }
        if (sudo.exitStatus() == QProcess::NormalExit && sudo.exitCode() == 0)
            return true;
        const QString out = QString::fromUtf8(sudo.readAll());
        if (out.contains("not in the sudoers") || out.contains("not allowed"))
            showErrorMessage(tr("Your account isn't an administrator (not in the wheel group)."), FatalError);
        else
            showErrorMessage(tr("Wrong password, try again."), PasswordError);
        return false;
    }
};

} // namespace

bool askAdminPassword(const QString &purpose)
{
    if (geteuid() == 0)
        return true;
    SudoPasswordDialog dialog;
    dialog.setWindowTitle(QGuiApplication::applicationDisplayName());
    dialog.setIcon(QIcon::fromTheme("dialog-password"));
    dialog.setPrompt(purpose + "\n\n" + QObject::tr("Enter your password to continue."));
    return dialog.exec() == QDialog::Accepted;
}
