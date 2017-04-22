#include <stdio.h>
#include <stdlib.h>
#include "../src/skiplist.h"

int main(int argc, char **argv)
{
	int i;
	slNode_t *pNode;
	sl_t *sl = slCreate();
	for (i = 0; i < 1000000; i++) {
		slNode_t *p = slCreateNode(slRandomLevel(), NULL, rand() % 1000 * 0.1);
		slInsertNode(sl, p, NULL);
	}
	printf("size of node %lu\n", sizeof(*pNode));
	printf("sl size=%d\n", slGetSize(sl));
	SL_FOREACH_RANGE(sl, 0, 4, pNode, i) {
		printf("i=%d, p=%p %lf\n", i, (void *)pNode, pNode->score);
	}
	pNode = slGetNodeByRank(sl, 2);
	printf("to delete i=2, p=%p %lf\n", (void *)pNode, pNode->score);
	printf("rank = %d\n", slGetRank(sl, pNode, NULL));
	slDeleteNode(sl, pNode, NULL, NULL);
	printf("sl size=%d\n", slGetSize(sl));
	SL_FOREACH_RANGE(sl, 0, 4, pNode, i) {
		printf("i=%d, p=%p %lf\n", i, (void *)pNode, pNode->score);
	}
	slFree(sl, NULL, NULL);
	return 0;
}
