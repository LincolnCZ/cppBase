-- input
local iKey = KEYS[1]
local iName = KEYS[2]
local iMyTmId = KEYS[3]
local iTmTasksKey = KEYS[4]

-- return value
local oExisted = 0
local oOriginal

-- check if task exists
if redis.call("EXISTS", iKey) == 1 then
    oExisted = 1
    oOriginal = redis.call("HGETALL", iKey)

    local rOpt = redis.call("HGET", iKey, "opt")
    if rOpt ~= false and rOpt ~= nil and tonumber(rOpt) == 0 then
        -- stopped task

        redis.call("HMSET", iKey, "tm", "", "sap", "", "taskId", "")
        redis.call("HDEL", iTmTasksKey, iName)

        local rVer = redis.call("HGET", iKey, "ver")
        if rVer ~= false and rVer ~= nil then
            redis.call("HMSET", iKey, "ver", tonumber(rVer) + 1)
        end
    end
end

return { oExisted, oOriginal, redis.call("HGETALL", iKey) }
