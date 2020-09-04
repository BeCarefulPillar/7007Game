local  M = {}

function M.getBytes(str)
    local bytes = {}
    local len = #str
    for i = 1, len do
        bytes[i] = string.byte(string.sub(str,i,i))
    end
    return bytes
end

-- 字符串转int
function M.strToInt32(str)
    local bytes = M.getBytes(str)
    local num1 = bytes[1]
    local num2 = bytes[2]
    local num3 = bytes[3]
    local num4 = bytes[4]
    return M.bufToInt32(num1, num2, num3, num4)
end

-- 二进制转int
function M.bufToInt32(num1, num2, num3, num4)
    local num = 0;
    num = num + num1
    num = num + M.leftShift(num2, 8);
    num = num + M.leftShift(num3, 16);
    num = num + M.leftShift(num4, 24);
    return num;
end
 
-- int转二进制
function M.int32ToBufStr(num)
    local str = "";
    str = str .. M.numToAscii(num);
    str = str .. M.numToAscii(M.rightShift(num, 8));
    str = str .. M.numToAscii(M.rightShift(num, 16));
    str = str .. M.numToAscii(M.rightShift(num, 24));
    return str;
end
 
-- str转shot
function M.strToInt16(str)
    local bytes = M.getBytes(str)
    local num1 = bytes[1]
    local num2 = bytes[2]

    return M.bufToInt16(num1, num2)
end

-- 二进制转shot
function M.bufToInt16(num1, num2)
    local num = 0;
    num = num + num1
    num = num + M.leftShift(num2, 8);
    return num;
end
 
-- shot转二进制
function M.int16ToBufStr(num)
    local str = "";
    str = str .. M.numToAscii(num);
    str = str .. M.numToAscii(M.rightShift(num, 8));
    return str;
end
 
 
-- str转char
function M.strToInt8(str)
    local bytes = M.getBytes(str)
    local num1 = bytes[1]
    return M.bufToInt8(num1)
end

-- 二进制转char
function M.bufToInt8(num1)
    local num = 0;
    num = num + num1;
    return num;
end

-- char转二进制
function M.int8ToBufStr(num)
    local str = "";
    str = str .. M.numToAscii(num);
    return str;
end

-- 左移
function M.leftShift(num, shift)
    return math.floor(num * (2 ^ shift));
end
 
-- 右移
function M.rightShift(num, shift)
    return math.floor(num / (2 ^ shift));
end
 
-- 转成Ascii
function M.numToAscii(num)
    num = num % 256;
    return string.char(num);
end

return M