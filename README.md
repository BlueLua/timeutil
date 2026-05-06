# timeutil ⏳

`timeutil` is a C-backed Lua module for wall-clock time, monotonic time, and
blocking sleep.

The module includes POSIX, Windows, and macOS-specific code paths, and is
intended for Lua 5.1, Lua 5.2, Lua 5.3, Lua 5.4, Lua 5.5, and LuaJIT.

> [!WARNING]
>
> Windows and macOS support is not fully verified. The local test suite runs on
> Linux/POSIX.

## Contents

- [Install](#install)
- [API](#api)
- [Development](#development)

## Install

```sh
luarocks install timeutil
```

## API

```lua
local timeutil = require("timeutil")
local start = timeutil.mono()
timeutil.sleep(0.05)
print(timeutil.now() - start)
```

### Functions

#### `timeutil.now()`

Returns Unix wall-clock time in seconds.

```lua
local time = require("timeutil")
local now = time.now()
local seconds = math.floor(now)
local ms = math.floor((now - seconds) * 1000)
local dt = string.format("%s.%03d", os.date("%Y-%m-%d %H:%M:%S", seconds), ms)
print(dt) --> YYYY-MM-DD HH:mm:ss.SSS
```

#### `timeutil.mono()`

Returns monotonic elapsed time in seconds.

```lua
local time = require("timeutil")
local monotime = time.mono

local start = monotime()
time.sleep(0.5)
local stop = monotime()

local elapsed_ms = math.floor((stop - start) * 1000)
print(elapsed_ms.."ms") --> 500ms
```

#### `timeutil.sleep(seconds)`

Blocks for a finite non-negative duration.

```lua
local time = require("timeutil")
time.sleep(1.5) -- sleep for 1.5 seconds
```

## Development

Use `scripts/build-and-test.sh` to build the rock with
[LuaRocks](https://luarocks.org) and run the tests across the Lua versions
installed on the machine.

For a local install, use
`luarocks --lua-version=5.x make rockspecs/timeutil-0.1.0-1.rockspec`.
