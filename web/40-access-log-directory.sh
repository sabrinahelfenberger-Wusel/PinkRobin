#!/bin/sh
set -eu
# Variable log paths are opened by nginx workers, not the root master process.
mkdir -p /var/log/pinkrobin
chown nginx:nginx /var/log/pinkrobin
chmod 750 /var/log/pinkrobin
