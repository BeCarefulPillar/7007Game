local utils = require("utils")

local M = {}

local TRACER_PROP = {'_parent', '_mykey', '_save_kvs'}

--
function M.new(raw, pnode)
  local o = {}

  if not pnode then
    o._save_kvs = {}
    o._mykey = ''
  else
    o._parent = pnode
  end

  local _mt = {
    __index = function(_, _k)
      if not utils.table_find(TRACER_PROP, _k) then
        return raw[_k]
      end
    end,
    __newindex = function(_t, _k, _v)
      if utils.table_find(TRACER_PROP, _k) then
        rawset(_t, _k, _v)
        return
      end

      if type(_v) == "table" then
        _v = M.new(_v, _t)
        _v._parent = _t
        _v._mykey  = _k
      end
      raw[_k] = _v

      local topkey = _k
      local parent = _t
      while parent do
        if parent._parent then
          topkey = parent._mykey
          parent = parent._parent
        else
          if parent._save_kvs then
            parent._save_kvs[topkey] = parent[topkey]
          end
          break
        end
      end
    end,
    __pairs = function ()
      return function (_, _k)
        local nextkey, nextvalue = next(raw, _k)
        return nextkey, nextvalue
      end
    end,
    __len = function ()
      return #raw
    end
  }
  setmetatable(o, _mt)

  for k, v in pairs(raw) do
    if type(v) == "table" then
      local _v = M.new(v, o)
      _v._mykey = k
      rawset(raw, k, _v)
      assert(_v._parent==o,  _v._mykey..' parent fail !!')
    end
  end

  return o
end

--
function M.reset_changes(m, now)
  m._save_kvs = {}
end

--
function M.get_changes(m)
  return m._save_kvs
end

return M