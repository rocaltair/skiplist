lskiplist = require "lskiplist"

function comp(a, b, scoreA, scoreB, pdiff)
	return scoreA - scoreB
end

local map_len = 1e5
local map = {}
for i = 1, map_len do
	map[i] = math.random()
end

function benchmark(f, ...)
	local s = os.clock()
	f(...)
	return os.clock() - s
end

local sl = lskiplist.new(comp)

function insert()
	local len = #map
	for i=1, len do
		sl:insert(i, map[i])
	end
end

function rank_range(count, range)
	for j = 1, count do
		local s = math.random(1, map_len - range)
		local list = sl:rank_range(s, s + range)
	end
end

function rank_of(test_count)
	for i = 1, test_count do
		local s = math.random(1, map_len)
		sl:rank_of(s)
	end
end

function delete()
	local len = #map
	for i=1, len do
		sl[i] = nil
	end
end

print("insert", benchmark(insert))
print("rank_range", benchmark(rank_range, 1e5, 50))
print("rank_of", benchmark(rank_of, 1e5))
print("delete", benchmark(delete))

