#include "diff.h"
#include "node.h"

void cui_diff_run(cui_node *declared, cui_node **retained) {
	(void)declared;
	if (retained) *retained = declared;
}
