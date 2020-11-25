local cursor = "0"
local prefix = ARGV[1]
local filter = ARGV[2]
local output_keys = {}

local function isempty(s) return s == nil or s == "" end

local pattern = ""
if (isempty(filter)) then
    pattern = prefix .. "*"
else
    pattern = prefix .. "*" .. filter .. "*"
end

local i=0
repeat
    local result = redis.call("SCAN", cursor, "MATCH", pattern, "COUNT", 100000)
    local found_keys = result[2]
    for _, key in ipairs(found_keys) do output_keys[key] = 1 end
    cursor = result[1]
    i=i+1
until cursor == "0"


return {cjson.encode(output_keys)}
