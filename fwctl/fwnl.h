#ifndef FWNL_H
#define FWNL_H

#include <stdint.h>
#include "fw_cmd_draft.h"

typedef void (*rule_cb)(const struct fw_rule_entry *e, void *arg);

/* Tat ca tra 0 neu OK, -1 neu loi (da in thong bao ra stderr) */
int fwnl_add(const struct fw_rule *r, uint32_t *id);
int fwnl_del(uint32_t id);
int fwnl_list(rule_cb cb, void *arg);
int fwnl_flush(void);
int fwnl_set_policy(uint8_t pol);
int fwnl_stats(struct fw_stats *s);
void fwnl_close(void);

#endif
