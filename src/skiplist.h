#ifndef  _SKIPLIST_H_IOZX0MYF_
#define  _SKIPLIST_H_IOZX0MYF_

#if defined (__cplusplus)
extern "C" {
#endif

#define SKIPLIST_MAXLEVEL 32
#define SKIPLIST_P 0.25

#define SL_FOREACH_RANGE(sl, rankMin, rankMax, node, n) \
	for (node = slGetNodeByRank(sl, rankMin), n = 0; \
	     node != NULL && n <= rankMax - rankMin; \
	     node = node->level[0].next, n++)

#define SL_FOREACH(sl, node, n) \
	for (node = slGetNodeByRank(sl, 0), n = 0; \
	     node != NULL; \
	     node = node->level[0].next, n++)

struct slNode_s;
struct skiplist_s;

typedef struct slNode_s slNode_t;
typedef struct skiplist_s sl_t;

typedef void (*slFreeCb)(void *udata, void *ctx);
typedef int (*slCompareCb)(slNode_t *nodeA, slNode_t *nodeB, void *ctx);

struct levelNode_s {
	slNode_t *next;
	size_t span;
};

struct slNode_s {
	double score;
	void *udata;
	slNode_t *prev;
	int levelSize;
	struct levelNode_s level[1];
};

struct skiplist_s {
	slNode_t *head;
	slNode_t *tail;
	int level;
	size_t size;
	slCompareCb comp;
	void *udata;
};

int slRandomLevel();

slNode_t * slCreateNode(int level, void *udata, double score);
void slFreeNode(slNode_t *node, slFreeCb freeCb, void *ctx);

int slInit(sl_t *sl);
void slDestroy(sl_t *sl, slFreeCb freeCb, void *ctx);

sl_t *slCreate();
void slFree(sl_t *sl, slFreeCb freeCb, void *ctx);

slCompareCb slSetCompareCb(sl_t *sl, slCompareCb comp);

void slInsertNode(sl_t *sl, slNode_t *node, void *ctx);
int slDeleteNode(sl_t *sl, slNode_t *node, void *ctx, slNode_t **pNode);
int slGetRank(sl_t *sl, slNode_t *node, void *ctx);
int slGetSize(sl_t *sl);
slNode_t * slGetNodeByRank(sl_t *sl, int rank);

#if defined (__cplusplus)
}	/*end of extern "C"*/
#endif

#endif /* end of include guard:  _SKIPLIST_H_IOZX0MYF_ */


