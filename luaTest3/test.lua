local M = {}
function M.test()
    for j = 1, 10, 3 do
        print(j)
    end
end 

local function appendFile(fileName,content)
    local f = assert(io.open(fileName,'a'))
    f:write(content)
    f:close()
end


local function aa()
    math.randomseed(tonumber(tostring(os.time()):reverse():sub(1,7))) --设置时间种子

    for index = 1 , 100000 do
        local allTeam = {}
        for i = 5, 32 do
            table.insert(allTeam, i)
        end
        
        local group = {}

        for i=1, 28 do
            local numPos = math.random(1, 28 - i + 1)
            group[i] = allTeam[numPos]
            table.remove(allTeam, numPos)
        end 


        math.random(1,8)
        local fourNum = {1,2,3,4}
        for i = 1, 4 do
            local numPos = math.random(1, 4 - i + 1)
            local pos = (i - 1) * 8 + math.random(1,8)
            table.insert(group, pos, fourNum[numPos])
            table.remove(fourNum, numPos)
        end
        local str = index - 1
        for i = 1, #group do
            str = str .. "," .. group[i]
        end
        appendFile('test.txt',str .. '\n')
    end
end

return M

