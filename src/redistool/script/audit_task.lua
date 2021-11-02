-- input
local iKey = KEYS[1]
local iTaskName = KEYS[2]
local iTmId = KEYS[3]
local iTmTaskList = KEYS[4]
local iHeartBeat = KEYS[5]

-- return value
local oExisted = 0
local oRemoveFromTmList = 0
local oStopped = 0
local oUpdated = 0
local oOriginal

-- check if task exists
if redis.call("EXISTS", iKey) == 1
then
    -- task exists
    oExisted = 1
    oOriginal = redis.call("HGETALL", iKey)

    -- check tm
    local rTm = redis.call("HGET", iKey, "tm")
    if rTm ~= false and rTm ~= nil
    then
        if rTm == iTmId
        then
            -- my task
            local rOpt = redis.call("HGET", iKey, "opt")
            if rOpt ~= false and rOpt ~= nil and tonumber(rOpt) == 0
            then
                -- stopped task
                oStopped = 1

                -- update task ver
                local rVer = redis.call("HGET", iKey, "ver")
                if rVer ~= false and rVer ~= nil
                then
                    redis.call("HSET", iKey, "ver", tonumber(rVer + 1))
                end

                -- wipe out tm, sap, taskId
                redis.call("HMSET", iKey,
                        "tm", "",
                        "sap", "",
                        "taskId", ""
                )

                -- remove from my tm tasks
                redis.call("HDEL", iTmTaskList, iTaskName)
                oRemoveFromTmList = 1
            else
                -- task is still running, update heart beat
                redis.call("HSET", iTmTaskList, iTaskName, iHeartBeat)
                oUpdated = 1
            end
        else
            -- task belongs to other tm, remove it from my tm tasks
            redis.call("HDEL", iTmTaskList, iTaskName)
            oRemoveFromTmList = 1
        end
    end
else
    -- does not exists, remove from tm task list
    redis.call("HDEL", iTmTaskList, iTaskName)
    oRemoveFromTmList = 1
end

return { oExisted, oRemoveFromTmList, oStopped, oUpdated, oOriginal, redis.call("HGETALL", iKey) }
