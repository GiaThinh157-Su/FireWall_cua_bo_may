# fwctl: công cụ dòng lệnh

Biên dịch: `sudo apt install libnl-3-dev libnl-genl-3-dev pkg-config && make -C fwctl`

| Lệnh | Ví dụ |
|---|---|
| add | `sudo fwctl add --proto tcp --src 192.168.1.0/24 --dport 80 --action drop` |
| del | `sudo fwctl del --id 3` |
| list | `sudo fwctl list` |
| flush | `sudo fwctl flush` |
| policy | `sudo fwctl policy drop` |
| stats | `sudo fwctl stats` |
| save / load | `sudo fwctl save /etc/fwctl/rules.conf`, `sudo fwctl load /etc/fwctl/rules.conf` |

Tuỳ chọn của add: `--proto tcp|udp|icmp|any`, `--src/--dst IP[/mask]`, `--sport/--dport P hoặc P1-P2`, `--dir in|out|any`, `--action accept|drop` (bắt buộc).

Chế độ giả lập (không cần kernel): `FWCTL_MOCK=1 fwctl list`. Test tự động: `fwctl/test_cli.sh`.

Ghi chú: `save` chỉ lưu luật. Policy mặc định chưa có lệnh đọc ngược từ kernel, muốn giữ lại thì thêm dòng `policy drop` vào file luật.
