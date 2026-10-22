#pragma once

#include <QString>

// The administrator password for the settings windows (jk-power gui,
// jk-charge gui), asked in KDE's password dialog (KPasswordDialog) and
// checked with sudo itself. A correct password leaves sudo's time stamp
// fresh for this program (sudo -n then runs without asking, for sudo's
// timeout), so the password itself is never kept.
//
// askAdminPassword: show the dialog until the password is right or the user
// cancels; true if sudo now runs without asking. <purpose> says what the
// password allows. Running as root, true at once.
bool askAdminPassword(const QString &purpose);
