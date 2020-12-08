function table_max_index(t)
        local max_index = 0
        for k, v in pairs(t) do
                if max_index < k then
                        max_index = k
                end
        end
        return max_index
end

mytable = setmetatable({10}, {
__call = function(mytable, newtable)
                sum = 0
                for i = 1, table_max_index(mytable) do
                        sum = sum + mytable[i]
                end

                for i = 1, table_max_index(newtable) do
                        sum = sum + newtable[i]
                end
                return sum
        end
})

newtable = {10, 20, 30}
print(mytable(newtable))