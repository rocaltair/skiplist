# skiplist

## description

[skiplist](https://en.wikipedia.org/wiki/Skip_list) is a data structure that allows fast search within an ordered sequence of elements. Fast search is made possible by maintaining a linked hierarchy of subsequences, with each successive subsequence skipping over fewer elements than the previous one. Searching starts in the sparsest subsequence until two consecutive elements have been found, one smaller and one larger than or equal to the element searched for. Via the linked hierarchy, these two elements link to elements of the next sparsest subsequence, where searching is continued until finally we are searching in the full sequence. The elements that are skipped over may be chosen probabilistically or deterministically, with the former being more common.

space O(N)
search O(logN)
insert O(logN)
delete O(logN)

see [zset in redis](https://github.com/antirez/redis/blob/3.0/src/t_zset.c)

## features
use score or your own function to compare elems

## benchmark

macOS 2.5GHz Intel Core i5

insert 1000k 1.2 seconds
search random 1000k 0.3 seconds

## lua-bind
see [test](lua-bind/test.lua)

## TODO
memory pool for slNode_t allocation
