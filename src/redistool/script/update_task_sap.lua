-- input
local iKey = KEYS[1]
local iMyTm = KEYS[2]
local iTm = KEYS[3]
local iSap = KEYS[4]
local iTaskId = KEYS[5]


-- return value
local oExisted = 0
local oUpdated = 0
local oOriginal

-- check if task exists
if redis.call("EXISTS", iKey) ~= 1
then
    return { oExisted, oUpdated }
end
oExisted = 1

oOriginal = redis.call("HGETALL", iKey)

local rTm = redis.call("HGET", iKey, "tm")
if rTm == iMyTm
then
    local rVer = redis.call("HGET", iKey, "ver")
    redis.call("HMSET", iKey,
            "ver", tonumber(rVer + 1),
            "tm", iTm,
            "sap", iSap,
            "taskId", iTaskId
    )
    oUpdated = 1
end
return { oExisted, oUpdated, oOriginal, redis.call("HGETALL", iKey) }
