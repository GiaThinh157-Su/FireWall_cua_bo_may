#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>
#include "fw_cmd_draft.h"

void print_header(void);
void print_rule_row(const struct fw_rule_entry *e);
void print_stats(const struct fw_stats *s);
/* Ghi mot luat duoi dang dong lenh "add ..." (dung cho save) */
void print_rule_as_cmd(FILE *f, const struct fw_rule *r);

#endif
