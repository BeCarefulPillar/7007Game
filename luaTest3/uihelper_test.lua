local json =  require "json"

local M = {}
function M.test()
    for j = 1, 10, 3 do
        print(j)
    end
end 

function M.Printtb(t)
    local print_r_cache = {}
    local output = {}
    local function sub_print_r(t, indent)
        if (print_r_cache[tostring(t)]) then
            table.insert(output, (indent.."*"..tostring(t)))
        else
            print_r_cache[tostring(t)] = true
            if (type(t) == "table") then
                for pos,val in pairs(t) do
                    if (type(val) == "table") then
                        table.insert(output, indent.."["..pos.."] => "..tostring(t).." {")
                        sub_print_r(val, indent..string.rep(" ",string.len(pos)+8))
                        table.insert(output, indent..string.rep(" ",string.len(pos)+6).."}")
                    elseif (type(val) == "string") then
                        table.insert(output, (indent.."["..pos..'] => "'..val..'"'))

                    else
                        table.insert(output, (indent.."["..pos.."] => "..tostring(val)))
                    end
                end
            else
                table.insert(output, (indent..tostring(t)))
            end
        end
    end

    if (type(t) == "table") then
        table.insert(output, (tostring(t).." {"))
        sub_print_r(t,"  ")
        table.insert(output, ("}"))
    else
        sub_print_r(t,"  ")
    end
    print(table.concat(output, "\n"))
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




function M.encodeBase64(source_str)
    local b64chars = 'lnbYdSrXGjQhKRvaktJBm52Vz1pZegLOf0TiuqNxI4wPACFD3WEUMs79c8o6yH+/'
    local s64 = ''
    local str = source_str

    while #str > 0 do
        local bytes_num = 0
        local buf = 0

        for byte_cnt=1,3 do
            buf = (buf * 256)
            if #str > 0 then
                buf = buf + string.byte(str, 1, 1)
                str = string.sub(str, 2)
                bytes_num = bytes_num + 1
            end
        end

        for group_cnt=1,(bytes_num+1) do
            local b64char = math.fmod(math.floor(buf/262144), 64) + 1
            s64 = s64 .. string.sub(b64chars, b64char, b64char)
            buf = buf * 64
        end

        for fill_cnt=1,(3-bytes_num) do
            s64 = s64 .. '='
        end
    end

    return s64
end

function M.decodeBase64(str64)
    local b64chars = 'lnbYdSrXGjQhKRvaktJBm52Vz1pZegLOf0TiuqNxI4wPACFD3WEUMs79c8o6yH+/'
    local temp={}
    for i=1,64 do
        temp[string.sub(b64chars,i,i)] = i
    end
    temp['=']=0
    local str=""
    for i=1,#str64,4 do
        if i>#str64 then
            break
        end
        local data = 0
        local str_count=0
        for j=0,3 do
            local str1=string.sub(str64,i+j,i+j)
            if not temp[str1] then
                return
            end
            if temp[str1] < 1 then
                data = data * 64
            else
                data = data * 64 + temp[str1]-1
                str_count = str_count + 1
            end
        end
        for j=16,0,-8 do
            if str_count > 0 then
                str=str .. string.char(math.floor(data/math.pow(2,j)))
                data=math.fmod(data, math.pow(2,j))
                str_count = str_count - 1
            end
        end
    end
 
    local last = tonumber(string.byte(str, string.len(str), string.len(str)))
    if last == 0 then
        str = string.sub(str, 1, string.len(str) - 1)
    end
    return str
end


function M.decodeBase64TokenKR(str64)
    local b64chars = 'lnbYdSrXGjQhKRvakt+JBm52Vz1pZeg/LOf0TiuqNxI4wPACFD3WEUMs79c8o6yH'
    local temp={}
    for i=1,64 do
        temp[string.sub(b64chars,i,i)] = i
    end
    temp['=']=0
    local str=""
    for i=1,#str64,4 do
        if i>#str64 then
            break
        end
        local data = 0
        local str_count=0
        for j=0,3 do
            local str1=string.sub(str64,i+j,i+j)
            if not temp[str1] then
                return
            end
            if temp[str1] < 1 then
                data = data * 64
            else
                data = data * 64 + temp[str1]-1
                str_count = str_count + 1
            end
        end
        for j=16,0,-8 do
            if str_count > 0 then
                str=str .. string.char(math.floor(data/math.pow(2,j)))
                data=math.fmod(data, math.pow(2,j))
                str_count = str_count - 1
            end
        end
    end
 
    local last = tonumber(string.byte(str, string.len(str), string.len(str)))
    if last == 0 then
        str = string.sub(str, 1, string.len(str) - 1)
    end
    return str
end


local function bb()
    local result = {sns = "facebook",
                    sns_user_id="1" }

    print(json.encode(result))
end

return M

