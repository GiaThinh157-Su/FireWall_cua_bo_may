#include <stdio.h>
#include <string.h>

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

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return 1;
    }

    const char *cmd = argv[1];

    if (!strcmp(cmd, "add"))
        printf("[MOCK] se gui lenh ADD\n");
    else if (!strcmp(cmd, "del"))
        printf("[MOCK] se gui lenh DEL\n");
    else if (!strcmp(cmd, "list"))
        printf("[MOCK] se gui lenh LIST\n");
    else if (!strcmp(cmd, "flush"))
        printf("[MOCK] se gui lenh FLUSH\n");
    else if (!strcmp(cmd, "policy"))
        printf("[MOCK] se gui lenh SET_POLICY\n");
    else if (!strcmp(cmd, "stats"))
        printf("[MOCK] se gui lenh GET_STATS\n");
    else {
        fprintf(stderr, "Lenh khong hop le: %s\n", cmd);
        usage();
        return 1;
    }
    return 0;
}
