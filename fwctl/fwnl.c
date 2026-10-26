#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include "fwnl.h"

struct ctx {
    rule_cb cb;
    void *arg;
    uint32_t id;
    int have_id;
    struct fw_stats stats;
    int have_stats;
};

static struct nl_sock *sk;
static int family;
static int mock;

static int ensure(void)
{
    if (sk || mock)
        return 0;
    if (getenv("FWCTL_MOCK")) {
        mock = 1;
        return 0;
    }
    sk = nl_socket_alloc();
    if (!sk) {
        fprintf(stderr, "Loi: khong tao duoc socket Netlink\n");
        return -1;
    }
    if (genl_connect(sk)) {
        fprintf(stderr, "Loi: khong ket noi duoc Generic Netlink\n");
        nl_socket_free(sk);
        sk = NULL;
        return -1;
    }
    family = genl_ctrl_resolve(sk, FW_GENL_NAME);
    if (family < 0) {
        fprintf(stderr, "Loi: khong thay family '%s'. Da nap module kernel chua? (sudo insmod ...)\n",
                FW_GENL_NAME);
        nl_socket_free(sk);
        sk = NULL;
        return -1;
    }
    return 0;
}

static int on_msg(struct nl_msg *msg, void *arg)
{
    struct ctx *c = arg;
    struct nlattr *a[FW_ATTR_MAX + 1];

    if (genlmsg_parse(nlmsg_hdr(msg), 0, a, FW_ATTR_MAX, NULL) < 0)
        return NL_SKIP;

    if (a[FW_ATTR_ID]) {
        c->id = nla_get_u32(a[FW_ATTR_ID]);
        c->have_id = 1;
    }
    if (a[FW_ATTR_STATS] && nla_len(a[FW_ATTR_STATS]) >= (int)sizeof(struct fw_stats)) {
        memcpy(&c->stats, nla_data(a[FW_ATTR_STATS]), sizeof(struct fw_stats));
        c->have_stats = 1;
    }
    if (a[FW_ATTR_RULE_ENTRY] && c->cb &&
        nla_len(a[FW_ATTR_RULE_ENTRY]) >= (int)sizeof(struct fw_rule_entry)) {
        struct fw_rule_entry e;
        memcpy(&e, nla_data(a[FW_ATTR_RULE_ENTRY]), sizeof(e));
        c->cb(&e, c->arg);
    }
    return NL_OK;
}

static int do_cmd(int cmd, int flags, int attr, const void *data, int len, struct ctx *c)
{
    struct nl_msg *msg;
    int err;

    if (ensure())
        return -1;

    msg = nlmsg_alloc();
    if (!msg) {
        fprintf(stderr, "Loi: het bo nho\n");
        return -1;
    }
    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, family, 0, flags, cmd, FW_GENL_VERSION) ||
        (attr && nla_put(msg, attr, len, data) < 0)) {
        fprintf(stderr, "Loi: khong dong goi duoc thong diep Netlink\n");
        nlmsg_free(msg);
        return -1;
    }

    nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, on_msg, c);

    err = nl_send_auto(sk, msg);
    nlmsg_free(msg);
    if (err < 0) {
        fprintf(stderr, "Loi gui lenh: %s\n", nl_geterror(err));
        return -1;
    }
    err = nl_recvmsgs_default(sk);
    if (err < 0) {
        fprintf(stderr, "Loi tu kernel: %s (neu bi tu choi quyen, chay lai voi sudo)\n",
                nl_geterror(err));
        return -1;
    }
    return 0;
}

#define F_CMD (NLM_F_REQUEST | NLM_F_ACK)

int fwnl_add(const struct fw_rule *r, uint32_t *id)
{
    struct ctx c;

    memset(&c, 0, sizeof(c));
    if (ensure())
        return -1;
    if (mock) {
        *id = 1;
        return 0;
    }
    if (do_cmd(FW_CMD_ADD, F_CMD, FW_ATTR_RULE, r, sizeof(*r), &c))
        return -1;
    *id = c.have_id ? c.id : 0;
    return 0;
}

int fwnl_del(uint32_t id)
{
    struct ctx c;

    memset(&c, 0, sizeof(c));
    if (ensure())
        return -1;
    if (mock)
        return 0;
    return do_cmd(FW_CMD_DEL, F_CMD, FW_ATTR_ID, &id, sizeof(id), &c);
}

int fwnl_flush(void)
{
    struct ctx c;

    memset(&c, 0, sizeof(c));
    if (ensure())
        return -1;
    if (mock)
        return 0;
    return do_cmd(FW_CMD_FLUSH, F_CMD, 0, NULL, 0, &c);
}

int fwnl_set_policy(uint8_t pol)
{
    struct ctx c;

    memset(&c, 0, sizeof(c));
    if (ensure())
        return -1;
    if (mock)
        return 0;
    return do_cmd(FW_CMD_SET_POLICY, F_CMD, FW_ATTR_POLICY, &pol, sizeof(pol), &c);
}

static void mock_list(rule_cb cb, void *arg)
{
    struct fw_rule_entry e;

    memset(&e, 0, sizeof(e));
    e.rule.id = 1;
    e.rule.proto = FW_PROTO_TCP;
    e.rule.action = FW_ACT_DROP;
    inet_pton(AF_INET, "192.168.1.0", &e.rule.src_ip);
    inet_pton(AF_INET, "255.255.255.0", &e.rule.src_mask);
    e.rule.sport_max = 65535;
    e.rule.dport_min = e.rule.dport_max = 80;
    e.packets = 12;
    e.bytes = 720;
    cb(&e, arg);

    memset(&e, 0, sizeof(e));
    e.rule.id = 2;
    e.rule.proto = FW_PROTO_ICMP;
    e.rule.action = FW_ACT_DROP;
    e.rule.direction = FW_DIR_IN;
    e.rule.sport_max = 65535;
    e.rule.dport_max = 65535;
    e.packets = 3;
    e.bytes = 252;
    cb(&e, arg);
}

int fwnl_list(rule_cb cb, void *arg)
{
    struct ctx c;

    memset(&c, 0, sizeof(c));
    c.cb = cb;
    c.arg = arg;
    if (ensure())
        return -1;
    if (mock) {
        mock_list(cb, arg);
        return 0;
    }
    return do_cmd(FW_CMD_LIST, NLM_F_REQUEST | NLM_F_DUMP, 0, NULL, 0, &c);
}

int fwnl_stats(struct fw_stats *s)
{
    struct ctx c;

    memset(&c, 0, sizeof(c));
    if (ensure())
        return -1;
    if (mock) {
        s->total_packets = 1000;
        s->accepted_packets = 950;
        s->dropped_packets = 50;
        s->active_connections = 7;
        return 0;
    }
    if (do_cmd(FW_CMD_GET_STATS, F_CMD, 0, NULL, 0, &c))
        return -1;
    if (!c.have_stats) {
        fprintf(stderr, "Loi: kernel khong tra ve thong ke\n");
        return -1;
    }
    *s = c.stats;
    return 0;
}

void fwnl_close(void)
{
    if (sk) {
        nl_socket_free(sk);
        sk = NULL;
    }
}
