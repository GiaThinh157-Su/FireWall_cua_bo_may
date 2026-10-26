#ifndef PARSE_H
#define PARSE_H
#include "fw_common_draft.h"

/* Tra 0 neu OK, -1 neu loi (da in thong bao loi ra stderr) */
int parse_add(int argc, char **argv, struct fw_rule *r);

#endif
