local uihelper =  require "uihelper"


local test  = {}
test.is = true
local aa = test and test.is or false
test = {cc = 1, is = false}

test.is = aa
print(aa,test.is)