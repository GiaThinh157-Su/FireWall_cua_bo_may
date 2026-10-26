#ifndef FW_CMD_DRAFT_H
#define FW_CMD_DRAFT_H

#include <stdint.h>
#include "fw_common_draft.h"

#define FW_GENL_NAME    "fw_netlink"
#define FW_GENL_VERSION 1

enum fw_cmd {
    FW_CMD_UNSPEC = 0,
    FW_CMD_ADD,
    FW_CMD_DEL,
    FW_CMD_LIST,
    FW_CMD_FLUSH,
    FW_CMD_SET_POLICY,
    FW_CMD_GET_STATS,
    __FW_CMD_MAX
};
#define FW_CMD_MAX (__FW_CMD_MAX - 1)

enum fw_attr {
    FW_ATTR_UNSPEC = 0,
    FW_ATTR_RULE,        /* struct fw_rule */
    FW_ATTR_RULE_ENTRY,  /* struct fw_rule_entry */
    FW_ATTR_ID,          /* u32 */
    FW_ATTR_POLICY,      /* u8: FW_ACT_ACCEPT / FW_ACT_DROP */
    FW_ATTR_STATS,       /* struct fw_stats */
    __FW_ATTR_MAX
};
#define FW_ATTR_MAX (__FW_ATTR_MAX - 1)

/* Một luật kèm bộ đếm, dùng cho lệnh LIST */
struct fw_rule_entry {
    struct fw_rule rule;
    uint64_t packets;
    uint64_t bytes;
};

/* Thống kê tổng, dùng cho lệnh GET_STATS */
struct fw_stats {
    uint64_t total_packets;
    uint64_t accepted_packets;
    uint64_t dropped_packets;
    uint64_t active_connections;
};

#endif
