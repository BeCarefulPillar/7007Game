local Utils = require "Utils"

local M = {}

local function toClass(t)
    _pBuff = ""
    _nSize = 1024
    _nWritePos = 0
    _nReadPos = 0

    t = t or {}
    return M.makeObject({
        _pBuff = t._pBuff or _pBuff,
        _nSize = t._nSize or _nSize,
        _nWritePos = t._nWritePos or _nWritePos,
        _nReadPos = t._nReadPos or _nReadPos,
    })
end

function M:CanRead(n)
    return self._nSize - self._nReadPos >= n
end

function M:CanWrite(n)
    return self._nSize - self._nWritePos >= n
end

function M:Push(n)
    self._nReadPos = self._nReadPos + n
end

function M:Pop(n)
    self._nWritePos = self._nWritePos + n
end

function M:Data()
    return self._pBuff
end

function M:Length()
    return self._nWritePos
end

function M:SetWritePos(n)
    self._nWritePos = n;
end


function M:Read(nLen, bOffset)
    if bOffset == nil then
        bOffset = true
    end
    local data
    if self:CanRead(nLen) then
        data = string.sub(self._pBuff, self._nReadPos + 1, self._nReadPos + nLen)
        if bOffset then
            self:Push(nLen)
        end

    end
    return data
end
--[[
function M:ReadArray(type, len) {
    local int32Len = 32
    local dataStr = Read(int32Len, false)
    local nArrLen = Utils.strToInt32(dataStr)
    --判断能不能放的下
    if nArrLen < len then
        --计算数组的实际字节长度
        local nLen = nArrLen * sizeof(T);
        if (CanRead(nLen + sizeof(uint32_t))) {
            --计算已读位置+数组长度所占有空间
            Push(int32Len);
            memcpy(pArray, _pBuff + _nReadPos, nLen);
            Push(nLen);
            return nArrLen;
        }
    end
    return 0;
end--]]

function M:ReadStr()
    local int32Len = 4
    local dataStr = self:Read(int32Len, false)
    local nArrLen = Utils.strToInt32(dataStr)
    local nLen = nArrLen * 1;
    if self:CanRead(nLen + int32Len) then
        --计算已读位置+数组长度所占有空间
        self:Push(int32Len);
        local dataStr = self:Read(nLen)
        return dataStr;
    end
    return "";
end

function M:ReadInt8()
    local dataStr = self:Read(1)
    local data = Utils.strToInt8(dataStr)
    return data
end

function M:ReadInt16()
    local dataStr = self:Read(2)
    local data = Utils.strToInt16(dataStr)
    return data
end

function M:ReadInt32()
    local dataStr = self:Read(4)
    local data = Utils.strToInt32(dataStr)
    return data
end

function M:Write(nLen, n)
    if self:CanWrite(nLen) then
        local startStr = string.sub(self._pBuff, 1, self._nWritePos)
        local endStr = string.sub(self._pBuff, self._nWritePos + nLen + 1, -1)
        self._pBuff = startStr .. n .. endStr
        self:Pop(nLen);
        return true;
    end
    return false;
end

function M:WriteInt8(n)
    local str = Utils.int8ToBufStr(n)
    return self:Write(1, str)
end

function M:WriteInt16(n)
    local str = Utils.int16ToBufStr(n)
    return self:Write(2, str)
end

function M:WriteInt32(n)
    local str = Utils.int32ToBufStr(n)
    return self:Write(4, str)
end

function M:WriteStr(n)
    local nLen = #n * 1
    if self:CanWrite(nLen + 32) then
        --写入数组长度
        self:WriteInt32(#n);
        self._pBuff = self._pBuff .. n
        self:Pop(nLen);
        return true;
    end
    return false
end

function M:WriteHeader(cmd)
    self:WriteInt16(cmd)
    self:WriteInt16(0)
end

function M:Finsh()
    local pos = self:Length()
    self:SetWritePos(2)
    self:WriteInt16(pos)
    self:SetWritePos(pos)
end

function M.new(t)
    return toClass(t)
end

function M.makeObject(t)
    return setmetatable(t, {__index=M})
end

return M


