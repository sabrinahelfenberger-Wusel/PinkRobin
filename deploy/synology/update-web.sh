#!/bin/sh
# Run locally on the NAS. Credentials live in Docker's login configuration.
set -eu
PATH=$PATH:/usr/local/bin:/usr/bin:/bin:/usr/syno/bin
export PATH
IMAGE=${PINKROBIN_IMAGE:-ghcr.io/sabrinahelfenberger-wusel/pinkrobin-web:main}
PORT=${PINKROBIN_PORT:-8080}
BIND=${PINKROBIN_BIND:-127.0.0.1}
NAME=pinkrobin-web
PREVIOUS=pinkrobin-web-previous
CANDIDATE=pinkrobin-web-candidate
LOCK=${PINKROBIN_LOCK:-/tmp/pinkrobin-web-deploy.lock}

if ! mkdir "$LOCK" 2>/dev/null; then
    echo 'Another deployment is running (or a stale lock needs checking).'
    exit 0
fi
switched=false
restore=false
cleanup() {
    result=$?
    trap - EXIT
    docker rm -f "$CANDIDATE" >/dev/null 2>&1 || true
    if [ "$result" -ne 0 ] && [ "$switched" = true ]; then
        docker rm -f "$NAME" >/dev/null 2>&1 || true
        if [ "$restore" = true ]; then
            docker rename "$PREVIOUS" "$NAME"
            docker start "$NAME" >/dev/null
            echo 'Deployment failed; previous container restored.' >&2
        fi
    fi
    rmdir "$LOCK"
    exit "$result"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM

exists() { docker inspect "$1" >/dev/null 2>&1; }
healthy() {
    running=$(docker inspect --format '{{.State.Running}}' "$1")
    [ "$running" = true ] || return 1
    count=0
    while [ "$count" -lt 30 ]; do
        status=$(docker inspect --format '{{.State.Health.Status}}' "$1")
        [ "$status" != healthy ] || return 0
        [ "$status" != unhealthy ] || return 1
        count=$((count + 1))
        sleep 2
    done
    return 1
}

if exists "$NAME"; then
    managed=$(docker inspect --format '{{index .Config.Labels "ch.wanderwusel.pinkrobin.managed"}}' "$NAME")
    if [ "$managed" != true ]; then
        echo 'Existing container is not managed by this deployment. Perform the documented first migration.' >&2
        exit 1
    fi
fi

docker pull "$IMAGE"
# Resolve the moving tag once. All following operations use this exact image ID.
TARGET=$(docker image inspect --format '{{.Id}}' "$IMAGE")
if exists "$NAME"; then
    current=$(docker inspect --format '{{.Image}}' "$NAME")
    if [ "$current" = "$TARGET" ]; then
        healthy "$NAME"
        echo 'Already running the current release.'
        exit 0
    fi
fi

docker run -d --name "$CANDIDATE" "$TARGET" >/dev/null
healthy "$CANDIDATE"
docker exec "$CANDIDATE" wget -q -O /dev/null http://127.0.0.1/index.html

if exists "$NAME"; then
    # The earlier successful container is retained until the next deployment.
    if exists "$PREVIOUS"; then docker rm "$PREVIOUS" >/dev/null; fi
    docker stop "$NAME" >/dev/null
    if ! docker rename "$NAME" "$PREVIOUS"; then
        docker start "$NAME" >/dev/null
        exit 1
    fi
    restore=true
fi
switched=true
docker run -d --name "$NAME" --restart unless-stopped \
    --label ch.wanderwusel.pinkrobin.managed=true \
    -p "$BIND:$PORT:80" "$TARGET" >/dev/null
healthy "$NAME"
docker exec "$NAME" wget -q -O /dev/null http://127.0.0.1/index.html
echo "Release deployed: $TARGET"
