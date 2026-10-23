# A new release (jk-update-watch, /run/jk-update/state), on text consoles
# and in the dev session; the desktop shows a notification.
case "$-" in
    *i*)
        if [ -z "${KDE_FULL_SESSION:-}" ] && [ -z "${JK_UPDATE_SHOWN:-}" ] && [ -r /run/jk-update/state ]; then
            if [ "$(sed -n 's/^STATE=//p' /run/jk-update/state)" = available ]; then
                _v=$(sed -n 's/^VERSION=//p' /run/jk-update/state)
                # Another build of the version running now: a re-publish.
                [ "$_v" = "$(sed -n 's/^CURRENT=//p' /run/jk-update/state)" ] && _v="$_v (new build $(sed -n 's/^BUILD=//p' /run/jk-update/state))"
                printf '\033[1;33mjk_os %s is available:\033[0m sudo jk-update (or jk-update gui)\n\n' "$_v"
                unset _v
            fi
            export JK_UPDATE_SHOWN=1
        fi ;;
esac
