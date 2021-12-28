local iTmSet = KEYS[1]
local iTmStatusPrefix = KEYS[2]

local ret = ""
local instances = {}
local results = redis.call("SMEMBERS", iTmSet)
for i, v in ipairs(results) do
    local task = redis.call("HGETALL", iTmStatusPrefix .. v)
    if task ~= false and task ~= nil
    then
        table.insert(instances, { v, task })
    end
end
return instances 
