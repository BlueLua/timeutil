#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <lauxlib.h>
#include <lua.h>

#include <math.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <float.h>
#include <windows.h>
#else
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
#include <stdint.h>
#endif
#endif

#if LUA_VERSION_NUM == 501
static void timeutil_newlib(lua_State *L, const luaL_Reg *funcs) {
  lua_newtable(L);
  luaL_register(L, NULL, funcs);
}
#else
#define timeutil_newlib luaL_newlib
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#define TIMEUTIL_EXPORT __declspec(dllexport)
#else
#define TIMEUTIL_EXPORT
#endif

#if defined(_WIN32) && defined(_MSC_VER)
static int timeutil_isfinite(double value) { return _finite(value); }
#else
static int timeutil_isfinite(double value) { return isfinite(value); }
#endif

static double timeutil_check_seconds(lua_State *L, int arg) {
  double seconds;

  seconds = (double)luaL_checknumber(L, arg);
  if (!timeutil_isfinite(seconds) || seconds < 0.0) {
    luaL_argerror(L, arg, "expected a finite non-negative number of seconds");
  }

  return seconds;
}

#if defined(_WIN32)
typedef VOID(WINAPI *timeutil_get_precise_time_fn)(LPFILETIME);

static double timeutil_windows_filetime_seconds(const FILETIME *ft) {
  ULARGE_INTEGER value;

  value.LowPart = ft->dwLowDateTime;
  value.HighPart = ft->dwHighDateTime;

  return ((double)value.QuadPart / 10000000.0) - 11644473600.0;
}

static int timeutil_wall_seconds(double *seconds) {
  HMODULE kernel32;
  FARPROC proc;
  FILETIME ft;

  kernel32 = GetModuleHandleA("kernel32.dll");
  proc = kernel32 ? GetProcAddress(kernel32, "GetSystemTimePreciseAsFileTime")
                  : NULL;

  if (proc != NULL) {
    ((timeutil_get_precise_time_fn)proc)(&ft);
  } else {
    GetSystemTimeAsFileTime(&ft);
  }

  *seconds = timeutil_windows_filetime_seconds(&ft);
  return 0;
}

static int timeutil_sleep_chunk(double seconds) {
  HANDLE timer;
  LARGE_INTEGER due_time;
  DWORD wait_result;
  double ticks;

  if (seconds <= 0.0) {
    return 0;
  }

  ticks = seconds * 10000000.0 + 0.5;
  if (ticks < 1.0) {
    ticks = 1.0;
  }

  timer = CreateWaitableTimerA(NULL, TRUE, NULL);
  if (timer == NULL) {
    Sleep((DWORD)(seconds * 1000.0 + 0.999));
    return 0;
  }

  due_time.QuadPart = -(LONGLONG)ticks;
  if (!SetWaitableTimer(timer, &due_time, 0, NULL, NULL, FALSE)) {
    CloseHandle(timer);
    Sleep((DWORD)(seconds * 1000.0 + 0.999));
    return 0;
  }

  wait_result = WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);

  return wait_result == WAIT_OBJECT_0 ? 0 : -1;
}

static int timeutil_sleep_seconds(double seconds) {
  while (seconds > 0.0) {
    double chunk;

    chunk = seconds > 86400.0 ? 86400.0 : seconds;
    if (timeutil_sleep_chunk(chunk) != 0) {
      return -1;
    }
    seconds -= chunk;
  }

  return 0;
}
#else
static int timeutil_wall_seconds(double *seconds) {
#if defined(CLOCK_REALTIME)
  struct timespec ts;

  if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
    return -1;
  }

  *seconds = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
  return 0;
#else
  struct timeval tv;

  if (gettimeofday(&tv, NULL) != 0) {
    return -1;
  }

  *seconds = (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
  return 0;
#endif
}

static int timeutil_sleep_seconds(double seconds) {
  while (seconds > 0.0) {
    struct timespec request;
    struct timespec remaining;
    double chunk;
    time_t whole;
    long nanoseconds;

    chunk = seconds > 86400.0 ? 86400.0 : seconds;
    whole = (time_t)chunk;
    nanoseconds = (long)((chunk - (double)whole) * 1000000000.0 + 0.5);

    if (nanoseconds >= 1000000000L) {
      whole += 1;
      nanoseconds -= 1000000000L;
    }

    request.tv_sec = whole;
    request.tv_nsec = nanoseconds;

    while (nanosleep(&request, &remaining) != 0) {
      if (errno != EINTR) {
        return -1;
      }
      request = remaining;
    }

    seconds -= chunk;
  }

  return 0;
}
#endif

static int timeutil_now(lua_State *L) {
  double seconds;

  if (timeutil_wall_seconds(&seconds) != 0) {
    return luaL_error(L, "failed to read wall-clock time");
  }

  lua_pushnumber(L, (lua_Number)seconds);
  return 1;
}

static int timeutil_mono(lua_State *L) {
  double seconds;

#if defined(_WIN32)
  LARGE_INTEGER frequency;
  LARGE_INTEGER counter;

  if (!QueryPerformanceFrequency(&frequency) ||
      !QueryPerformanceCounter(&counter)) {
    return luaL_error(L, "failed to read monotonic time");
  }

  seconds = (double)counter.QuadPart / (double)frequency.QuadPart;
#elif defined(__APPLE__) && defined(__MACH__)
  static mach_timebase_info_data_t timebase = {0, 0};
  uint64_t ticks;

  if (timebase.denom == 0) {
    if (mach_timebase_info(&timebase) != KERN_SUCCESS) {
      return luaL_error(L, "failed to read monotonic timebase");
    }
  }

  ticks = mach_absolute_time();
  seconds = ((double)ticks * (double)timebase.numer) /
            ((double)timebase.denom * 1000000000.0);
#elif defined(CLOCK_MONOTONIC)
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    return luaL_error(L, "failed to read monotonic time");
  }

  seconds = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
#elif defined(CLOCK_HIGHRES)
  struct timespec ts;

  if (clock_gettime(CLOCK_HIGHRES, &ts) != 0) {
    return luaL_error(L, "failed to read monotonic time");
  }

  seconds = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
#else
  return luaL_error(L, "no monotonic clock source available");
#endif

  lua_pushnumber(L, (lua_Number)seconds);
  return 1;
}

static int timeutil_sleep(lua_State *L) {
  double seconds;

  seconds = timeutil_check_seconds(L, 1);
  if (timeutil_sleep_seconds(seconds) != 0) {
    return luaL_error(L, "failed to sleep");
  }

  return 0;
}

static const luaL_Reg timeutil_functions[] = {{"now", timeutil_now},
                                              {"mono", timeutil_mono},
                                              {"sleep", timeutil_sleep},
                                              {NULL, NULL}};

#ifdef __cplusplus
extern "C"
#endif
TIMEUTIL_EXPORT int luaopen_timeutil(lua_State *L) {
  timeutil_newlib(L, timeutil_functions);
  return 1;
}
