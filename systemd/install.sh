#!/bin/bash
# Dung: sudo ./systemd/install.sh [ten_module]   (mac dinh: fw_module)
set -e
MODULE=${1:-fw_module}
cd "$(dirname "$0")/.."
[ "$(id -u)" = 0 ] || { echo "Hay chay bang sudo"; exit 1; }
[ -x fwctl/fwctl ] || { echo "Chua build fwctl. Chay: make -C fwctl"; exit 1; }
install -m 755 fwctl/fwctl /usr/local/bin/fwctl
mkdir -p /etc/fwctl
[ -f /etc/fwctl/rules.conf ] || echo "# Luat fwctl" > /etc/fwctl/rules.conf
sed "s/@MODULE@/$MODULE/g" systemd/fwctl.service.in > /etc/systemd/system/fwctl.service
systemctl daemon-reload
systemctl enable fwctl.service
echo "Xong. Thu: sudo systemctl start fwctl && systemctl status fwctl"
