local larr = require "liblarr"

local num_elements = 1 << 20
local array = larr.Vec.with_capacity('integer', num_elements)

local to_push = {}
for i = 1, num_elements do
	to_push[i] = i
end

local iter, invariant, init = ipairs(to_push)

print('pushing ' .. num_elements .. ' elements to larr.Vec')
local arr_start = os.clock()
array:append(iter, invariant, init)
local arr_end = os.clock()

print(arr_end - arr_start, 'seconds elapsed')

local tbl = {}

print('pushing ' .. num_elements .. ' elements to table')
local tbl_start = os.clock()
for i = 1, num_elements do
	tbl[i] = i
end
local tbl_end = os.clock()

print(tbl_end - tbl_start, 'seconds elapsed')
