#!/bin/sh
set -eu

shutdown() {
    if [ -n "${server_pid:-}" ] && kill -0 "$server_pid" 2>/dev/null; then
        kill "$server_pid"
    fi
}

trap shutdown INT TERM

su -s /bin/sh node -c 'node Web/Server/src/server.ts' &
server_pid="$!"

nginx -g 'daemon off;'
status="$?"
shutdown
exit "$status"
