local appid = "appid:" .. KEYS[1]

local members = redis.call('SMEMBERS', appid)
local result = {}

for idx, liveId in pairs(members) do
    local keyv = liveId .. "$v"
    local ver = redis.call("GET", keyv)
    if (not ver) then
        redis.call("SREM", appid, liveId)
    else
        local ret = string.format("%s,%s", liveId, ver)
        table.insert(result, ret)
    end
end

return result