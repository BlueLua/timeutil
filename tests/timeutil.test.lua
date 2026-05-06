local time = require("timeutil")

-- Export surface.

local expected = { now = true, mono = true, sleep = true }
for name, value in pairs(time) do
  assert(expected[name], "unexpected export: " .. name)
  assert(type(value) == "function", "export is not a function: " .. name)
end

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
