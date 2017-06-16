local lskiplist = require "lskiplist"

function dump(sl, prefix)
	for rank, v, score in sl:rank_pairs() do
		print(prefix, rank, v, score)
	end
end


local test = {}
function test.insert()
	local sl = lskiplist.new()
	local list = {2, 5, 1, 3, 9, 7, 6, 8, 4}
	for _, i in pairs(list) do
		sl:insert(i, i * 10)
	end
	return sl
end

function new()
	return test.insert()
end

function test.update(isfunc)
	local sl = new()
	local list = {
		{1, -1, "update"},
		{2, nil, "delete"},
		{0, -2, "insert"},
	}
	for _, vv in pairs(list) do
		local v, score, action = vv[1], vv[2], vv[3]
		if isfunc then
			print("update on function, action:", action, v, score, sl:update(v, score))
		else
			sl[v] = score
			print("update on __newindex, action:", action, v, score)
		end
	end
	dump(sl, "update" .. (isfunc and " on function" or "on __newindex"))
end

function test.delete()
	local sl = new()
	local list = {0, 5, 9, 1, 20}
	print("try to delete :", table.concat(list, ", "))
	for i, v in pairs(list) do
		local ret = sl:delete(v)
		print("delete", v, ret)
	end
	dump(sl, "delete")
	return sl
end

function test.exists()
	local sl = new()
	local list = {0, 5, 9, 1, 20}
	for _, v in pairs(list) do
		print("exists", v, sl:exists(v))
	end
end

function test.get_by_rank()
	local sl = new()
	local list = {
		1, 2, 5, 9, 0, 20,
	}
	for _, i in pairs(list) do
		print("get_by_rank", i, sl:get_by_rank(i))
	end
end

function test.del_by_rank()
	local sl = new()
	local list = {
		9, 1, 2, 5, 0, 20,
	}
	for _, i in pairs(list) do
		local v, score = sl:del_by_rank(i)
		print("del_by_rank", i, v, score)
	end
end

function test.del_by_rank_range()
	local list = {
		{2, 5},
		{5},
		{0},
		{12},
		{0, 3},
		{8, 12},
		{-1, 25},
		{5, 2},
	}
	for i, v in pairs(list) do
		local sl = new()
		local s, e = v[1], v[2]
		print("del_by_rank_range", pcall(sl.del_by_rank_range, sl, s, e))
	end
end

function test.rank_of()
	local sl = new()
	local list = {
		0, 20, 1, 2, 5, 9
	}
	for i, v in pairs(list) do
		local rank = sl:rank_of(v)
		print("rank_of", v, rank)
	end
	dump(sl, "rank_of !!!!")
end

function test.rank_range()
	local sl = new()
	local list = {
		{1, 5},
		{2, 3},
		{5},
		{9},
		{},
		{0, 2},
		{8, 20},
		{0, 20},
		{25, 20},
	}
	for _, v in pairs(list) do
		local s, e = v[1], v[2]
		local ok, l, err = pcall(sl.rank_range, sl, s, e)
		if ok then
			print("rank_range", s, e, "{" .. table.concat(l, ",").."}")
		else
			print("rank_range, error", s, e, l, err)
		end
	end
end

function test.get_score(isfunc)
	local sl = new()
	local list = {
		0, 20, 1, 2, 5, 9
	}
	for _, v in pairs(list) do
		if isfunc then
			local score = sl:get_score(v)
			print("get_score", isfunc and "on function" or "on index", v, score)
		else
			local score = sl[v]
			print("get_score", isfunc and "on function" or "on index", v, score)
		end
	end
end

function test.score_range()
	local sl = new()
	local list = {
		{10, 50},
		{20, 30},
		{50},
		{90},
		{},
		{0, 20},
		{80, 200},
		{0, 200},
		{250, 200},
	}
	for _, v in pairs(list) do
		local s, e = v[1], v[2]
		local ok, l, rankMin, rankMax = pcall(sl.score_range, sl, s, e)
		if ok then
			print("rank_range", s, e, rankMin, rankMax, "{" .. table.concat(l, ",").."}")
		else
			print("rank_range, error", s, e, l)
		end
	end
end

function test.next()
	local sl = new()
	local list = {
		0, 20, 1, 2, 5, 9
	}
	for i, v in pairs(list) do
		print("next", v, sl:next(v))
	end
end

function test.prev()
	local sl = new()
	local list = {
		0, 20, 1, 2, 5, 9
	}
	for i, v in pairs(list) do
		print("prev", v, sl:prev(v))
	end
end

function test.size()
	local sl = new()
	print("size", sl:size(), #sl)
end

function test.rank_pairs()
	local sl = new()
	local list = {
		{1, 5},
		{2, 3},
		{5},
		{9},
		{},
	}
	for _, v in pairs(list) do
		local s, e = v[1], v[2]
		print("-----------------------------------------------------")
		for rank, v, score in sl:rank_pairs(s, e) do
			print("rank_pairs", s, e, rank, v, score)
		end
	end
end

function main()
	local sl = test.insert()
	dump(sl, "insert")

	print("===============")
	test.delete()

	print("===============")
	test.exists()

	print("===============")
	test.update(true)

	print("===============")
	test.update(false)

	print("===============")
	test.get_by_rank()

	print("===============")
	test.del_by_rank()

	print("===============")
	test.del_by_rank_range()

	print("===============")
	test.rank_of()

	print("===============")
	test.rank_range()

	print("===============")
	test.get_score(true)

	print("===============")
	test.get_score(false)

	print("===============")
	test.score_range()

	print("===============")
	test.next()

	print("===============")
	test.prev()

	print("===============")
	test.size()

	print("===============")
	test.rank_pairs()
end

main()


