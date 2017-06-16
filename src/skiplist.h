#ifndef  _SKIPLIST_H_IOZX0MYF_
#define  _SKIPLIST_H_IOZX0MYF_

#if defined (__cplusplus)
extern "C" {
#endif

#define SKIPLIST_MAXLEVEL 32
#define SKIPLIST_P 0.25

#define SL_LVL_NEXT(node, l) ((node)->level[l].next)
#define SL_NEXT(node) ((node)->level[0].next)
#define SL_PREV(node) ((node)->prev)

#define SL_HEAD(sl) ((slNode_t *)&(sl->head))

#define SL_FIRST(sl) (sl->head.level[0].next)
#define SL_LAST(sl) ((slNode_t *)sl->tail)

#define SL_FOREACH_RANGE(sl, rankMin, rankMax, node, n) \
	for (node = slGetNodeByRank(sl, rankMin), n = 0; \
	     node != NULL && n <= rankMax - rankMin; \
	     node = SL_NEXT(node), n++)

#define SL_FOREACH(sl, node) \
	for (node = SL_NEXT(SL_HEAD(sl)); \
	     node != NULL; \
	     node = SL_NEXT(node))


struct slNode_s;
struct skiplist_s;

typedef struct slNode_s slNode_t;
typedef struct skiplist_s sl_t;

typedef void (*slFreeCb)(void *udata, void *ctx);
typedef int (*slCompareCb)(slNode_t *nodeA, slNode_t *nodeB, sl_t *sl, void *ctx);

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

struct slNodeMax_s {
	double score;
	void *udata;
	slNode_t *prev;
	int levelSize;
	struct levelNode_s level[SKIPLIST_MAXLEVEL];
};

struct skiplist_s {
	struct slNodeMax_s head;
	slNode_t *tail;
	int level;
	size_t size;
	slCompareCb comp;
	void *udata;
};

int slRandomLevel();

slNode_t * slCreateNode(int level, void *udata, double score);
void slFreeNode(slNode_t *node, slFreeCb freeCb, void *ctx);

/**
 * you can use slInit to init a struct pointer by yourself
 * */
void slInit(sl_t *sl);

/**
 * just free every node inside of sl, do not free sl;
 */ 
void slDestroy(sl_t *sl, slFreeCb freeCb, void *ctx);

/**
 * alloc and init sl;
 */
sl_t *slCreate();

/**
 * slDestroy and free sl;
 */
void slFree(sl_t *sl, slFreeCb freeCb, void *ctx);

/**
 * set your own comp function
 */
slCompareCb slSetCompareCb(sl_t *sl, slCompareCb comp);

/**
 * ctx would be passed to sl->comp function
 */
void slInsertNode(sl_t *sl, slNode_t *node, void *ctx);

/**
 * rank of the node
 * ctx would be passed to sl->comp function
 */
int slGetRank(sl_t *sl, slNode_t *node, void *ctx);

/**
 * size of sl
 */
int slGetSize(sl_t *sl);
slNode_t * slGetNodeByRank(sl_t *sl, int rank);

/**
 * delete node,
 * return 0 if succeed
 * call slFreeNode(node) if pNode == NULL and if node found in sl;
 * write &node to pNode if pNode != NULL and if node found in sl,
 * you should call slFreeNode by yourself;
 */
int slDeleteNode(sl_t *sl, slNode_t *node, void *ctx, slNode_t **pNode);

#define slDeleteByRank(sl, rank, freeCb, ctx) slDeleteByRankRange(sl, rank, rank, freeCb, ctx)

/**
 * delete rank range [rankMin, rankMax]
 * return deleted count
 * ctx would be passed to sl->comp function
 */
int slDeleteByRankRange(sl_t *sl,
		      int rankMin, int rankMax,
		      slFreeCb freeCb, void *ctx);

/**
 * greater or equal than score
 */
slNode_t * slFirstGEThan(sl_t *sl, double score);

/**
 * less or equal than score
 */
slNode_t * slLastLEThan(sl_t *sl, double score);

#if defined (__cplusplus)
}	/*end of extern "C"*/
#endif

#endif /* end of include guard:  _SKIPLIST_H_IOZX0MYF_ */


