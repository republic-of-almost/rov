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
      -- Headers
      make.get_proj_root() .. "**.hpp",
      make.get_proj_root() .. "**.h",

      -- Source
      make.get_proj_root() .. "**.c",
      make.get_proj_root() .. "**.cpp",
      make.get_proj_root() .. "**.mm",
      make.get_proj_root() .. "**.m",

      -- Doc
      make.get_proj_root() .. "**.md",
      make.get_proj_root() .. "**.txt",
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
