# Sourced by jk-install and jk-setup: ask for the first accounts and create
# them with shadow-utils (useradd, chpasswd). Needs say/warn/ask from the caller.
#
#   accounts_ask                 fill ACC_* from the user's answers
#   accounts_apply <prefix> <home-root>
#       create them in <prefix>/etc (a staging copy of /etc, or / itself),
#       with the home directory under <home-root>
#
# ACC_FULLNAME ACC_USER ACC_PASS ACC_ADMIN (yes/no) ACC_ROOTPASS ACC_HOSTNAME

# ask_secret <prompt>: read a password twice, without echo, into $secret.
ask_secret() {
    while :; do
        printf '%s: ' "$1"; stty -echo 2>/dev/null; read -r secret; r1=$?; stty echo 2>/dev/null; echo
        [ $r1 = 0 ] || die "input closed"
        if [ ${#secret} -lt 4 ]; then warn "use at least 4 characters"; continue; fi
        printf '%s (again): ' "$1"; stty -echo 2>/dev/null; read -r again; stty echo 2>/dev/null; echo
        [ "$secret" = "$again" ] && return 0
        warn "the passwords do not match"
    done
}

accounts_ask() {
    head_line "Who are you?"
    ask "  Your name (optional)" "${ACC_FULLNAME:-}"
    ACC_FULLNAME=$(echo "$ans" | tr -d ':,')
    default=$(echo "${ACC_FULLNAME%% *}" | tr 'A-Z' 'a-z' | tr -cd 'a-z0-9_-')
    while :; do
        ask "  Username" "$default"
        ACC_USER="$ans"
        echo "$ACC_USER" | grep -qE '^[a-z_][a-z0-9_-]{0,31}$' \
            || { warn "use lowercase letters, digits, - and _, starting with a letter"; continue; }
        grep -q "^$ACC_USER:" /etc/passwd && { warn "$ACC_USER is a system account"; continue; }
        break
    done
    ask_secret "  Password for $ACC_USER"; ACC_PASS="$secret"
    while :; do
        ask "  Computer name" "${ACC_HOSTNAME:-$(cat /etc/hostname 2>/dev/null)}"
        ACC_HOSTNAME="$ans"
        echo "$ACC_HOSTNAME" | grep -qE '^[A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?$' && break
        warn "use letters, digits and -, at most 63 characters"
    done
    say ""
    say "  Only root can create and manage users. $ACC_USER can change only their"
    say "  own password, name and shell, unless made an administrator: then"
    say "  $ACC_USER runs commands as root with sudo (their own password), and"
    say "  may become root with su (the root password)."
    ask "  Make $ACC_USER an administrator? (yes/no)" yes
    ACC_ADMIN=no; [ "$ans" = yes ] && ACC_ADMIN=yes
    ask_secret "  Password for root"; ACC_ROOTPASS="$secret"
    unset secret again
}

accounts_summary() {
    say "  - create user $ACC_USER${ACC_FULLNAME:+ ($ACC_FULLNAME)}$([ "$ACC_ADMIN" = yes ] && echo ', administrator (sudo, su)')"
    say "  - set the root password and the computer name ($ACC_HOSTNAME)"
}

accounts_apply() {
    prefix="$1" homes="$2"
    popt=""; [ "$prefix" = / ] || popt="--prefix $prefix"
    groups=""; [ "$ACC_ADMIN" = yes ] && groups="-G wheel"
    # shellcheck disable=SC2086 # $popt and $groups are option lists
    useradd $popt -M -d "/home/$ACC_USER" -s /bin/sh -c "$ACC_FULLNAME" $groups "$ACC_USER" \
        || die "useradd $ACC_USER failed"
    # One chpasswd per account: it picks a single salt per run.
    # shellcheck disable=SC2086
    printf '%s:%s\n' "$ACC_USER" "$ACC_PASS" | chpasswd $popt -c YESCRYPT || die "chpasswd failed"
    # shellcheck disable=SC2086
    printf 'root:%s\n' "$ACC_ROOTPASS" | chpasswd $popt -c YESCRYPT || die "chpasswd failed"

    ids=$(awk -F: -v u="$ACC_USER" '$1 == u { print $3 ":" $4 }' "${prefix%/}/etc/passwd")
    [ -n "$ids" ] || die "$ACC_USER missing from ${prefix%/}/etc/passwd"
    mkdir -p "$homes/$ACC_USER"
    cp -a /etc/skel/. "$homes/$ACC_USER/"
    chown -R "$ids" "$homes/$ACC_USER"
    chmod 0700 "$homes/$ACC_USER"

    echo "$ACC_HOSTNAME" > "${prefix%/}/etc/hostname"
    sed -i "s/^127\.0\.1\.1[[:space:]].*/127.0.1.1\t$ACC_HOSTNAME/" "${prefix%/}/etc/hosts"
    unset ACC_PASS ACC_ROOTPASS
}
