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
LOG_DIR=${PINKROBIN_LOG_DIR:-/volume1/docker/pinkrobin-logs}
LOCK=${PINKROBIN_LOCK:-/tmp/pinkrobin-web-deploy.lock}

if ! mkdir "$LOCK" 2>/dev/null; then
    echo "Deployment lock unavailable: $LOCK. Check for a running deployment or a stale lock." >&2
    exit 1
fi
rollback_needed=false
previous_image=
cleanup() {
    result=$?
    trap - EXIT HUP INT TERM
    # Cleanup must release the lock even if Docker or the rollback fails.
    set +e
    docker rm -f "$CANDIDATE" >/dev/null 2>&1
    if [ "$result" -ne 0 ] && [ "$rollback_needed" = true ]; then
        remaining_image=$(docker inspect --format '{{.Image}}' "$NAME" 2>/dev/null)
        if [ "$remaining_image" = "$previous_image" ]; then
            # Interrupted during stop, before the old container was renamed.
            if docker start "$NAME" >/dev/null; then
                echo 'Deployment failed; previous container restarted.' >&2
            else
                echo 'ERROR: Previous container could not be restarted. Manual recovery required.' >&2
            fi
        else
            # Rename may have finished just before a signal interrupted the shell.
            docker rm -f "$NAME" >/dev/null 2>&1
            if docker rename "$PREVIOUS" "$NAME" && docker start "$NAME" >/dev/null; then
                echo 'Deployment failed; previous container restored.' >&2
            else
                echo 'ERROR: Rollback failed. Check both container names before retrying.' >&2
            fi
        fi
    fi
    if ! rmdir "$LOCK"; then
        echo "ERROR: Could not release deployment lock: $LOCK" >&2
        result=1
    fi
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

# Only prune our daily access logs, never other files in the directory.
case "$LOG_DIR" in
    /*) ;;
    *) echo 'PINKROBIN_LOG_DIR must be an absolute path.' >&2; exit 1 ;;
esac
case "$LOG_DIR" in
    *,*) echo 'PINKROBIN_LOG_DIR must not contain commas.' >&2; exit 1 ;;
esac
mkdir -p "$LOG_DIR"
find "$LOG_DIR" -maxdepth 1 -type f -name 'access-????-??-??.log' -mtime +13 -delete

docker pull "$IMAGE"
# Resolve the moving tag once. All following operations use this exact image ID.
TARGET=$(docker image inspect --format '{{.Id}}' "$IMAGE")
if exists "$NAME"; then
    current=$(docker inspect --format '{{.Image}}' "$NAME")
    mounted_logs=$(docker inspect --format '{{range .Mounts}}{{if eq .Destination "/var/log/pinkrobin"}}{{.Source}}{{end}}{{end}}' "$NAME")
    if [ "$current" = "$TARGET" ] && [ "$mounted_logs" = "$LOG_DIR" ]; then
        healthy "$NAME"
        echo 'Already running the current release.'
        exit 0
    fi
fi

docker run -d --name "$CANDIDATE"     --mount "type=bind,source=$LOG_DIR,target=/var/log/pinkrobin"     --log-driver json-file --log-opt max-size=10m --log-opt max-file=3 "$TARGET" >/dev/null
healthy "$CANDIDATE"
docker exec "$CANDIDATE" wget -q -O /dev/null http://127.0.0.1/index.html

if exists "$NAME"; then
    # The earlier successful container is retained until the next deployment.
    if exists "$PREVIOUS"; then docker rm "$PREVIOUS" >/dev/null; fi
    previous_image=$(docker inspect --format '{{.Image}}' "$NAME")
    # Arm recovery before the first operation that can stop the live site.
    rollback_needed=true
    docker stop "$NAME" >/dev/null
    docker rename "$NAME" "$PREVIOUS"
fi
docker run -d --name "$NAME" --restart unless-stopped \
    --label ch.wanderwusel.pinkrobin.managed=true \
    --mount "type=bind,source=$LOG_DIR,target=/var/log/pinkrobin"     --log-driver json-file --log-opt max-size=10m --log-opt max-file=3     -p "$BIND:$PORT:80" "$TARGET" >/dev/null
healthy "$NAME"
docker exec "$NAME" wget -q -O /dev/null http://127.0.0.1/index.html
echo "Release deployed: $TARGET"
