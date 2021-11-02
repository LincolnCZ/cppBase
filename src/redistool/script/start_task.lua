-- input
local iOptVer =          KEYS[1]
local iKey =             KEYS[2]
local iName =            KEYS[3]
local iAppId =           KEYS[4]
local iSid =             KEYS[5]
local iType =            KEYS[6]
local iParam =           KEYS[7]
local iAllTaskKey =      KEYS[8]
local iIdle =            KEYS[9]
local iMyTm =            KEYS[10]
local iMyTmKey =         KEYS[11]
local iHeartBeat =       KEYS[12]
local iTransient =       KEYS[13]
local iTimeoutStop =     KEYS[14]
local iCmdType =         tonumber(KEYS[15])

-- return value
local oUpdated = 1
local oOriginal

-- check if task exists
if redis.call("EXISTS", iKey) == 1
then
  oOriginal = redis.call("HGETALL", iKey)

  -- check if update is needed
  local valArrs = redis.call("HMGET", iKey, "optVer", "opt", "preStop")
  local rOptVer = valArrs[1]
  local rOpt = valArrs[2]
  local rPreStop = valArrs[3]
  local canCreate = (iCmdType == 0 or (iCmdType == 1 and (tonumber(rOpt) == 0 or tonumber(rPreStop) ~= 0)))
  local canUpdate = (iCmdType == 0 or (iCmdType == 2 and tonumber(rOpt) ~= 0 and tonumber(rPreStop) == 0))
  if (false == rOptVer or nil == rOptVer or tonumber(rOptVer) < tonumber(iOptVer)) and (canCreate or canUpdate)
  then
    redis.call(
      "HMSET",      iKey,
      "optVer",     iOptVer,
      "opt",        1,
      "taskParam",  iParam,
      "taskType",   iType,
      "appId",      iAppId,
      "sid",        iSid,
      "transient",  iTransient,
      "heartbeat",  iHeartBeat,
      "timeout",    iTimeoutStop,
      "preStop",    0,
      "stopReason", 0
      )
  else
    -- version check fail, treat it as outdated
    oUpdated = 0
  end
else
  if iCmdType == 0 or iCmdType == 1
  then
      -- new task
      redis.call(
        "HMSET",        iKey,
        "optVer",       iOptVer,
        "opt",          1,
        "taskParam",    iParam,
        "taskType",     iType,
        "appId",        iAppId,
        "sid",          iSid,
        "tm",           "",
        "sap",          "",
        "ver",          "0",
        "taskId",       "",
        "transient",    iTransient,
        "heartbeat",    iHeartBeat,
        "timeout",      iTimeoutStop,
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
  redis.call("EXPIRE", iKey, 2592000)
  redis.call("SADD", iAllTaskKey, iName)

  local rTm = redis.call("HGET", iKey, "tm")
  if rTm == "" and tonumber (iIdle) == 1
  then
    redis.call("HMSET", iKey, "tm", iMyTm)
    redis.call("HSET", iMyTmKey, iName, iHeartBeat)
  end

  local rVer = redis.call("HGET", iKey, "ver")
  redis.call("HMSET", iKey, "ver", tonumber(rVer+1))
end

return {oUpdated, oOriginal, redis.call("HGETALL", iKey) }
