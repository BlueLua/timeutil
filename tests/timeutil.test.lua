local time = require("timeutil")

local expected = { now = "function", mono = "function", sleep = "function", _VERSION = "string" }
for name, value in pairs(time) do
  local expected_type = expected[name]
  assert(expected_type, "unexpected export: " .. name)
  assert(type(value) == expected_type, "export is not a " .. expected_type .. ": " .. name)
end

assert(time._VERSION == "timeutil 0.0.0", "unexpected version: " .. tostring(time._VERSION))

-- Return types.

local now = time.now
local mono = time.mono
local sleep = time.sleep

assert(type(now()) == "number", "now() must return a number")
assert(type(mono()) == "number", "mono() must return a number")
assert(sleep(0) == nil, "sleep() must return nil")

-- Sleep duration.

local before_sleep = mono()
sleep(0.05)
local after_sleep = mono()
local slept = after_sleep - before_sleep

assert(slept >= 0.04, "sleep(0.05) returned too early: " .. slept)
assert(slept < 2.0, "sleep(0.05) took unexpectedly long: " .. slept)

-- Clock monotonicity.

local now_before = now()
local mono_before = mono()
sleep(0.05)

assert(now() >= now_before, "now() moved backwards")
assert(mono() >= mono_before, "mono() moved backwards")

-- Input validation.

assert(not pcall(sleep), "sleep() must require a duration")
assert(not pcall(sleep, -0.001), "sleep() must reject negative durations")
assert(not pcall(sleep, 0 / 0), "sleep() must reject NaN")

print("timeutil tests passed")
