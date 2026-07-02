#!/bin/sh
set -eu

shutdown() {
    if [ -n "${server_pid:-}" ] && kill -0 "$server_pid" 2>/dev/null; then
        kill "$server_pid"
    fi
}

trap shutdown INT TERM

chown node:node /data

su-exec node node Web/Server/src/server.ts &
server_pid="$!"

nginx -g 'daemon off;'
status="$?"
shutdown
exit "$status"
