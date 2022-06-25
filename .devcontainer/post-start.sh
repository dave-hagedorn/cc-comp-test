#!/usr/bin/env bash

set -euo pipefail

readonly TAG="devc-start"
readonly EMPTY_TAG="            "
readonly HOST_HOME="/home/host"

lastLevel=
log() {
    local -r LEVEL="${FUNCNAME[1]}"
    local -r LEVEL_W="$(printf "%-5s" "$LEVEL")"

    # shellcheck disable=SC2034
    declare -A COLORS=(
        ["info"]="32"
        ["warn"]="34"
        ["error"]="35"
    )

    if [[ "$lastLevel" != "$LEVEL" ]]; then
        echo -e "\033[${COLORS[$LEVEL]};1m[$TAG $LEVEL_W]\033[0m \033[3m$*\033[0m"
    else
        echo -e " $EMPTY_TAG      \033[3m$*\033[0m"
    fi

    lastLevel="$LEVEL"
}

info() {
    log "$*"
}
warn() {
    log "$*"
}
error() {
    log "$*"
}

link-host-config() {
    local -r HOST_FILES=(
        .zshrc
        .bashrc
    )

    for file in "${HOST_FILES[@]}"; do
        local src="$HOST_HOME/$file"
        local dst="$HOME/$file"

        if [ -f "$src" ]; then
            info "found host's config $file you might want in dev env -> linking to $dst"
            rm -f "$dst"
            ln -s "$src" "$dst"
        fi
    done
}

main() {
    info "devcontainer post-start"
    link-host-config
}

main "$@"