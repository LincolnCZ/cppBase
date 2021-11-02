local iTmSet = KEYS[1]
local iTmId = KEYS[2]
local iTmIdKey = KEYS[3]
local iIp = KEYS[4]
local iPort = KEYS[5]
local iHeartbeat = KEYS[6]
local iIpStr = KEYS[7]

redis.call("SADD", iTmSet, iTmId)
redis.call("HMSET", iTmIdKey,
        "ip", iIp,
        "port", iPort,
        "ts", iHeartbeat,
        "ipStr", iIpStr
)
