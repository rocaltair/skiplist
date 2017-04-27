#include <stdio.h>
#include <stdlib.h>
#include "../src/skiplist.h"

#include <sys/time.h>
#include <unistd.h>

double timenow()
{
	struct timeval tm;
	gettimeofday(&tm, NULL);
	return tm.tv_sec * 1.0 + tm.tv_usec * 1e-6;
}

int main(int argc, char **argv)
{
	int i;
	slNode_t *pNode;
	sl_t *sl = slCreate();
	int totalSize = 100000000;

	double s;
	s = timenow();
	for (i = 0; i < totalSize; i++) {
		slNode_t *p = slCreateNode(slRandomLevel(), NULL, rand() % 10000000 * 0.01);
		slInsertNode(sl, p, NULL);
	}
	printf("insert %d time=%f\n", totalSize, timenow() - s);
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

	s = timenow();
	slDeleteByRankRange(sl, 1, 10000, NULL, NULL);
	printf("delete [1, 10000] time=%f\n", timenow() - s);

	s = timenow();
	for (i = 0; i < 10000; i++) {
		int rank = rand() % totalSize;
		slGetNodeByRank(sl, rank);
	}
	printf("random slGetNodeByRank [1, 10000] time=%f\n", timenow() - s);

	printf("after delete rank range[2, 3]\n");
	printf("sl size=%d\n", slGetSize(sl));
	SL_FOREACH_RANGE(sl, 1, 5, pNode, i) {
		printf("i=%d, p=%p %lf\n", i, (void *)pNode, pNode->score);
	}
	sleep(1000);
	slFree(sl, NULL, NULL);
	return 0;
}
