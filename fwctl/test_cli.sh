#!/bin/bash
# Test CLI fwctl o che do gia lap (khong can kernel)
cd "$(dirname "$0")" || exit 1
export FWCTL_MOCK=1
pass=0; fail=0

t() {  # t "ten" ma_thoat_mong_doi lenh...
    name=$1; want=$2; shift 2
    "$@" >/dev/null 2>&1
    got=$?
    if [ "$got" = "$want" ]; then
        echo "PASS: $name"; pass=$((pass+1))
    else
        echo "FAIL: $name (mong $want, duoc $got)"; fail=$((fail+1))
    fi
}

printf 'add --proto tcp --dport 99999 --action drop\n' > /tmp/fwctl_bad.conf
./fwctl save /tmp/fwctl_ok.conf >/dev/null 2>&1

t "add hop le"            0 ./fwctl add --proto tcp --src 192.168.1.0/24 --dport 80 --action drop
t "add dai cong"          0 ./fwctl add --proto udp --dport 1000-2000 --action accept
t "add cong sai"          1 ./fwctl add --proto tcp --dport 99999 --action drop
t "add icmp kem dport"    1 ./fwctl add --proto icmp --dport 80 --action drop
t "add IP sai"            1 ./fwctl add --proto tcp --src 300.1.1.1 --action drop
t "add thieu action"      1 ./fwctl add --proto tcp --dport 80
t "add tuy chon la"       1 ./fwctl add --abc 1 --action drop
t "del hop le"            0 ./fwctl del --id 3
t "del id chu"            1 ./fwctl del --id abc
t "del id 0"              1 ./fwctl del --id 0
t "policy drop"           0 ./fwctl policy drop
t "policy sai"            1 ./fwctl policy xyz
t "list"                  0 ./fwctl list
t "list thua tham so"     1 ./fwctl list thua
t "flush"                 0 ./fwctl flush
t "stats"                 0 ./fwctl stats
t "lenh la"               1 ./fwctl abc
t "khong tham so"         1 ./fwctl
t "save"                  0 ./fwctl save /tmp/fwctl_t.conf
t "load file dung"        0 ./fwctl load /tmp/fwctl_ok.conf
t "load file khong co"    1 ./fwctl load /tmp/khong_co_file.conf
t "load file co loi"      1 ./fwctl load /tmp/fwctl_bad.conf

echo "Tong: $pass PASS, $fail FAIL"
[ "$fail" -eq 0 ]
