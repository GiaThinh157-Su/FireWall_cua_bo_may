# Giao thức lệnh fwctl <-> kernel (Generic Netlink)

- Tên family: `fw_netlink`, version 1
- Dữ liệu đi kèm đóng gói bằng thuộc tính (attribute) Netlink.
- Mọi lệnh đều được kernel trả về mã kết quả: 0 = OK, số âm = lỗi (-EINVAL, -ENOENT, -ENOMEM, -EPERM).
- Cần quyền root (CAP_NET_ADMIN) để gửi lệnh ADD/DEL/FLUSH/SET_POLICY.

## Danh sách lệnh

| Lệnh | fwctl gõ | Gửi vào kernel | Kernel trả về |
|---|---|---|---|
| FW_CMD_ADD | `fwctl add --proto tcp --dport 80 --action drop` | FW_ATTR_RULE (struct fw_rule) | FW_ATTR_ID = id luật mới |
| FW_CMD_DEL | `fwctl del --id 3` | FW_ATTR_ID (u32) | chỉ mã kết quả (-ENOENT nếu id không có) |
| FW_CMD_LIST | `fwctl list` | không có | nhiều FW_ATTR_RULE_ENTRY (struct fw_rule_entry), kèm bộ đếm gói/byte |
| FW_CMD_FLUSH | `fwctl flush` | không có | chỉ mã kết quả (xóa toàn bộ luật) |
| FW_CMD_SET_POLICY | `fwctl policy accept` hoặc `fwctl policy drop` | FW_ATTR_POLICY (u8: 0 accept, 1 drop) | chỉ mã kết quả |
| FW_CMD_GET_STATS | `fwctl stats` | không có | FW_ATTR_STATS (struct fw_stats) |

## Thuộc tính (attribute)

| Tên | Kiểu | Ý nghĩa |
|---|---|---|
| FW_ATTR_RULE | binary, struct fw_rule | một luật để thêm |
| FW_ATTR_RULE_ENTRY | binary, struct fw_rule_entry | một luật kèm bộ đếm (dùng cho LIST) |
| FW_ATTR_ID | u32 | id luật |
| FW_ATTR_POLICY | u8 | policy mặc định |
| FW_ATTR_STATS | binary, struct fw_stats | thống kê tổng |

## Quy ước dữ liệu

- IP và mask: network byte order. Cổng: host byte order.
- proto: 0 any, 1 icmp, 6 tcp, 17 udp.
- action: 0 accept, 1 drop.
- direction: 0 any, 1 in, 2 out.
- Cổng đơn (80) được lưu thành dải min = max = 80. Không chỉ định cổng thì dải là 0-65535.
- LIST có thể trả nhiều gói tin Netlink (multipart/dump), mỗi gói chứa một FW_ATTR_RULE_ENTRY.

## Việc cần chốt với trưởng nhóm (A) và B

- Tên trường và kiểu dữ liệu trong struct fw_rule có trùng với fw_common.h chính thức không.
- Giới hạn số luật tối đa.
- Có thêm trường `priority`/thứ tự luật không (hiện tại: luật khớp đầu tiên theo thứ tự thêm).
