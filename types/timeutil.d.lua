---@meta timeutil

---
---Wall-clock time, monotonic time, and native sleep utilities.
---
---```lua
---local timeutil = require("timeutil")
---local start = timeutil.mono()
---timeutil.sleep(0.1)
---print(timeutil.now() - start)
---```
---
---@class timeutil
local M = {}

---
---Returns wall-clock Unix time in seconds.
---
---```lua
---local time = require("timeutil")
---local now = time.now()
---local seconds = math.floor(now)
---local ms = math.floor((now - seconds) * 1000)
---local dt = string.format("%s.%03d", os.date("%Y-%m-%d %H:%M:%S", seconds), ms)
---print(dt) --> YYYY-MM-DD HH:mm:ss.SSS
---```
---
---@return number seconds
---@nodiscard
function M.now() end

---
---Returns monotonic elapsed time in seconds.
---
---```lua
---local time = require("timeutil")
---local monotime = time.mono
---
---local start = monotime()
---time.sleep(0.5)
---local stop = monotime()
---
---local elapsed_ms = math.floor((stop - start) * 1000)
---print(elapsed_ms.."ms") --> 500ms
---```
---
---@return number seconds
---@nodiscard
function M.mono() end

---
---Blocks the current thread for the requested non-negative duration.
---
---```lua
---local time = require("timeutil")
---time.sleep(1.5) -- sleep for 1.5 seconds
---```
---
---@param seconds number
function M.sleep(seconds) end

return M
