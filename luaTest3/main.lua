local json =  require "json"
local uihelper =  require "uihelper"
local result = {sns = "facebook",
                sns_user_id=999000199,
                time = 1591693079}

print(uihelper.encodeBase64(json.encode(result))) 