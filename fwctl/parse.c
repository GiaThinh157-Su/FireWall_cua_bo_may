#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "parse.h"

static int parse_cidr(const char *s, uint32_t *ip, uint32_t *mask)
{
    char buf[64];
    char *slash;
    int prefix = 32;
    struct in_addr addr;

    if (strlen(s) >= sizeof(buf)) return -1;
    strcpy(buf, s);

    slash = strchr(buf, '/');
    if (slash) {
        char *end;
        *slash = '\0';
        errno = 0;
        long p = strtol(slash + 1, &end, 10);
        if (errno || *end != '\0' || end == slash + 1 || p < 0 || p > 32)
            return -1;
        prefix = (int)p;
    }
    if (inet_pton(AF_INET, buf, &addr) != 1) return -1;

    *ip = addr.s_addr;
    *mask = prefix == 0 ? 0 : htonl(0xFFFFFFFFu << (32 - prefix));
    *ip &= *mask;
    return 0;
}

static int parse_port_num(const char *s, uint16_t *out)
{
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno || end == s || *end != '\0' || v < 0 || v > 65535) return -1;
    *out = (uint16_t)v;
    return 0;
}

static int parse_port_range(const char *s, uint16_t *lo, uint16_t *hi)
{
    char buf[32];
    char *dash;

    if (strlen(s) >= sizeof(buf)) return -1;
    strcpy(buf, s);
    dash = strchr(buf, '-');
    if (dash) {
        *dash = '\0';
        if (parse_port_num(buf, lo) || parse_port_num(dash + 1, hi)) return -1;
        if (*lo > *hi) return -1;
        return 0;
    }
    if (parse_port_num(buf, lo)) return -1;
    *hi = *lo;
    return 0;
}

static int parse_proto(const char *s, uint8_t *out)
{
    if (!strcmp(s, "tcp"))  { *out = FW_PROTO_TCP;  return 0; }
    if (!strcmp(s, "udp"))  { *out = FW_PROTO_UDP;  return 0; }
    if (!strcmp(s, "icmp")) { *out = FW_PROTO_ICMP; return 0; }
    if (!strcmp(s, "any"))  { *out = FW_PROTO_ANY;  return 0; }
    return -1;
}

int parse_add(int argc, char **argv, struct fw_rule *r)
{
    int have_action = 0;

    memset(r, 0, sizeof(*r));
    r->sport_max = 65535;
    r->dport_max = 65535;

    for (int i = 0; i < argc; i++) {
        const char *opt = argv[i];

        if (i + 1 >= argc) {
            fprintf(stderr, "Loi: tuy chon %s thieu gia tri\n", opt);
            return -1;
        }
        const char *val = argv[++i];

        if (!strcmp(opt, "--proto")) {
            if (parse_proto(val, &r->proto)) {
                fprintf(stderr, "Loi: proto '%s' khong hop le (tcp|udp|icmp|any)\n", val);
                return -1;
            }
        } else if (!strcmp(opt, "--src")) {
            if (parse_cidr(val, &r->src_ip, &r->src_mask)) {
                fprintf(stderr, "Loi: IP nguon '%s' khong hop le\n", val);
                return -1;
            }
        } else if (!strcmp(opt, "--dst")) {
            if (parse_cidr(val, &r->dst_ip, &r->dst_mask)) {
                fprintf(stderr, "Loi: IP dich '%s' khong hop le\n", val);
                return -1;
            }
        } else if (!strcmp(opt, "--sport")) {
            if (parse_port_range(val, &r->sport_min, &r->sport_max)) {
                fprintf(stderr, "Loi: sport '%s' khong hop le\n", val);
                return -1;
            }
        } else if (!strcmp(opt, "--dport")) {
            if (parse_port_range(val, &r->dport_min, &r->dport_max)) {
                fprintf(stderr, "Loi: dport '%s' khong hop le\n", val);
                return -1;
            }
        } else if (!strcmp(opt, "--action")) {
            if (!strcmp(val, "accept"))    r->action = FW_ACT_ACCEPT;
            else if (!strcmp(val, "drop")) r->action = FW_ACT_DROP;
            else {
                fprintf(stderr, "Loi: action '%s' khong hop le (accept|drop)\n", val);
                return -1;
            }
            have_action = 1;
        } else if (!strcmp(opt, "--dir")) {
            if (!strcmp(val, "in"))        r->direction = FW_DIR_IN;
            else if (!strcmp(val, "out"))  r->direction = FW_DIR_OUT;
            else if (!strcmp(val, "any"))  r->direction = FW_DIR_ANY;
            else {
                fprintf(stderr, "Loi: dir '%s' khong hop le (in|out|any)\n", val);
                return -1;
            }
        } else {
            fprintf(stderr, "Loi: tuy chon la '%s'\n", opt);
            return -1;
        }
    }

    if (!have_action) {
        fprintf(stderr, "Loi: thieu --action accept|drop\n");
        return -1;
    }
    if ((r->dport_min != 0 || r->dport_max != 65535 ||
         r->sport_min != 0 || r->sport_max != 65535) &&
        r->proto != FW_PROTO_TCP && r->proto != FW_PROTO_UDP) {
        fprintf(stderr, "Loi: dung --sport/--dport thi phai co --proto tcp hoac udp\n");
        return -1;
    }
    return 0;
}
