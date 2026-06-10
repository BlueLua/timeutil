---@diagnostic disable

describe("timeutil", function()
  local time = require("timeutil")

  describe("exports", function()
    it("should export the correct functions and types", function()
      local expected = {
        now = "function",
        mono = "function",
        sleep = "function",
        _VERSION = "string",
      }
      for name, value in pairs(time) do
        local expected_type = expected[name]
        assert.Truthy(expected_type, "unexpected export: " .. name)
        assert.Equal(expected_type, type(value), "export is not a " .. expected_type .. ": " .. name)
      end
    end)

    it("should have the correct version string", function()
      assert.String(time._VERSION)
    end)
  end)

  describe("return types", function()
    it("now() should return a number", function()
      assert.Number(time.now())
    end)

    it("mono() should return a number", function()
      assert.Number(time.mono())
    end)

    it("sleep(0) should return nil", function()
      assert.Nil(time.sleep(0))
    end)
  end)

  describe("sleep duration", function()
    it("should sleep for approximately the requested duration", function()
      local before_sleep = time.mono()
      time.sleep(0.05)
      local after_sleep = time.mono()
      local slept = after_sleep - before_sleep

      assert.True(slept >= 0.04)
      assert.True(slept < 2.0)
    end)
  end)

  describe("clock monotonicity", function()
    it("now() and mono() should not move backwards", function()
      local now_before = time.now()
      local mono_before = time.mono()
      time.sleep(0.05)

      assert.True(time.now() >= now_before)
      assert.True(time.mono() >= mono_before)
    end)
  end)

  describe("input validation", function()
    it("sleep() should require a duration", function()
      assert.has_error(function()
        time.sleep()
      end)
    end)

    it("sleep() should reject negative durations", function()
      assert.has_error(function()
        time.sleep(-0.001)
      end)
    end)

    it("sleep() should reject NaN", function()
      assert.has_error(function()
        time.sleep(0 / 0)
      end)
    end)
  end)
end)
