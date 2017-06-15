# skiplist

## Description

[skiplist](https://en.wikipedia.org/wiki/Skip_list) is a data structure that allows fast search within an ordered sequence of elements. Fast search is made possible by maintaining a linked hierarchy of subsequences, with each successive subsequence skipping over fewer elements than the previous one. Searching starts in the sparsest subsequence until two consecutive elements have been found, one smaller and one larger than or equal to the element searched for. Via the linked hierarchy, these two elements link to elements of the next sparsest subsequence, where searching is continued until finally we are searching in the full sequence. The elements that are skipped over may be chosen probabilistically or deterministically, with the former being more common.

## Features
use score or your own function to compare elems

## VS
### skiplist VS qsort
skiplist takes 5x times as qsort;

2x space as array (qsort);

### skiplist VS RBTree(Red-black Tree)
insert, delete, search : both O(logN)

skiplist takes a little more space than RBTree,
but offers a feature with *rank of elem* with O(logN)

## Space and Time
space : O(N)

search one: O(logN)

search range [min, max]: O(logN + max - min)

get rank of data: O(logN)

insert one: O(logN)

delete one: O(logN)

see [zset in redis](https://github.com/antirez/redis/blob/3.0/src/t_zset.c)

## Benchmark

macOS 2.5GHz Intel Core i5

insert 1000k 1.2 seconds

search random 1000k 0.3 seconds

## API for C

### int slRandomLevel();

### slNode_t * slCreateNode(int level, void *udata, double score);
### void slFreeNode(slNode_t *node, slFreeCb freeCb, void *ctx);

### void slInit(sl_t *sl);
you can use slInit to init a struct pointer by yourself

### void slDestroy(sl_t *sl, slFreeCb freeCb, void *ctx);
just free every node inside of sl, do not free sl;

### sl_t *slCreate();
alloc and init sl;

### void slFree(sl_t *sl, slFreeCb freeCb, void *ctx);
slDestroy and free sl;

### slCompareCb slSetCompareCb(sl_t *sl, slCompareCb comp);
set your own comp function

### void slInsertNode(sl_t *sl, slNode_t *node, void *ctx);
ctx would be passed to sl->comp function

### int slGetRank(sl_t *sl, slNode_t *node, void *ctx);
rank of the node;

ctx would be passed to sl->comp function


### int slGetSize(sl_t *sl);
size of sl

### slNode_t * slGetNodeByRank(sl_t *sl, int rank);

### int slDeleteNode(sl_t *sl, slNode_t *node, void *ctx, slNode_t **pNode);
delete node;

return 0 if succeed;

call slFreeNode(node) if pNode == NULL and if node found in sl;

write &node to pNode if pNode != NULL and if node found in sl,

you should call slFreeNode by yourself;


### slDeleteByRank(sl, rank, freeCb, ctx)

### int slDeleteByRankRange(sl_t *sl, int rankMin, int rankMax, slFreeCb freeCb, void *ctx);
delete rank range [rankMin, rankMax];

return deleted count;

ctx would be passed to sl->comp function;

### slNode_t * slFirstGEThan(sl_t *sl, double score);
greater or equal than score;


### slNode_t * slLastLEThan(sl_t *sl, double score);
less or equal than score;

## lua-bind

see [test](lua-bind/test.lua)
see [benchemark](lua-bind/benchmark.lua)

## Benchmark for lua-bind

on macOS serria 10.12.2, macbook Mid-2012, 2.5GHz Intel Core i5

```
insert, create() size=100000, time=0.74824
update, sl:size() == 100000, update cnt=100000 time=1.43351
rank_range sl:size() == 100000,cnt=100000(x, x+50) time=1.07677
rank_of sl:size() == 100000,cnt=100000,time=0.96903
get_by_rank sl:size() == 100000,cnt=100000,time=0.11332
delete sl:size() == 100000,cnt=100000,time=0.69354
```

## API for Lua

### lskiplist.new(comp_func)
create a skiplist
```
example:

local levelMap = {...}

-- ret < 0 : higher priority of a for a and b
-- ret == 0 same priority for a and b
-- ret > 0 lower priority of a for a and b
function comp_func(a, b, scoreA, scoreB, node_ptr_diff)
	if scoreA ~= scoreB then
		return scoreA - scoreB
	end
	if levelMap[a] ~= levelMap[b] then
		return levelMap[a] - levelMap[b]
	end
	return node_ptr_diff
end
sl = lskiplist:new(comp_func)
```

### sl:insert(data, score)
score : default == 0

### sl:update(data, score)
if none or nil for score, delete data;
insert if data does exist;
if data exists, delete data first, and then update score, insert at last;

### sl[data] = score
it's the same with sl:update(data, score)

### sl:delete(data)

### sl[data] = nil
it's the same with sl:delete(data) or sl:update(data, nil)

### sl:exists(data)
return true if exists, or false if not

### sl:get_by_rank(rank)
return data, score if rank exists

### sl:del_by_rank(rank)

### sl:del_by_rank_range(rankMin, rankMax)

### sl:rank_of(data)
return rank of data

### sl:rank_range(rankMin, rankMax)
return an table {[rankMin] = data1, [rankMin + 1] = data2, ..., [rankMax] = dataN}

### sl:get_score(data)
it's the same with sl[data]

### sl:score_range(scoreMin, scoreMax)
return map, rankMin

map = {[rankMin] = data1, [rankMin + 1] = data2, ..., [rankMax] = dataN}

### sl:next(data)
return next value for rank of data

### sl:prev()
return prev value for rank of data

### sl:size()
sizeof sl

### #sl
it's the same with sl:size()

### sl:rank_pairs(rankMin[, rankMax])
iterator for sl;

rankMax : default == sl:size()

example:
```
for rank, data, score in sl:rank_pairs(rankMin, rankMax) do
	print(rank, data, score)
end
```

## TODO
memory pool for slNode_t allocation
