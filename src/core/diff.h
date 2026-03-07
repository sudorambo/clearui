#ifndef CLEARUI_DIFF_H
#define CLEARUI_DIFF_H

struct cui_node;

void cui_diff_run(struct cui_node *declared, struct cui_node **retained);

#endif
