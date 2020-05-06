local M = {}

local function append_table(t,x)
    for k,v in pairs(x) do
        assert(not t[k], "uihelper already has name: " .. k)
        t[k] = v
    end
end

M.RankRward_Rarity = {
    Green = "Green",
    Blue = "Blue",
    Purple = "Purple",
    Orange = "Orange",
}

append_table(M, require("uihelper_test"))
return M