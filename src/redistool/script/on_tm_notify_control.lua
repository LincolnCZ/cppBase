-- input
local iKey =        KEYS[1]
local iName =       KEYS[2]
local iMyTmId =     KEYS[3]
local iTmTasksKey = KEYS[4]
local iHeartBeat =  KEYS[5]

-- return value
local oExisted = 0
local oPreempted = 0
local oOriginal

-- check if task exists
if redis.call("EXISTS", iKey) == 1 then
  oExisted = 1
  oOriginal = redis.call("HGETALL", iKey)

  local rTm =  redis.call("HGET", iKey, "tm")
  local rOpt = redis.call("HGET", iKey, "opt")
  local rVer = redis.call("HGET", iKey, "ver")

  if rTm ~= false and rTm ~= nil and rTm == "" and rOpt ~= false and rOpt ~= nil and tonumber(rOpt) == 1 then
    -- preempt it only when the task is running and not owned by any tm,
    -- and I'm idle
    local newVer = 1
    if rVer ~= false and rVer ~= nil then
      newVer = tonumber(rVer) + 1
    end

    redis.call("HMSET", iKey, "tm", iMyTmId, "ver", tonumber(newVer))
    redis.call("HSET", iTmTasksKey, iName, tonumber(iHeartBeat))
    oPreempted = 1 
  end
end

return { oExisted, oPreempted, oOriginal, redis.call("HGETALL", iKey) }
