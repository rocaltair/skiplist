#include <stdlib.h>
#include <float.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include "skiplist.h"

#define ENABLE_SL_DEBUG 0
#if (ENABLE_SL_DEBUG > 0)
# define DLOG(fmt, ...) fprintf(stderr, "<skiplist>" fmt "\n", ##__VA_ARGS__)
#else
# define DLOG(...)
#endif


static void slInitNode(slNode_t *node, int level, void *udata, double score);
static int internalComp(slNode_t *nodeA, slNode_t *nodeB, sl_t *sl, void *ctx);
static void slDeleteNodeUpdate(sl_t *sl, slNode_t *node, slNode_t **update);

int slRandomLevel()
{
	int level = 1;
	while((rand() & 0xffff) < (SKIPLIST_P * 0xffff))
		level += 1;
	return (level < SKIPLIST_MAXLEVEL) ? level : SKIPLIST_MAXLEVEL;
}

static int internalComp(slNode_t *nodeA, slNode_t *nodeB, sl_t *sl, void *ctx)
{
	ptrdiff_t d;
	if (nodeA->score != nodeB->score)
		return nodeA->score - nodeB->score < 0 ? -1 : 1;

	d = (const char *)(nodeA->udata == NULL ? nodeA : nodeA->udata)
			- (const char *)(nodeB->udata == NULL ? nodeB : nodeB->udata);

	if (d == 0)
		return 0;
	return d < 0 ? -1 : 1;
}

void slInit(sl_t *sl)
{
	sl->level = 1;
	sl->size = 0;
	sl->tail = NULL;
	sl->comp = internalComp;
	slInitNode(SL_HEAD(sl), SKIPLIST_MAXLEVEL, NULL, DBL_MIN);
}

sl_t * slCreate()
{
	sl_t *sl = malloc(sizeof(*sl));
	if (sl == NULL) {
		return NULL;
	}
	slInit(sl);
	sl->udata = NULL;
	return sl;
}

slCompareCb slSetCompareCb(sl_t *sl, slCompareCb comp)
{
	slCompareCb old = sl->comp;
	sl->comp = comp;
	return old;
}

void slDestroy(sl_t *sl, slFreeCb freeCb, void *ctx)
{
	slNode_t *node;
	slNode_t *next;
	for (node = SL_FIRST(sl); node != NULL; node = next) {
		next = node->level[0].next;
		slFreeNode(node, freeCb, ctx);
	}
}

void slFree(sl_t *sl, slFreeCb freeCb, void *ctx)
{
	slDestroy(sl, freeCb, ctx);
	free(sl);
}

static void slInitNode(slNode_t *node, int level, void *udata, double score)
{
	int i;
	node->score = score;
	node->udata = udata;
	node->prev = NULL;
	node->levelSize = level;
	for (i = 0; i < level; i++) {
		node->level[i].next = NULL;
		node->level[i].span = 0;
	}
}

slNode_t * slCreateNode(int level, void *udata, double score)
{
	size_t node_sz = sizeof(slNode_t) + sizeof(struct levelNode_s) * (level - 1);
	slNode_t *node = malloc(node_sz);
	if (node == NULL)
		goto finished;
	slInitNode(node, level, udata, score);
finished:
	return node;
}

void slFreeNode(slNode_t *node, slFreeCb freeCb, void *ctx)
{
	if (freeCb != NULL) {
		freeCb(node->udata, ctx);
	}
	free(node);
}

void slInsertNode(sl_t *sl, slNode_t *node, void *ctx)
{
	int level;
	slNode_t *update[SKIPLIST_MAXLEVEL];
	slNode_t *p;
	int rank[SKIPLIST_MAXLEVEL];
	int i;
	p = SL_HEAD(sl);
	for (i = sl->level - 1; i >= 0; i--) {
        	rank[i] = i == (sl->level-1) ? 0 : rank[i+1];
		while (p->level[i].next != NULL
			&& (sl->comp(p->level[i].next, node, sl, ctx) < 0)) {
			rank[i] += p->level[i].span;
			p = p->level[i].next;
		}
		update[i] = p;
	}
	level = node->levelSize;
	if (level > sl->level) {
		for (i = sl->level; i < level; i++) {
			rank[i] = 0;
			update[i] = SL_HEAD(sl);
			update[i]->level[i].span = sl->size;
		}
		sl->level = level;
	}
	for (i = 0; i < level; i++) {
		node->level[i].next = update[i]->level[i].next;
		update[i]->level[i].next = node;

        	node->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
		update[i]->level[i].span = rank[0] - rank[i] + 1;
	}
	for (i = level; i < sl->level; i++) {
		update[i]->level[i].span++;
	}
	node->prev = (update[0] == SL_HEAD(sl)) ? NULL : update[0];
	if (node->level[0].next)
		node->level[0].next->prev = node;
	else
		sl->tail = node;
	sl->size++;
}

static void slDeleteNodeUpdate(sl_t *sl, slNode_t *node, slNode_t **update)
{
	int i;
	slNode_t *header;
	for (i = 0; i < sl->level; i++) {
		if (update[i]->level[i].next == node) {
			update[i]->level[i].span += node->level[i].span - 1;
			update[i]->level[i].next = node->level[i].next;
		} else {
			update[i]->level[i].span -= 1;
		}
	}
	if (node->level[0].next != NULL) {
		node->level[0].next->prev = node->prev;
	} else {
		sl->tail = node->prev;
	}
	header = SL_HEAD(sl);
	while(sl->level > 1 && header->level[sl->level-1].next == NULL)
		sl->level--;
	sl->size--;
}

/**
 * call slFreeNode by yourself after slDeleteNode if pNode == NULL
 */
int slDeleteNode(sl_t *sl, slNode_t *node, void *ctx, slNode_t **pNode)
{
	int i;
	slNode_t *p;
	slNode_t *next;

	slNode_t *update[SKIPLIST_MAXLEVEL];
	p = SL_HEAD(sl);
	for (i = sl->level - 1; i >= 0; i--) {
		while (p->level[i].next != NULL &&
		       sl->comp(p->level[i].next, node, sl, ctx) < 0) {
			p = p->level[i].next;
		}
		update[i] = p;
	}
	next = p->level[0].next;
	if (next != node) {
		DLOG("delete error,p=%p,node=%p,next=%p\n", (void *)p, (void *)node, (void *)next);
		return -1;
	}
	p = next;
	slDeleteNodeUpdate(sl, p, update);
	if (pNode != NULL)
		*pNode = p;
	else
		slFreeNode(p, NULL, NULL);
	return 0;
}

slNode_t * slGetNodeByRank(sl_t *sl, int rank)
{
	int traversed = 0;
	slNode_t *p;
	int i;
	p = SL_HEAD(sl);
	for (i = sl->level - 1; i >= 0; i--) {
		while (p->level[i].next != NULL && p->level[i].span + traversed <= rank) {
			traversed += p->level[i].span;
			p = p->level[i].next;
		}
		if (traversed == rank) {
			return p;
		}
	}
	return NULL;
}

int slGetNodesRankRange(sl_t *sl,
			int rankMin, int rankMax,
			slNode_t **nodeArr, int sz)
{
	int n = 0;
	slNode_t *p;
	SL_FOREACH_RANGE(sl, rankMin, rankMax, p, n) {
		nodeArr[n] = p;
		if (n + 1 >= sz)
			break;
	}
	return n;
}

int slGetSize(sl_t *sl)
{
	return sl->size;
}

int slGetRank(sl_t *sl, slNode_t *node, void *ctx)
{
	int traversed = 0;
	slNode_t *p;
	int i;
	if (node == NULL)
		return 0;
	p = SL_HEAD(sl);
	for (i = sl->level - 1; i >= 0; i--) {
		while (p->level[i].next != NULL &&
			sl->comp(p->level[i].next, node, sl, ctx) <= 0) {
			traversed += p->level[i].span;
			p = p->level[i].next;
		}
		if (sl->comp(p, node, sl, ctx) == 0) {
			return traversed;
		}
	}
	return 0;
}

int slDeleteByRankRange(sl_t *sl, int rankMin, int rankMax, slFreeCb freeCb, void *ctx)
{
	slNode_t *update[SKIPLIST_MAXLEVEL], *node;
	int traversed = 0;
	int removed = 0;
	int i;

	assert(1 <= rankMin && rankMin <= rankMax && rankMin <= sl->size);
	assert(1 <= rankMax && rankMin <= rankMax && rankMax <= sl->size);

	node = SL_HEAD(sl);
	for (i = sl->level-1; i >= 0; i--) {
		while (node->level[i].next && (traversed + node->level[i].span) < rankMin) {
			traversed += node->level[i].span;
			node = node->level[i].next;
		}
		update[i] = node;
	}

	traversed++;
	node = node->level[0].next;
	while (node && traversed <= rankMax) {
		slNode_t *next = node->level[0].next;
		slDeleteNode(sl, node, NULL, update);
		slFreeNode(node, freeCb, ctx);
		removed++;
		traversed++;
		node = next;
	}
	return removed;
}

/**
 * greater or equal than score
 */
slNode_t * slFirstGEThan(sl_t *sl, double score)
{
	int i;
	slNode_t *p;
	p = SL_HEAD(sl);
	for (i = sl->level - 1; i >= 0; i--) {
		while (p->level[i].next != NULL &&
		       p->level[i].next->score < score) {
			p = p->level[i].next;
		}
	}
	return p->level[0].next;
}

/**
 * less or equal than score
 */
slNode_t * slLastLEThan(sl_t *sl, double score)
{
	int i;
	slNode_t *p;
	p = SL_HEAD(sl);
	for (i = sl->level - 1; i >= 0; i--) {
		while (p->level[i].next != NULL &&
		       p->level[i].next->score <= score) {
			p = p->level[i].next;
		}
	}
	return p;
}

