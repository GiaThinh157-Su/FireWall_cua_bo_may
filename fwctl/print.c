#include <stdio.h>
#include <arpa/inet.h>
#include "print.h"

static const char *proto_name(uint8_t p)
{
    switch (p) {
    case FW_PROTO_TCP:  return "tcp";
    case FW_PROTO_UDP:  return "udp";
    case FW_PROTO_ICMP: return "icmp";
    default:            return "any";
    }
}

static const char *dir_name(uint8_t d)
{
    switch (d) {
    case FW_DIR_IN:  return "in";
    case FW_DIR_OUT: return "out";
    default:         return "any";
    }
}

static const char *act_name(uint8_t a)
{
    return a == FW_ACT_DROP ? "drop" : "accept";
}

static void fmt_ip(char *buf, size_t n, uint32_t ip, uint32_t mask)
{
    char tmp[INET_ADDRSTRLEN];
    int prefix;

    if (mask == 0) {
        snprintf(buf, n, "any");
        return;
    }
    inet_ntop(AF_INET, &ip, tmp, sizeof(tmp));
    prefix = __builtin_popcount(ntohl(mask));
    if (prefix == 32)
        snprintf(buf, n, "%s", tmp);
    else
        snprintf(buf, n, "%s/%d", tmp, prefix);
}

static void fmt_port(char *buf, size_t n, uint16_t lo, uint16_t hi)
{
    if (lo == 0 && hi == 65535)
        snprintf(buf, n, "any");
    else if (lo == hi)
        snprintf(buf, n, "%u", lo);
    else
        snprintf(buf, n, "%u-%u", lo, hi);
}

void print_header(void)
{
    printf("%-4s %-5s %-4s %-18s %-18s %-11s %-11s %-7s %8s %10s\n",
           "ID", "PROTO", "DIR", "SRC", "DST", "SPORT", "DPORT", "ACTION", "PKTS", "BYTES");
}

void print_rule_row(const struct fw_rule_entry *e)
{
    const struct fw_rule *r = &e->rule;
    char src[48], dst[48], sp[16], dp[16];

    fmt_ip(src, sizeof(src), r->src_ip, r->src_mask);
    fmt_ip(dst, sizeof(dst), r->dst_ip, r->dst_mask);
    fmt_port(sp, sizeof(sp), r->sport_min, r->sport_max);
    fmt_port(dp, sizeof(dp), r->dport_min, r->dport_max);

    printf("%-4u %-5s %-4s %-18s %-18s %-11s %-11s %-7s %8llu %10llu\n",
           r->id, proto_name(r->proto), dir_name(r->direction), src, dst, sp, dp,
           act_name(r->action),
           (unsigned long long)e->packets, (unsigned long long)e->bytes);
}

void print_stats(const struct fw_stats *s)
{
    printf("Tong so goi:            %llu\n", (unsigned long long)s->total_packets);
    printf("Goi cho qua:            %llu\n", (unsigned long long)s->accepted_packets);
    printf("Goi bi chan:            %llu\n", (unsigned long long)s->dropped_packets);
    printf("Ket noi dang theo doi:  %llu\n", (unsigned long long)s->active_connections);
}

void print_rule_as_cmd(FILE *f, const struct fw_rule *r)
{
    char b[48];

    fprintf(f, "add");
    if (r->proto)
        fprintf(f, " --proto %s", proto_name(r->proto));
    if (r->src_mask) {
        fmt_ip(b, sizeof(b), r->src_ip, r->src_mask);
        fprintf(f, " --src %s", b);
    }
    if (r->dst_mask) {
        fmt_ip(b, sizeof(b), r->dst_ip, r->dst_mask);
        fprintf(f, " --dst %s", b);
    }
    if (r->sport_min != 0 || r->sport_max != 65535) {
        fmt_port(b, sizeof(b), r->sport_min, r->sport_max);
        fprintf(f, " --sport %s", b);
    }
    if (r->dport_min != 0 || r->dport_max != 65535) {
        fmt_port(b, sizeof(b), r->dport_min, r->dport_max);
        fprintf(f, " --dport %s", b);
    }
    if (r->direction)
        fprintf(f, " --dir %s", dir_name(r->direction));
    fprintf(f, " --action %s\n", act_name(r->action));
}
