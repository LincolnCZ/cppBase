-- taskmanager:taskset
local iTaskSetKey = KEYS[1]
local iTaskPrefix = KEYS[2]
local iAppid = KEYS[3]
local iType = KEYS[4]

local collect = {}
local results = redis.call("SMEMBERS", iTaskSetKey)
for _, taskName in ipairs(results) do
    -- check if task exists
    local lTaskKey = iTaskPrefix .. taskName
    local rAppid = "invalid"
    local rTaskType = "invalid"
    local params = redis.call("HMGET", lTaskKey, "appId", "taskType")

    local i = 0
    for _, param in ipairs(params) do
        i = i + 1
        if param ~= nil and param ~= false
        then
            if i == 1
            then
                rAppid = param
            end

            if i == 2
            then
                rTaskType = param
            end
        end
    end

    if rAppid == iAppid and rTaskType == iType
    then
        table.insert(collect, taskName)
    end
end
return collect
