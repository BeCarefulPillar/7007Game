关于tokeninfoKR API
token的值

token：数据, string  

result = {sns：登入供應者Code,string,
          sns_user_id: 登入供應者提供的ID,string
        }
把result数据转成json 然后进行base64加密 得到token


例子:
local result = {sns = "facebook",
                sns_user_id="1" }

local data =  json.encode(result)
token = base64(data)