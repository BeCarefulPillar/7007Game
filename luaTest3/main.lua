local hotfix = require "hotfix"
local test =  require "test"
local test_hot = require "test_hot"

--[[print("before hotfix")
for i = 1, 5 do 
    test.print_index() -- 热更前，调用 print_index，打印 index 的值
end 
--]]
local i = 1
while true do
    local name, value = debug.getupvalue(test, i)
    print("update_func",name,value)
    if name == nil then -- 当所有上值收集完时，跳出循环
        break
    end
    i = i + 1
end

--[[hotfix.update(test.print_index, test_hot) -- 收集旧函数的上值，用于新函数的引用，这个对应之前说的归纳第2小点
test.print_index = test_hot -- 新函数替换旧的函数，对应之前说的归纳第1小点


print("after hotfix")

for i = 1, 5 do 
    test.print_index() -- 打印更新后的 index 值
end 
--]]
--test.show()