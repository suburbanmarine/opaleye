#!/usr/bin/env bash
trap 'exit -1' err

rsync -vP l4t-r32.5.0/*.deb  root@repo.suburbanmarine.io:/site/l4t-r32.5.0/arm64/
rsync -vP l4t-r32.6.1/*.deb  root@repo.suburbanmarine.io:/site/l4t-r32.6.1/arm64/
rsync -vP ubuntu-18.04/*.deb root@repo.suburbanmarine.io:/site/ubuntu-18.04/amd64/
rsync -vP ubuntu-20.04/*.deb root@repo.suburbanmarine.io:/site/ubuntu-20.04/amd64/
rsync -vP ubuntu-22.04/*.deb root@repo.suburbanmarine.io:/site/ubuntu-22.04/amd64/

ssh root@repo.suburbanmarine.io "cd /site; /site/update_repo.sh"
