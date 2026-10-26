#ifndef FW_COMMON_DRAFT_H
#define FW_COMMON_DRAFT_H

#include <stdint.h>

#define FW_PROTO_ANY  0
#define FW_PROTO_ICMP 1
#define FW_PROTO_TCP  6
#define FW_PROTO_UDP  17

#define FW_ACT_ACCEPT 0
#define FW_ACT_DROP   1

#define FW_DIR_ANY 0
#define FW_DIR_IN  1
#define FW_DIR_OUT 2

struct fw_rule {
    uint32_t id;
    uint8_t  proto;
    uint8_t  action;
    uint8_t  direction;
    uint32_t src_ip;
    uint32_t src_mask;
    uint32_t dst_ip;
    uint32_t dst_mask;
    uint16_t sport_min, sport_max;
    uint16_t dport_min, dport_max;
};

#endif
