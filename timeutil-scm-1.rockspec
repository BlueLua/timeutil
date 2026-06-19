package = "timeutil"
version = "scm-1"

source = {
  url = "git+https://github.com/BlueLua/timeutil.git",
}

description = {
  license = "MIT",
}

dependencies = {
  "lua >= 5.1",
}

build = {
  type = "builtin",
  modules = {
    timeutil = {
      sources = { "src/timeutil.c" },
    },
    ["timeutil.types/timeutil"] = "types/timeutil.d.lua",
  },
}
