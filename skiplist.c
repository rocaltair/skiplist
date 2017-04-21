#include <stdlib.h>
#include <float.h>
#include <stddef.h>
#include "skiplist.h"


struct skiplist_s {
	slNode_t *head;
	slNode_t *tail;
	slCompareCb comp;
	int level;
	size_t size;
};

int slRandomLevel()
{
	int level = 1;
	while((random() & 0xffff) < (SKIPLIST_P * 0xffff))
		level += 1;
	return (level < SKIPLIST_MAXLEVEL) ? level : SKIPLIST_MAXLEVEL;
}

static void slDeleteNodeUpdate(sl_t *sl, slNode_t *node, slNode_t **update);

static int internalComp(slNode_t *nodeA, slNode_t *nodeB, void *ctx)
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

sl_t * slCreate()
{
	sl_t *sl = malloc(sizeof(*sl));
	if (sl == NULL)
		goto finished;
	sl->head = slCreateNode(SKIPLIST_MAXLEVEL, NULL, DBL_MIN);
	if (sl->head == NULL) {
		free(sl);
		sl = NULL;
		goto finished;
	}
	sl->level = 1;
	sl->size = 0;
	sl->head->prev = NULL;
	sl->tail = NULL;
	sl->comp = internalComp;
finished:
	return sl;
}

slCompareCb slSetCompareCb(sl_t *sl, slCompareCb comp)
{
	slCompareCb old = sl->comp;
	sl->comp = comp;
	return old;
}

void slFree(sl_t *sl, slFreeCb freeCb, void *ctx)
{
	slNode_t *node;
	slNode_t *next;
	for (node = sl->head; node != sl->tail; node = next) {
		next = node->level[0].next;
		slFreeNode(node, freeCb, ctx);
	}
	free(sl);
}

slNode_t * slCreateNode(int level, void *udata, double score)
{
	int i;
	slNode_t *node = malloc(sizeof(*node) + sizeof(struct levelNode_s) * (level - 1));
	if (node == NULL)
		goto finished;

	node->score = score;
	node->udata = udata;
	node->prev = NULL;
	node->levelSize = level;
	for (i = 0; i < level; i++) {
		node->level[i].next = NULL;
		node->level[i].span = 0;
	}
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
	p = sl->head;
	for (i = sl->level - 1; i >= 0; i--) {
        	rank[i] = i == (sl->level-1) ? 0 : rank[i+1];
		while (p->level[i].next != NULL
			&& (sl->comp(p->level[i].next, node, ctx) < 0)) {
			rank[i] += p->level[i].span;
			p = p->level[i].next;
		}
		update[i] = p;
	}
	level = node->levelSize;
	if (level > sl->level) {
		for (i = sl->level; i < level; i++) {
			rank[i] = 0;
			update[i] = sl->head;
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
	node->prev = (update[0] == sl->head) ? NULL : update[0];
	if (node->level[0].next)
		node->level[0].next->prev = node;
	else
		sl->tail = node;
	sl->size++;
}

static void slDeleteNodeUpdate(sl_t *sl, slNode_t *node, slNode_t **update)
{
	int i;
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
	while(sl->level > 1 && sl->head->level[sl->level-1].next == NULL)
		sl->level--;
	sl->size--;
}

/**
 * call slFreeNode by yourself after slDeleteNode
 */
int slDeleteNode(sl_t *sl, slNode_t *node, void *ctx, slNode_t **pNode)
{
	int i;
	slNode_t *p;
	slNode_t *update[SKIPLIST_MAXLEVEL];
	p = sl->head;
	for (i = sl->level - 1; i >= 0; i--) {
		while (p->level[i].next != NULL &&
		       sl->comp(p->level[i].next, node, ctx) < 0) {
			p = p->level[i].next;
		}
		update[i] = p;
	}
	p = p->level[0].next;
	if (p != node)
		return -1;
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
	p = sl->head;
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
		return sl->size + 1;
	p = sl->head;
	for (i = sl->level - 1; i >= 0; i--) {
		while (p->level[i].next != NULL &&
			sl->comp(p->level[i].next, node, ctx) <= 0) {
			traversed += p->level[i].span;
			p = p->level[i].next;
		}
		if (sl->comp(p, node, ctx) == 0) {
			return traversed;
		}
	}
	return sl->size;
}

int slDeleteRankRange(sl_t *sl, int rankBegin, int rankEnd, slFreeCb cb, void *ctx)
{
	return 0;
}

slNode_t * slFirstInRange(sl_t *sl, double min, double max)
{
	return NULL;
}

slNode_t * slLastInRange(sl_t *sl, double min, double max)
{
	return NULL;
}
