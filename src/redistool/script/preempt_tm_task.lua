local iMyTmId           = KEYS[1]
local iOldTmId          = KEYS[2]
local iTmTaskKeyPrefix  = KEYS[3]
local iTaskPrefix       = KEYS[4]
local iHeartBeat        = tonumber(KEYS[5])
local iDenominator      = tonumber(KEYS[6])


local lMyTmTaskKey  = iTmTaskKeyPrefix..iMyTmId
local lOldTmTaskKey = iTmTaskKeyPrefix..iOldTmId

local i = 0
local total = 0

local results = redis.call("HGETALL", lOldTmTaskKey)
for _, _ in ipairs(results) do
  -- count task number
  if i % 2 == 0 then
    total = total + 1
  end
  i = i + 1
end

if iDenominator < 0 or iDenominator == 0 then
  iDenominator = 10
end

local target = total / iDenominator
if target == 0 then
  target = total
end

local count = 0
local collect = {}
i = 0
for _, taskName in ipairs(results) do
  if i % 2 == 0 and count < target then
    local lTaskKey = iTaskPrefix..taskName

    -- check if task exists
    if redis.call("EXISTS", lTaskKey) == 1 then
      -- task exists, check task tm
      local rTm = redis.call("HGET", lTaskKey, "tm")
      if rTm ~= false and rTm ~= nil then
        if rTm == iOldTmId then
          -- this task is still under old tm's control
          local rOpt = redis.call("HGET", lTaskKey, "opt")
          if rOpt ~= false and rOpt ~= nil and tonumber(rOpt) == 1 then
            -- this task is still running, preempt it

            local rVer = redis.call("HGET", lTaskKey, "ver")
            if rVer ~= false and rVer ~= nil then
              redis.call("HMSET", lTaskKey, "ver", tonumber(rVer)+1, "tm", iMyTmId)
            else
              redis.call("HMSET", lTaskKey, "ver", 0, "tm", iMyTmId)
            end

            -- insert into my tm task list
            redis.call("HSET", lMyTmTaskKey, taskName, iHeartBeat)

            table.insert(collect, {taskName, redis.call("HGETALL", lTaskKey)})
            count = count + 1
          end
        end
      end
    end
  end
  i = i + 1
end
return {count, total, target, collect}
