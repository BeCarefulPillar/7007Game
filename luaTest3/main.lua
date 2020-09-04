local hotfix = require "hotfix"
local test =  require "test"
local test_hot = require "test_hot"
local log = require"log"
local Utils = require"Utils"


local stream = require"stream"

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

--[[
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
--]]

--[[local function getInt(str)
    local bytes = {0,0,0,0}
    local len = string.len(str)
    for i=1,len do
        local index = len - i + 1
        bytes[index] = string.byte(string.sub(str,i,i))
    end
    return Utils.bufToInt32(bytes[4],bytes[3],bytes[2],bytes[1])
    -- body
end

local t={}
local function getByte(s, size)
    for i=1,string.len(s) do
        table.insert(t,string.byte(string.sub(s,i,i)))
    end
end

local str = Utils.int32ToBufStr(1)
getByte("23123", 80)

for i=1,#t do print(t[i]) end--]]

--[[
local proto = {}
proto[15] = {
     {name = "name", pType = "str"},
     {name = "age", pType = "int"},
     {name = "isHeath", pType ="bool"},
}

local function read(data)

end

local function testProto(msgId, data)
    local p = proto[msgId]
    if not p then
        assert(false,"not find proto")
    end

    for i = 1,#p do
        local pType = p[i].pType
        local pName = p[i].name
        if pType == "str" then

        elseif pType == "int" then

        elseif pType == "bool" then

        end
    end
    
end

local str = "1234567"
local len = 2
local pos = 1
local n = string.sub(str, pos + 1, pos + len)
print(n)
--]]


local s = stream.new()
s:WriteHeader(2)

s:WriteInt8(11)
s:WriteInt16(555)
s:WriteInt32(12344)    
s:WriteStr("1111")
s:WriteStr("sunkang")
s:WriteStr("ll")
s:Finsh()

local r = stream.new({_pBuff =tostring(s:Data()), _nSize = s._nSize})
print(r._pBuff)
print(r:ReadInt16())
print(r:ReadInt16())
print(r:ReadInt8())
print(r:ReadInt16())
print(r:ReadInt32())
print(r:ReadStr())
print(r:ReadStr())
print(r:ReadStr())
