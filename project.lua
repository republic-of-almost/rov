#!lua

--[[
  Render O Vision Project Description
]]--

table.insert(
  projects,
  {
    name     = "ROV",
    kind     = "StaticLib",
    language = "C++",
    location = make.get_proj_root(),

    src_files = {
      make.add_headers(make.get_proj_root()),
      make.add_src(make.get_proj_root()),
      make.add_doc(make.get_proj_root()),
    },

    inc_dirs = {
      make.get_proj_root() .. "./",
    },

    project_dependencies = {
      "External",
      "Utilities",
      "Math",
    },
  }
)
