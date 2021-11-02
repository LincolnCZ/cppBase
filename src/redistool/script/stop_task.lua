-- input
local iOptVer =          KEYS[1]
local iKey =             KEYS[2]
local iName =            KEYS[3]
local iType =            KEYS[4]
local iAllTaskKey =      KEYS[5]
local iMyTm =            KEYS[6]
local iMyTmKey =         KEYS[7]
local iInternal=         KEYS[8]
local iCmdType =         tonumber(KEYS[9])

-- return value
local oUpdated = 1
local oOriginal

-- check if task exists
if redis.call("EXISTS", iKey) == 1
then
  oOriginal = redis.call("HGETALL", iKey)

  -- check if it's an external stop, we will check optVer in this case
  if tonumber(iInternal) == 0
  then
    -- external stop

    -- check if update is needed
    local valArrs = redis.call("HMGET", iKey, "optVer", "opt")
    local rOptVer = valArrs[1]
    local rOpt = valArrs[2]
    local canStop = (iCmdType == 0 or (iCmdType == 3 and tonumber(rOpt) == 1))

    if (false == rOptVer or nil == rOptVer or tonumber(rOptVer) < tonumber(iOptVer)) and canStop
    then
      redis.call(
        "HMSET",      iKey,
        "optVer",     iOptVer,
        "opt",        0,
        "taskParam",  "",
        "taskType",   iType,
        "appId",      0,
        "sid",        0,
        "preStop",    0,
        "stopReason", 0
        )
    else
      -- version check fail, treat it as outdated
      oUpdated = 0
    end
  else
    -- internal stop, don't override optVer in redis
    redis.call(
      "HMSET",      iKey,
      "opt",        0,
      "taskParam",  "",
      "taskType",   iType,
      "appId",      0,
      "sid",        0,
      "preStop",    0,
      "stopReason", 0
      )
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
      oOriginal = redis.call("HGETALL", iKey)
  else
      oUpdated = 0
  end
end

if oUpdated == 1
then
  redis.call("EXPIRE", iKey, 60)
  redis.call("SREM", iAllTaskKey, iName)

  local rTm = redis.call("HGET", iKey, "tm")
  if rTm == iMyTm
  then
    redis.call("HMSET", iKey, "tm", "", "sap", "", "taskId", "")
    redis.call("HDEL", iMyTmKey, iName)
  end

  local rVer = redis.call("HGET", iKey, "ver")
  redis.call("HMSET", iKey, "ver", tonumber(rVer+1))
end

return {oUpdated, oOriginal, redis.call("HGETALL", iKey) }
