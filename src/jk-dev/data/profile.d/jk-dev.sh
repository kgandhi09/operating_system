# The dev session's terminal opens with jk_os's banner, once: shells started
# inside it (and a ~/.zshrc that shows the banner itself) see JK_BANNER_SHOWN.
case "$-" in
    *i*)
        if [ -n "${JK_DEV_SESSION:-}" ] && [ -z "${JK_BANNER_SHOWN:-}" ] && [ -r /usr/share/jk_os/banner ]; then
            echo
            cat /usr/share/jk_os/banner
            echo
            export JK_BANNER_SHOWN=1
        fi ;;
esac
