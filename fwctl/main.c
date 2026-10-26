#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "parse.h"

static void usage(void)
{
    fprintf(stderr,
        "Cach dung: fwctl <lenh> [tuy chon]\n"
        "  add --proto tcp --dport 80 --action drop\n"
        "  del --id N\n"
        "  list\n"
        "  flush\n"
        "  policy accept|drop\n"
        "  stats\n");
}

static int parse_del(int argc, char **argv, uint32_t *id)
{
    char *end;
    unsigned long v;

    if (argc != 2 || strcmp(argv[0], "--id")) {
        fprintf(stderr, "Loi: dung 'del --id N'\n");
        return -1;
    }
    errno = 0;
    v = strtoul(argv[1], &end, 10);
    if (errno || end == argv[1] || *end != '\0' || v == 0 || v > 0xFFFFFFFFUL) {
        fprintf(stderr, "Loi: id '%s' khong hop le (so nguyen duong)\n", argv[1]);
        return -1;
    }
    *id = (uint32_t)v;
    return 0;
}

static int parse_policy(int argc, char **argv, uint8_t *pol)
{
    if (argc != 1) {
        fprintf(stderr, "Loi: dung 'policy accept|drop'\n");
        return -1;
    }
    if (!strcmp(argv[0], "accept"))    *pol = FW_ACT_ACCEPT;
    else if (!strcmp(argv[0], "drop")) *pol = FW_ACT_DROP;
    else {
        fprintf(stderr, "Loi: policy '%s' khong hop le (accept|drop)\n", argv[0]);
        return -1;
    }
    return 0;
}

static int no_args(const char *cmd, int argc)
{
    if (argc != 0) {
        fprintf(stderr, "Loi: lenh '%s' khong nhan tham so\n", cmd);
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return 1;
    }

    const char *cmd = argv[1];
    int n = argc - 2;
    char **args = argv + 2;

    if (!strcmp(cmd, "add")) {
        struct fw_rule r;
        char src[INET_ADDRSTRLEN] = "any";

        if (parse_add(n, args, &r))
            return 1;
        if (r.src_mask)
            inet_ntop(AF_INET, &r.src_ip, src, sizeof(src));
        printf("[MOCK] se gui ADD: proto=%u action=%u src=%s dport=%u-%u\n",
               r.proto, r.action, src, r.dport_min, r.dport_max);
    } else if (!strcmp(cmd, "del")) {
        uint32_t id;
        if (parse_del(n, args, &id))
            return 1;
        printf("[MOCK] se gui DEL id=%u\n", id);
    } else if (!strcmp(cmd, "list")) {
        if (no_args(cmd, n)) return 1;
        printf("[MOCK] se gui lenh LIST\n");
    } else if (!strcmp(cmd, "flush")) {
        if (no_args(cmd, n)) return 1;
        printf("[MOCK] se gui lenh FLUSH\n");
    } else if (!strcmp(cmd, "policy")) {
        uint8_t pol;
        if (parse_policy(n, args, &pol))
            return 1;
        printf("[MOCK] se gui SET_POLICY %s\n", pol == FW_ACT_DROP ? "drop" : "accept");
    } else if (!strcmp(cmd, "stats")) {
        if (no_args(cmd, n)) return 1;
        printf("[MOCK] se gui lenh GET_STATS\n");
    } else {
        fprintf(stderr, "Lenh khong hop le: %s\n", cmd);
        usage();
        return 1;
    }
    return 0;
}
