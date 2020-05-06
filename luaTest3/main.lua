local uihelper =  require "uihelper"
print(14.3 % 2.2)

local aa = {}

local function call()
    for _,v in ipairs(aa) do
        v()
    end
    -- body
end

table.insert(aa, function ()
    print("1111")
end)

call()