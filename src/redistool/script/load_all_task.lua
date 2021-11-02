local iTmId = KEYS[1]
local iTmTaskKeyPrefix = KEYS[2]
local iTaskPrefix = KEYS[3]

local lMyTmTaskKey = iTmTaskKeyPrefix .. iTmId

local ret = ""
local collect = {}
local results = redis.call("HGETALL", lMyTmTaskKey)
local i = 0
for _, taskName in ipairs(results) do
    if i % 2 == 0
    then
        local lTaskKey = iTaskPrefix .. taskName

        -- check if task exists
        if redis.call("EXISTS", lTaskKey) == 1
        then
            -- task exists, check task tm
            local rTm = redis.call("HGET", lTaskKey, "tm")
            if rTm ~= false and rTm ~= nil
            then
                if rTm == iTmId
                then
                    -- my task
                    local rOpt = redis.call("HGET", lTaskKey, "opt")
                    if rOpt ~= false and rOpt ~= nil and tonumber(rOpt) == 0
                    then
                        -- stopped task, remove from my tm tasks
                        redis.call("HDEL", lMyTmTaskKey, taskName)
                    end

                    -- update task ver
                    local rVer = redis.call("HGET", lTaskKey, "ver")
                    if rVer ~= false and rVer ~= nil
                    then
                        redis.call("HMSET", lTaskKey, "ver", tonumber(rVer + 1))
                    end
                else
                    -- task belongs to other tm, remove it from my tm tasks
                    redis.call("HDEL", lMyTmTaskKey, taskName)
                end
            end
        else
            -- task doesn't exist, remove it from my tm task list
            redis.call("HDEL", lMyTmTaskKey, taskName)
        end

        local task = redis.call("HGETALL", lTaskKey)
        if task ~= false and task ~= nil
        then
            table.insert(collect, { taskName, task })
        end
    end
    i = i + 1
end
return collect
