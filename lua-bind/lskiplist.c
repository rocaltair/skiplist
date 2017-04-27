#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include "skiplist.h"

#if LUA_VERSION_NUM < 502
# ifndef luaL_newlib
#  define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
# endif
# ifndef lua_setuservalue
#  define lua_setuservalue(L, n) lua_setfenv(L, n)
# endif
# ifndef lua_getuservalue
#  define lua_getuservalue(L, n) lua_getfenv(L, n)
# endif
#endif

#define ENABLE_LSL_INDEX 1

#define CLASS_SKIPLIST "cls{skiplist}"
#define CHECK_SL(L, n) ((sl_t *)luaL_checkudata(L, n, CLASS_SKIPLIST))

static slNode_t *luac__get_node(lua_State *L, int sl_idx, int node_idx)
{
	sl_t *sl;

	int top = lua_gettop(L);

	sl = CHECK_SL(L, sl_idx);

	if (sl_idx < 0)
		sl_idx = top + sl_idx + 1;
	if (node_idx < 0)
		node_idx = top + node_idx + 1;

	luaL_argcheck(L, !lua_isnoneornil(L, node_idx), node_idx,
		      "number|string|table|udata required!");

	lua_getuservalue(L, sl_idx);
	lua_getfield(L, -1, "node_map");

	lua_pushvalue(L, node_idx);
	lua_rawget(L, -2);
	if (lua_isuserdata(L, -1)) {
		slNode_t *p = lua_touserdata(L, -1);
		lua_settop(L, top);
		return p;
	}
	lua_settop(L, top);
	return NULL;
}

static int comp(slNode_t *nodeA, slNode_t *nodeB, void *ctx)
{
	lua_State *L = ctx;
	sl_t *sl = CHECK_SL(L, 1);
	int top = lua_gettop(L);
	ptrdiff_t diff;
	int idiff;
	int fret, ret;

	(void)sl;
	lua_checkstack(L, 9);
	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "value_map");
	lua_getfield(L, -2, "comp_func"); 	/*value_map, comp_func*/
		lua_pushlightuserdata(L, nodeA);
		lua_rawget(L, -3);		/*value_map, comp_func, valueA*/

		lua_pushlightuserdata(L, nodeB);
		lua_rawget(L, -4);		/*value_map, comp_func, valueA, valueB*/

	lua_pushnumber(L, nodeA->score);
	lua_pushnumber(L, nodeB->score);	/*value_map, comp_func, valueA, valueB, scoreA, scoreB*/

	diff = (const char *)nodeA - (const char *)nodeB;
	if (diff == 0)
		idiff = 0;
	else
		idiff = diff > 0 ? 1 : -1;

	lua_pushnumber(L, idiff);	/*value_map, comp_func, valueA, valueB, scoreA, scoreB, idiff*/

	fret = lua_pcall(L, 5, 1, 0);
	if (fret != 0)
		return luaL_error(L, "comp error %s", lua_tostring(L, -1));
	
	ret = lua_tointeger(L, -1);
	
	lua_settop(L, top);
	return ret;
}

static int lua__new(lua_State *L)
{
	sl_t *sl;
	luaL_checktype(L, 1, LUA_TFUNCTION);
	sl = (sl_t *)lua_newuserdata(L, sizeof(sl_t));
	slInit(sl);
	sl->comp = comp;

	luaL_getmetatable(L, CLASS_SKIPLIST);
	lua_setmetatable(L, -2);

	lua_newtable(L);
		lua_pushvalue(L, 1);
		lua_setfield(L, -2, "comp_func");

		lua_newtable(L);
		lua_setfield(L, -2, "value_map"); /* k = node_ptr, value = lua_value*/

		lua_newtable(L);
		lua_setfield(L, -2, "node_map"); /* k = lua_value, value = node_ptr*/

	lua_setuservalue(L, -2);

	return 1;
}

static int lua__skiplist_gc(lua_State *L)
{
	sl_t *sl = CHECK_SL(L, 1);
	slDestroy(sl, NULL, NULL);
	return 0;
}

static int lua__insert(lua_State *L)
{
	double score;
	sl_t *sl;
	slNode_t *node;
	int level;
	
	sl = CHECK_SL(L, 1);
	node = luac__get_node(L, 1, 2);
	score = luaL_optnumber(L, 3, 0.0);
	if (node != NULL)
		return luaL_error(L, "value exists");

	level = slRandomLevel();
	if ((node = slCreateNode(level, NULL, score)) == NULL)
		return luaL_error(L, "no memory in lua__insert");

	node->udata = node;
	lua_getuservalue(L, 1);
		lua_getfield(L, -1, "node_map");
		lua_pushvalue(L, 2);
		lua_pushlightuserdata(L, (void *)node);
		lua_rawset(L, -3);
		lua_pop(L, 1);

		lua_getfield(L, -1, "value_map");
		lua_pushlightuserdata(L, (void *)node);
		lua_pushvalue(L, 2);
		lua_rawset(L, -3);
		lua_pop(L, 1);

	slInsertNode(sl, node, L);
	lua_pushlightuserdata(L, node);
	return 1;
}

static int lua__update(lua_State *L)
{
	sl_t *sl = CHECK_SL(L, 1);
	slNode_t *node = luac__get_node(L, 1, 2);
	double score = luaL_checknumber(L, 3);
	int level;
	if (node != NULL) {
		slNode_t *pNode = NULL;
		slDeleteNode(sl, node, L, &pNode);
		assert(pNode == node);
		node->score = score;
	} else {
		level = slRandomLevel();
		if ((node = slCreateNode(level, NULL, score)) == NULL) {
			return luaL_error(L, "no memory in lua__update");
		}
		node->udata = node;
		lua_getuservalue(L, 1);
			lua_getfield(L, -1, "node_map");
			lua_pushvalue(L, 2);
			lua_pushlightuserdata(L, (void *)node);
			lua_rawset(L, -3);
			lua_pop(L, 1);

			lua_getfield(L, -1, "value_map");
			lua_pushlightuserdata(L, (void *)node);
			lua_pushvalue(L, 2);
			lua_rawset(L, -3);
			lua_pop(L, 1);
	}
	slInsertNode(sl, node, L);
	lua_pushlightuserdata(L, node);
	return 1;
}

#if ENABLE_LSL_INDEX
static int lua__index(lua_State *L)
{
	slNode_t *node;
	sl_t *sl = CHECK_SL(L, 1);
	(void)sl;
	if (lua_type(L, 2) == LUA_TSTRING) {
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if (!lua_isnil(L, -1))
			return 1;
	}
	node = luac__get_node(L, 1, 2);
	if (node == NULL) {
		lua_pushnil(L);
		return 0;
	}
	lua_pushnumber(L, node->score);
	return 1;
}
#endif

static int lua__delete(lua_State *L)
{
	sl_t *sl = CHECK_SL(L, 1);
	slNode_t *node = luac__get_node(L, 1, 2);
	if (node == NULL)
		return 0;

	slDeleteNode(sl, node, L, NULL);

	lua_getuservalue(L, 1);

	lua_getfield(L, -1, "node_map");
	lua_pushvalue(L, 2);
	lua_pushnil(L);
	lua_rawset(L, -3);
	lua_pop(L, 1);

	lua_getfield(L, -1, "value_map");
	lua_pushlightuserdata(L, (void *)node);
	lua_pushnil(L);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	
	lua_pushboolean(L, 1);
	return 1;
}

static int lua__exists(lua_State *L)
{
	sl_t *sl = CHECK_SL(L, 1);
	slNode_t *node = luac__get_node(L, 1, 2);
	(void)sl;
	lua_pushboolean(L, node != NULL);
	return 1;
}

static int lua__size(lua_State *L)
{
	sl_t *sl = CHECK_SL(L, 1);
	lua_pushinteger(L, sl->size);
	return 1;
}

static int lua__iterator(lua_State *L)
{
	slNode_t *node;
	sl_t *sl = CHECK_SL(L, lua_upvalueindex(1));
	int last = (int)lua_tointeger(L, lua_upvalueindex(2));
	int rankMax = (int)lua_tointeger(L, lua_upvalueindex(3));
	if (last > rankMax)
		return 0;
	
	lua_pushinteger(L, last + 1);
	lua_replace(L, lua_upvalueindex(2));

	node = slGetNodeByRank(sl, last);
	if (node == NULL)
		return 0;
	lua_getuservalue(L, lua_upvalueindex(1));
	lua_getfield(L, -1, "value_map");
	lua_pushlightuserdata(L, (void *)node);
	lua_rawget(L, -2);
	lua_pushinteger(L, last);
	lua_insert(L, -2);
	lua_pushnumber(L, node->score);
	return 3;
}

static int lua__rank_pairs(lua_State *L)
{
	sl_t *sl = CHECK_SL(L, 1);
	int rankMin = luaL_optinteger(L, 2, 1);
	int rankMax = luaL_optinteger(L, 3, INT_MAX);

	rankMax = rankMax >= sl->size ? sl->size : rankMax;

	if (rankMin < 0 || rankMin > rankMax)
		return luaL_error(L, "range error!");

	lua_pushvalue(L, 1);
	lua_pushinteger(L, rankMin);
	lua_pushinteger(L, rankMax);
	lua_pushcclosure(L, lua__iterator, 3);
	return 1;
}

static int lua__get_by_rank(lua_State *L)
{
	slNode_t *node;
	sl_t *sl = CHECK_SL(L, 1);
	int rank = luaL_checkinteger(L, 2);
	if (rank <= 0 && rank > sl->size) {
		lua_pushnil(L);
		lua_pushliteral(L, "err index");
		return 2;
	}
	node = slGetNodeByRank(sl, rank);
	if (node == NULL)
		return 0;
	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "value_map");
	lua_pushlightuserdata(L, (void *)node);
	lua_rawget(L, -2);
	return 1;
}

static int lua__del_by_rank(lua_State *L)
{
	slNode_t *node;
	sl_t *sl = CHECK_SL(L, 1);
	int rank = luaL_checkinteger(L, 2);
	if (rank <= 0 && rank > sl->size) {
		lua_pushnil(L);
		lua_pushliteral(L, "err index");
		return 2;
	}
	node = slGetNodeByRank(sl, rank);
	if (node == NULL)
		return 0;
	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "value_map");
	lua_pushlightuserdata(L, (void *)node);
	lua_rawget(L, -2);

	slDeleteNode(sl, node, L, NULL);

	/*uservalue, value_map, value*/
	lua_getfield(L, -3, "node_map");
	/*uservalue, value_map, value, node_map*/
	lua_pushvalue(L, -2);
	lua_pushnil(L);
	lua_rawset(L, -3);
	lua_pop(L, 2);

	/*uservalue, value_map*/
	lua_pushlightuserdata(L, (void *)node);
	lua_pushnil(L);
	lua_rawset(L, -3);

	return 0;
}

static int lua__get_score(lua_State *L)
{
	sl_t *sl = CHECK_SL(L, 1);
	slNode_t *node = luac__get_node(L, 1, 2);
	(void)sl;
	if (node == NULL)
		return 0;
	lua_pushnumber(L, node->score);
	return 1;
}

static int lua__score_range(lua_State *L)
{
	int i;
	slNode_t *pMin, *pMax, *node;
	sl_t *sl = CHECK_SL(L, 1);
	double min = luaL_checknumber(L, 2);
	double max = luaL_optnumber(L, 3, DBL_MAX);
	
	luaL_argcheck(L, min < max, 3, "max should greater or equal than min");
	pMin = slFirstGEThan(sl, min);
	pMax = slLastLEThan(sl, max);
	if (pMin == NULL && pMax == NULL)
		return 0;
	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "value_map");
	lua_newtable(L);
	for (node = pMin, i = 1; node != SL_NEXT(pMax); node = SL_NEXT(node)) {
		lua_pushlightuserdata(L, (void *)node);
		lua_rawget(L, -3);
		lua_rawseti(L, -2, i++);
	}
	return 1;
}

static int lua__rank_of(lua_State *L)
{
	int rank;
	sl_t *sl = CHECK_SL(L, 1);
	slNode_t *node = luac__get_node(L, 1, 2);
	if (node == NULL)
		return 0;
	rank = slGetRank(sl, node, L);
	lua_pushinteger(L, rank);
	return 1;
}

static int lua__rank_range(lua_State *L)
{
	int n;
	sl_t *sl = CHECK_SL(L, 1);
	slNode_t *node;
	int rankMin = luaL_optinteger(L, 2, 1);
	int rankMax = luaL_optinteger(L, 3, INT_MAX);

	rankMax = rankMax >= sl->size ? sl->size : rankMax;

	if (rankMin < 0 || rankMin > rankMax)
		return luaL_error(L, "range error!");

	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "value_map");

	lua_newtable(L);
	SL_FOREACH_RANGE(sl, rankMin, rankMax, node, n) {
		lua_pushlightuserdata(L, (void *)node);
		lua_rawget(L, -3);
		lua_rawseti(L, -2, rankMin + n - 1);
	}
	return 1;
}

static int lua__next(lua_State *L)
{
	slNode_t *next = NULL;
	sl_t *sl = CHECK_SL(L, 1);
	slNode_t *node = luac__get_node(L, 1, 2);
	(void)sl;
	if (node == NULL)
		return 0;
	next = SL_NEXT(node);
	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "value_map");
	lua_pushlightuserdata(L, (void *)next);
	lua_rawget(L, -2);
	return 1;
}

static int lua__prev(lua_State *L)
{
	slNode_t *prev = NULL;
	sl_t *sl = CHECK_SL(L, 1);
	slNode_t *node = luac__get_node(L, 1, 2);
	(void)sl;
	if (node == NULL)
		return 0;
	prev = SL_PREV(node);
	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "value_map");
	lua_pushlightuserdata(L, (void *)prev);
	lua_rawget(L, -2);
	return 0;
}

void deleteCb(void *udata, void *ctx)
{
	lua_State *L = ctx;
	int top = lua_gettop(L);
	/* node->udata = node, see lua__insert */ 
	lua_pushlightuserdata(L, udata);
	lua_rawget(L, 5);
	lua_pushnil(L);
	lua_rawset(L, 6);
	lua_pushnil(L);
	/* node->udata = node, see lua__insert */ 
	lua_pushlightuserdata(L, udata);
	lua_rawset(L, 5);
	lua_settop(L, top);
}

static int lua__del_rank_range(lua_State *L)
{
	sl_t *sl = CHECK_SL(L, 1);
	int min = luaL_checkinteger(L, 2);
	int max = luaL_optinteger(L, 3, min);
	int n;
	luaL_argcheck(L, 1 <= min && min <= max && min <= sl->size, 2, "min [1, size]");
	luaL_argcheck(L, 1 <= max && min <= max && max <= sl->size, 3, "max [1, size]");
	lua_settop(L, 3);
	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "value_map");		/*idx = 5*/
	lua_getfield(L, -2, "node_map");		/*idx = 6*/
	n = slDeleteByRankRange(sl, min, max, deleteCb, L);
	lua_pushinteger(L, n);
	return 1;
}

static int opencls__skiplist(lua_State *L)
{
	luaL_Reg lmethods[] = {
		{"insert", lua__insert},
		{"update", lua__update},
		{"delete", lua__delete},
		{"exists", lua__exists},
		{"get_by_rank", lua__get_by_rank},
		{"del_by_rank", lua__del_by_rank},
		{"del_by_rank_range", lua__del_rank_range},
		{"rank_of", lua__rank_of},
		{"rank_range", lua__rank_range},
		{"get_score", lua__get_score},
		{"score_range", lua__score_range},
		{"next", lua__next},
		{"prev", lua__prev},
		{"size", lua__size},
		{"rank_pairs", lua__rank_pairs},
		{NULL, NULL},
	};
	luaL_newmetatable(L, CLASS_SKIPLIST);
	luaL_newlib(L, lmethods);
#if ENABLE_LSL_INDEX
	lua_pushcclosure(L, lua__index, 1);
#endif
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__update);
	lua_setfield(L, -2, "__newindex");
	lua_pushcfunction(L, lua__skiplist_gc);
	lua_setfield (L, -2, "__gc");
	lua_pushcfunction(L, lua__size);
	lua_setfield (L, -2, "__len");
	return 1;
}

int luaopen_lskiplist(lua_State* L)
{
	luaL_Reg lfuncs[] = {
		{"new", lua__new},
		{NULL, NULL},
	};
	opencls__skiplist(L);
	luaL_newlib(L, lfuncs);
	return 1;
}
