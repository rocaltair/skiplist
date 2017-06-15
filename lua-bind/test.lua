lskiplist = require "lskiplist"

local sl = lskiplist.new(function(a, b, scoreA, scoreB, pdiff)
	if scoreA ~= scoreB then
		return scoreA - scoreB
	end
	if a ~= b then
		return a - b
	end
	return pdiff
end)

for i = 1, 10 do
	local score = math.random(1, 1000)
	sl:insert(i, score)
end

local t = sl:rank_range()
for rank, v in pairs(t) do
	local score = sl:get_score(v)
	print("sorted", rank, v, sl:get_score(v))
end

local delV, delScore = sl:del_by_rank(5)
print("del_by_rank data:", delV, delScore)

local s, e = 2, 8
local list = sl:rank_range(s, e)
for i, v in pairs(list) do
	local score = sl:get_score(v)
	print("after delete", i + s - 1, v, score)
end

print(sl:rank_of(7))
sl:update(7, -1)
for i, v in pairs(sl:rank_range()) do
	print("rank", i, v, sl:get_score(v))
end
for rank, v, score in sl:rank_pairs(1, 5) do
	print("rank_pairs", rank, v, score)
end
sl:update(7, 128)
sl:update(99, 128)
sl[234] = 127
print("score of 7", sl[7])

sl[234] = 125
sl[3] = nil

print("size", #sl, sl:size())
local srange, rankMin = sl:score_range(20, 679)
for i, v in pairs(srange) do
	print("score range of", rankMin + i - 1, v, sl:get_score(v))
end

