local dbconf= require "conf.db_conf"
local dbutils = require "lua/db_utils"
local M = {}

local function getLogDb()
    local log_config = dbconf.db_log
    local db = dbutils.ConnectDB(log_config)
    return db
end

local function writeFile(content)
    local f = assert(io.open("logs/log.txt",'a+'))
    f:write(content)
    f:close()
end

function M.logP(message)
    local message = debug.traceback("[INFO]:" .. message .. "    " .. os.date("%Y-%m-%d %H:%M:%S"), 2)
    writeFile(message .. "\n")
    local db = getLogDb()
    if not db then
        return
    end
    local SInfo = debug.getinfo(2, "S")
    local src = SInfo.short_src
    local lInfo = debug.getinfo(2, "l")
    local line = lInfo.currentline
    local fileline = src .. ":" .. line

    local res, err, errcode, sqlstate = db:query(string.format("INSERT INTO rogatewaylog(level,fileline,message,time) VALUES('INFO',%s,%s,NOW())", ngx.quote_sql_str(fileline), ngx.quote_sql_str(message)))
    db:close()
end

function M.logE(message, isSystemError)
    local fileline = message
    local message = debug.traceback("[ERROR]:" .. message .. "    " .. os.date("%Y-%m-%d %H:%M:%S"), 2)
    writeFile(message .. "\n")
    local db = getLogDb()
    if not db then
        return
    end
    
    if not isSystemError then
        local SInfo = debug.getinfo(2, "S")
        local src = SInfo.short_src
        local lInfo = debug.getinfo(2, "l")
        local line = lInfo.currentline
        fileline = src .. ":" .. line
    end

    local res, err, errcode, sqlstate = db:query(string.format("INSERT INTO rogatewaylog(level,fileline,message,time) VALUES('ERROR',%s,%s,NOW())", ngx.quote_sql_str(fileline), ngx.quote_sql_str(message)))
    db:close()
end

function M.run_safe(fn)
    local ok,err = pcall(fn)
    if not ok then
        M.logE(err,true)
    end
end

return M