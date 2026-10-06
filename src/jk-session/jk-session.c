/*
 * jk-session [desktop|dev]: start a graphical session on the console the
 * user is logged in on.
 *
 *   desktop  KDE Plasma (/usr/lib/jk_os/jk-gui-session; jk-gui)
 *   dev      the dev session: one terminal in the cage kiosk compositor
 *            (/usr/lib/jk_os/jk-dev-session; jk-dev)
 *
 * Either runs on any text console, tty1-tty6.
 *
 * Compositors need a logind (elogind) session to get the GPU and the input
 * devices and to let the kernel switch consoles while they run. Console
 * logins here don't go through PAM, so they don't have one; this helper opens
 * it, the way a display manager would, then runs the session as the user:
 *
 *   pam_open_session (service "jk-gui": pam_elogind) on this console,
 *   drop to the user, run the session program, wait for it,
 *   pam_close_session.
 *
 * Installed setuid root; jk-gui and jk-dev run it. It runs nothing the
 * caller chooses beyond picking one of the two programs above: the caller
 * must be a regular user, and its
 * standard input must be a text console (/dev/ttyN) that it owns, which login
 * made it. No password is asked: the user has already logged in there.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <security/pam_appl.h>

#define SERVICE "jk-gui"
#define DEFAULT_PATH "/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin"

static void die(const char *msg)
{
    fprintf(stderr, "jk-session: %s\n", msg);
    exit(1);
}

/* Nothing is authenticated, so there is nothing to ask. */
static int no_conv(int n, const struct pam_message **msg, struct pam_response **resp, void *data)
{
    (void)n; (void)msg; (void)resp; (void)data;
    return PAM_CONV_ERR;
}

static void check_pam(pam_handle_t *pamh, int rc, const char *what)
{
    if (rc != PAM_SUCCESS) {
        fprintf(stderr, "jk-session: %s: %s\n", what, pam_strerror(pamh, rc));
        exit(1);
    }
}

/* Copy one variable from the caller's environment into envp, if set. */
static void keep(char **envp, size_t *n, const char *name)
{
    const char *v = getenv(name);
    if (v && strlen(v) < 4096 && asprintf(&envp[*n], "%s=%s", name, v) >= 0)
        (*n)++;
}

int main(int argc, char **argv)
{
    const char *program = "/usr/lib/jk_os/jk-gui-session", *desktop = "KDE";
    if (argc == 2 && strcmp(argv[1], "dev") == 0) {
        program = "/usr/lib/jk_os/jk-dev-session";
        desktop = "cage";
    } else if (argc > 2 || (argc == 2 && strcmp(argv[1], "desktop") != 0)) {
        die("usage: jk-session [desktop|dev]");
    }

    uid_t uid = getuid();
    if (geteuid() != 0)
        die("must be installed setuid root");
    if (uid == 0)
        die("start a graphical session as a regular user, not as root");

    struct passwd *pw = getpwuid(uid);
    if (!pw)
        die("unknown user");
    /* getpwuid's buffer is overwritten by later lookups (PAM modules). */
    char *user = strdup(pw->pw_name), *home = strdup(pw->pw_dir), *shell = strdup(pw->pw_shell);
    gid_t gid = pw->pw_gid;
    if (!user || !home || !shell)
        die("out of memory");

    /* A text console that the caller is logged in on. */
    const char *tty = ttyname(STDIN_FILENO);
    struct stat st;
    unsigned vt;
    char rest;
    if (!tty || sscanf(tty, "/dev/tty%u%c", &vt, &rest) != 1 || vt < 1 || vt > 63)
        die("start the session from a text console you are logged in on");
    if (fstat(STDIN_FILENO, &st) != 0 || st.st_uid != uid)
        die("this console belongs to another user");
    char vtnr[8];
    snprintf(vtnr, sizeof vtnr, "%u", vt);

    /* Let the child get the terminal signals; this process only waits. */
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    pam_handle_t *pamh = NULL;
    struct pam_conv conv = { no_conv, NULL };
    check_pam(pamh, pam_start(SERVICE, user, &conv, &pamh), "pam_start");
    check_pam(pamh, pam_set_item(pamh, PAM_TTY, tty + strlen("/dev/")), "PAM_TTY");
    /* What pam_elogind registers: a graphical user session on seat0, on this VT. */
    const char *const session_env[] = {
        "XDG_SESSION_TYPE=wayland", "XDG_SESSION_CLASS=user", "XDG_SEAT=seat0", NULL,
    };
    for (const char *const *e = session_env; *e; e++)
        check_pam(pamh, pam_putenv(pamh, *e), "pam_putenv");
    char deskenv[64];
    snprintf(deskenv, sizeof deskenv, "XDG_SESSION_DESKTOP=%s", desktop);
    check_pam(pamh, pam_putenv(pamh, deskenv), "pam_putenv");
    char vtenv[32];
    snprintf(vtenv, sizeof vtenv, "XDG_VTNR=%s", vtnr);
    check_pam(pamh, pam_putenv(pamh, vtenv), "pam_putenv");
    check_pam(pamh, pam_acct_mgmt(pamh, 0), "account");
    check_pam(pamh, pam_open_session(pamh, 0), "cannot open a session");

    /* The session's environment: the session's (XDG_RUNTIME_DIR,
     * XDG_SESSION_ID, ...) plus a few of the caller's; nothing else passes
     * through a setuid program. */
    char **pam_env = pam_getenvlist(pamh);
    size_t n = 0, cap = 16;
    for (char **e = pam_env; e && *e; e++)
        cap++;
    char **envp = calloc(cap, sizeof *envp);
    if (!envp)
        die("out of memory");
    for (char **e = pam_env; e && *e; e++)
        envp[n++] = *e;
    if (asprintf(&envp[n], "HOME=%s", home) >= 0) n++;
    if (asprintf(&envp[n], "USER=%s", user) >= 0) n++;
    if (asprintf(&envp[n], "LOGNAME=%s", user) >= 0) n++;
    if (asprintf(&envp[n], "SHELL=%s", shell) >= 0) n++;
    envp[n++] = "PATH=" DEFAULT_PATH;
    keep(envp, &n, "TERM");
    keep(envp, &n, "LANG");
    keep(envp, &n, "TZ");
    /* jk-dev --rotate, for jk-dev-session (which checks it). */
    keep(envp, &n, "JK_DEV_ROTATION");
    envp[n] = NULL;

    pid_t child = fork();
    if (child < 0)
        die("fork failed");
    if (child == 0) {
        if (initgroups(user, gid) != 0 || setgid(gid) != 0 || setuid(uid) != 0)
            _exit(1);
        if (setuid(0) == 0)     /* the drop must be permanent */
            _exit(1);
        for (int s = 1; s < NSIG; s++)
            signal(s, SIG_DFL);
        if (chdir(home) != 0)
            (void)chdir("/");
        char *child_argv[] = { (char *)program, NULL };
        execve(program, child_argv, envp);
        fprintf(stderr, "jk-session: cannot run %s: %s\n", program, strerror(errno));
        _exit(127);
    }

    int status = 0;
    while (waitpid(child, &status, 0) < 0 && errno == EINTR)
        ;
    pam_close_session(pamh, 0);
    pam_end(pamh, PAM_SUCCESS);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
