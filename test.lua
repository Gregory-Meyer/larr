local larr = require "liblarr"

local num_elements = 1 << 12
local array = larr.Array.with_capacity('integer', num_elements)

print('pushing ' .. num_elements .. ' elements to larr.Array')
local arr_start = os.clock()
for i = 1, num_elements do
	array:push(i)
end
print(tostring(array))
local arr_end = os.clock()

print(arr_end - arr_start .. 'seconds elapsed')

local table = {}

print('pushing ' .. num_elements .. ' elements to table')
local tbl_start = os.clock()
for i = 1, num_elements do
	table[i] = i
end

io.write('[')

local first = true
for i, v in ipairs(table) do
	if not first then
		io.write(', ')
	else
		first = false
	end

	io.write(v)
end

io.write(']\n')

local tbl_end = os.clock()

print(tbl_end - tbl_start .. 'seconds elapsed')
