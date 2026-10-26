#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "parse.h"
#include "fwnl.h"
#include "print.h"

#define DEFAULT_RULEFILE "/etc/fwctl/rules.conf"

static int g_dry; /* 1 = chi kiem tra cu phap, khong gui vao kernel */

static int run_command(const char *cmd, int n, char **args);

static void usage(void)
{
    fprintf(stderr,
        "Cach dung: fwctl <lenh> [tuy chon]\n"
        "  add --proto tcp|udp|icmp|any [--src IP/MASK] [--dst IP/MASK]\n"
        "      [--sport P|P1-P2] [--dport P|P1-P2] [--dir in|out|any] --action accept|drop\n"
        "  del --id N\n"
        "  list\n"
        "  flush\n"
        "  policy accept|drop\n"
        "  stats\n"
        "  save [file]   (mac dinh " DEFAULT_RULEFILE ")\n"
        "  load [file]\n"
        "Chay voi sudo khi gui lenh vao kernel. FWCTL_MOCK=1 de chay gia lap.\n");
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

static void row_cb(const struct fw_rule_entry *e, void *arg)
{
    print_rule_row(e);
    (*(int *)arg)++;
}

struct save_ctx {
    FILE *f;
    int n;
};

static void save_cb(const struct fw_rule_entry *e, void *arg)
{
    struct save_ctx *s = arg;

    print_rule_as_cmd(s->f, &e->rule);
    s->n++;
}

static int cmd_save(int n, char **a)
{
    const char *path = DEFAULT_RULEFILE;
    struct save_ctx s;

    if (n > 1) {
        fprintf(stderr, "Loi: dung 'save [file]'\n");
        return 1;
    }
    if (n == 1)
        path = a[0];
    s.f = fopen(path, "w");
    if (!s.f) {
        fprintf(stderr, "Loi: khong ghi duoc %s: %s\n", path, strerror(errno));
        return 1;
    }
    s.n = 0;
    fprintf(s.f, "# Luat fwctl. Moi dong la mot lenh. Them dong 'policy drop' neu can.\n");
    if (fwnl_list(save_cb, &s)) {
        fclose(s.f);
        return 1;
    }
    fclose(s.f);
    printf("Da luu %d luat vao %s\n", s.n, path);
    return 0;
}

static int cmd_load(int n, char **a)
{
    const char *path = DEFAULT_RULEFILE;
    int count = 0;

    if (n > 1) {
        fprintf(stderr, "Loi: dung 'load [file]'\n");
        return 1;
    }
    if (n == 1)
        path = a[0];

    /* Luot 0: chi kiem tra cu phap. Luot 1: xoa luat cu roi nap that. */
    for (int pass = 0; pass < 2; pass++) {
        char line[512];
        int lineno = 0, errors = 0;
        FILE *f = fopen(path, "r");

        if (!f) {
            fprintf(stderr, "Loi: khong mo duoc %s: %s\n", path, strerror(errno));
            return 1;
        }
        g_dry = (pass == 0);
        if (pass == 1 && fwnl_flush()) {
            fclose(f);
            return 1;
        }
        while (fgets(line, sizeof(line), f)) {
            char *tok[32];
            char *p;
            int nt = 0;

            lineno++;
            p = strchr(line, '#');
            if (p)
                *p = '\0';
            for (p = strtok(line, " \t\r\n"); p && nt < 32; p = strtok(NULL, " \t\r\n"))
                tok[nt++] = p;
            if (nt == 0)
                continue;
            if (!strcmp(tok[0], "load") || !strcmp(tok[0], "save")) {
                fprintf(stderr, "Loi: khong dung '%s' trong file (dong %d)\n", tok[0], lineno);
                errors++;
                continue;
            }
            if (run_command(tok[0], nt - 1, tok + 1)) {
                fprintf(stderr, "  ^ loi o dong %d cua %s\n", lineno, path);
                errors++;
            } else if (pass == 1) {
                count++;
            }
        }
        fclose(f);
        g_dry = 0;
        if (errors) {
            if (pass == 0)
                fprintf(stderr, "File co loi, chua thay doi gi trong kernel.\n");
            else
                fprintf(stderr, "Nap dang do, co loi khi gui vao kernel.\n");
            return 1;
        }
    }
    printf("Da nap %d lenh tu %s\n", count, path);
    return 0;
}

static int run_command(const char *cmd, int n, char **args)
{
    if (!strcmp(cmd, "add")) {
        struct fw_rule r;
        uint32_t id = 0;

        if (parse_add(n, args, &r))
            return 1;
        if (g_dry)
            return 0;
        if (fwnl_add(&r, &id))
            return 1;
        if (id)
            printf("Da them luat id=%u\n", id);
        else
            printf("Da them luat\n");
    } else if (!strcmp(cmd, "del")) {
        uint32_t id;

        if (parse_del(n, args, &id))
            return 1;
        if (g_dry)
            return 0;
        if (fwnl_del(id))
            return 1;
        printf("Da xoa luat id=%u\n", id);
    } else if (!strcmp(cmd, "list")) {
        int count = 0;

        if (no_args(cmd, n))
            return 1;
        if (g_dry)
            return 0;
        print_header();
        if (fwnl_list(row_cb, &count))
            return 1;
        if (count == 0)
            printf("(chua co luat nao)\n");
    } else if (!strcmp(cmd, "flush")) {
        if (no_args(cmd, n))
            return 1;
        if (g_dry)
            return 0;
        if (fwnl_flush())
            return 1;
        printf("Da xoa het luat\n");
    } else if (!strcmp(cmd, "policy")) {
        uint8_t pol;

        if (parse_policy(n, args, &pol))
            return 1;
        if (g_dry)
            return 0;
        if (fwnl_set_policy(pol))
            return 1;
        printf("Da dat policy mac dinh: %s\n", pol == FW_ACT_DROP ? "drop" : "accept");
    } else if (!strcmp(cmd, "stats")) {
        struct fw_stats s;

        if (no_args(cmd, n))
            return 1;
        if (g_dry)
            return 0;
        if (fwnl_stats(&s))
            return 1;
        print_stats(&s);
    } else if (!strcmp(cmd, "save")) {
        return cmd_save(n, args);
    } else if (!strcmp(cmd, "load")) {
        return cmd_load(n, args);
    } else {
        fprintf(stderr, "Lenh khong hop le: %s\n", cmd);
        usage();
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int rc;

    if (argc < 2) {
        usage();
        return 1;
    }
    rc = run_command(argv[1], argc - 2, argv + 2);
    fwnl_close();
    return rc;
}
