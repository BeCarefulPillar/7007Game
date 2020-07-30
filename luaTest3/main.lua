local hotfix = require "hotfix"
local test =  require "test"
local test_hot = require "test_hot"
local log = require"log"
--[[print("before hotfix")
for i = 1, 5 do 
    test.print_index() -- 热更前，调用 print_index，打印 index 的值
end 
--]]
--[[local i = 1
while true do
    local name, value = debug.getupvalue(test, i)
    print("update_func",name,value)
    if name == nil then -- 当所有上值收集完时，跳出循环
        break
    end
    i = i + 1
end
--]]

--[[hotfix.update(test.print_index, test_hot) -- 收集旧函数的上值，用于新函数的引用，这个对应之前说的归纳第2小点
test.print_index = test_hot -- 新函数替换旧的函数，对应之前说的归纳第1小点


print("after hotfix")

for i = 1, 5 do 
    test.print_index() -- 打印更新后的 index 值
end 
--]]
--test.show()


co = coroutine.create(
    function (x,y)
        local temp=10;
        print("function",coroutine.status( co ));
        print("coroutine section",x,y,temp);
        x = x + 1
        y = y + 1
        local tempvar1=coroutine.yield(x,y);
        print("coroutine section2",x,y,temp);
        print(coroutine.status(co))
    end
)

print(coroutine.status(co));
print("main" ,coroutine.resume(co, 3, 2));
print(coroutine.status( co ));
print("main",coroutine.resume(co));

local fun = coroutine.wrap(function()
    coroutine.yield(1) 
    coroutine.yield(2)
    coroutine.yield(3)
    coroutine.yield(4)
    coroutine.yield(5)
end)

for i = 1, 5 do
    print(fun())
end
