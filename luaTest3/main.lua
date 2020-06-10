local json =  require "json"
local dkjson =  require "dkjson"
local uihelper =  require "uihelper"
local result = {sns = "facebook", sns_user_id = 999000199, time = 1591697507}

print(uihelper.encodeBase64(json.encode(result)))
print(uihelper.encodeBase64(dkjson.encode(result))) 