local function table_find(t, x)
    local equal = x
    if type(x) ~= "function" then
        equal = function(v) return v == x end
    end
    for _, v in pairs(t) do
        if equal(v) then
            return v
        end
    end
end

local M = {}

local DBEAN_PROP = {'_kvs', '_save_kvs', '_save_flag'}

local _mt = {
  __index = M,
  __newindex = function(_t, _k, _v)
    if not table_find(DBEAN_PROP, _k) then
      --if _t._kvs[_k] ~= _v then
        print(_v)
        _t._kvs[_k] = _v
        _t._save_kvs[_k] = _v
      --end
    else
      rawset(_t, _k, _v)
    end
  end
}

--
function M.new(opt)
  --opt.save_period = opt.save_period or 300

  o = {}
  o.testIndex = 10
  --o._opt = opt
  o._kvs = {}
  o._save_kvs = {}
  o._save_flag = {time=0, flag=BEAN_SAVE_UPDATE}

  setmetatable(o, _mt)
  return o
end

function M:upIndex()
  self.testIndex = self.testIndex + 100
end




return M
--