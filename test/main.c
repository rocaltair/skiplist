#include <stdio.h>
#include <stdlib.h>
#include "../src/skiplist.h"

int main(int argc, char **argv)
{
	int i;
	slNode_t *pNode;
	sl_t *sl = slCreate();
	for (i = 0; i < 1000000; i++) {
		slNode_t *p = slCreateNode(slRandomLevel(), NULL, rand() % 10000000 * 0.01);
		slInsertNode(sl, p, NULL);
	}
	printf("size of node %lu\n", sizeof(*pNode));
	printf("sl size=%d\n", slGetSize(sl));
	SL_FOREACH_RANGE(sl, 1, 4, pNode, i) {
		printf("i=%d, p=%p %lf\n", i, (void *)pNode, pNode->score);
	}
	pNode = slGetNodeByRank(sl, 2);
	printf("to delete i=2, p=%p %lf\n", (void *)pNode, pNode->score);
	printf("rank = %d\n", slGetRank(sl, pNode, NULL));
	slDeleteNode(sl, pNode, NULL, NULL);
	SL_FOREACH_RANGE(sl, 1, 5, pNode, i) {
		printf("i=%d, p=%p %lf\n", i, (void *)pNode, pNode->score);
	}
	slDeleteByRankRange(sl, 2, 3, NULL, NULL);
	printf("after delete rank range[2, 3]\n");
	printf("sl size=%d\n", slGetSize(sl));
	SL_FOREACH_RANGE(sl, 1, 5, pNode, i) {
		printf("i=%d, p=%p %lf\n", i, (void *)pNode, pNode->score);
	}
	slFree(sl, NULL, NULL);
	return 0;
}
