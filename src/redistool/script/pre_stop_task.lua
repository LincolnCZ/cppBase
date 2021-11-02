-- input
local iOptVer =          KEYS[1]
local iKey =             KEYS[2]
local iInternal =        KEYS[3]
local iPreStop =         KEYS[4]
local iStopReason =      KEYS[5]
local iType =            KEYS[6]
local iCmdType =         tonumber(KEYS[7])

-- return value
local oUpdated = 0
local oOriginal

-- check if task exists
if redis.call("EXISTS", iKey) == 1
then
  oOriginal = redis.call("HGETALL", iKey)

  local nowPreStop = iPreStop
  local valArrs = redis.call("HMGET", iKey, "optVer", "opt", "preStop")
  local rOptVer = valArrs[1]
  local rOpt = valArrs[2]
  local rPreStop = valArrs[3]
  if rPreStop ~= false and rPreStop ~= nil and tonumber(rPreStop) ~= 0
  then
    nowPreStop = rPreStop
  end

  -- check if it's an external stop, we will check optVer in this case
  if tonumber(iInternal) == 0
  then
    local canStop = (iCmdType == 0 or (iCmdType == 3 and tonumber(rOpt) == 1 and tonumber(rPreStop) == 0))

    -- check if update is needed
    if (false == rOptVer or nil == rOptVer or tonumber(rOptVer) < tonumber(iOptVer)) and canStop
    then
      redis.call(
      "HMSET",      iKey,
      "optVer",     iOptVer,
      "preStop",    nowPreStop,
      "stopReason", iStopReason
      )

      oUpdated = 1
    end
  else
    -- internal stop, don't override optVer in redis
    redis.call(
    "HMSET",      iKey,
    "preStop",    nowPreStop,
    "stopReason", iStopReason
    )

    oUpdated = 1
  end
else
  if iCmdType == 0
  then
      -- new task
      redis.call(
        "HMSET",        iKey,
        "optVer",       iOptVer,
        "opt",          0,
        "taskParam",    "",
        "taskType",     iType,
        "appId",        0,
        "sid",          0,
        "tm",           "",
        "sap",          "",
        "ver",          "0",
        "taskId",       "",
        "preStop",      0,
        "stopReason",   0
        )
      redis.call("EXPIRE", iKey, 60)
      oOriginal = redis.call("HGETALL", iKey)
      oUpdated = 1
  end
end

if oUpdated == 1
then
  local rVer = redis.call("HGET", iKey, "ver")
  redis.call("HMSET", iKey, "ver", tonumber(rVer+1))
end

return {oUpdated, oOriginal, redis.call("HGETALL", iKey) }
