local larr = require "liblarr"

local num_elements = 1 << 20
local array = larr.Vec.with_capacity('integer', num_elements)

print('pushing ' .. num_elements .. ' elements to larr.Vec')
local arr_start = os.clock()
for i = 1, num_elements do
	array:push(i)
end
local arr_str = tostring(array)
local arr_end = os.clock()

print(arr_end - arr_start, 'seconds elapsed')

local tbl = {}

print('pushing ' .. num_elements .. ' elements to table')
local tbl_start = os.clock()
for i = 1, num_elements do
	tbl[i] = i
end

local tbl_str = {'['}

local first = true
for i, v in ipairs(tbl) do
	if not first then
        tbl_str[#tbl_str + 1] = ', '
	else
		first = false
	end

    tbl_str[#tbl_str + 1] = tostring(v)
end

tbl_str[#tbl_str + 1] = ']\n'

tbl_str = table.concat(tbl_str)
local tbl_end = os.clock()

print(tbl_end - tbl_start, 'seconds elapsed')
