-- NOTE: This file is modified from the standard distribution.
-- It now only generates a .c file with the needed table in it.

-- Configuration part
local files = { "./lua/lua.h", "./lua/lauxlib.h", "./lua/lualib.h", "../../include/luart_dynamic.h" }
local dstHfile = "../../include/luart_static.h"
local dstCfile = "./lua/static/luart_static.cpp"
local headerdef = "LUA_DYNAMIC_H"

local outfile = assert(io.open(dstHfile, "w"))
local function out(...)
	outfile:write(string.format(...))
end

local function suffix(name)
	if name:match("^lua_") then name = name:sub(5) 
	elseif name:match("^luaL_") then name = name:sub(6).."L" 
	else name = name:sub(4)
	end
	name = name:sub(1,1):upper() .. name:sub(2)
	return name
end

out('/* Lua language header file, designed for dynamic loading. \n')
out('   This file is not part of the standard distribution, but was\n')
out('   generated from original files %s */\n\n', table.concat(files, ', '))
out('#ifndef %s\n#define %s\n\n#ifdef __cplusplus\nextern "C" {\n#endif\n', headerdef, headerdef)

out [[
#include "lua\luaconf.h"

#define LUA_PREFIX staticlua
]]

-- for k,v in pairs(conf_defines) do
-- 	out('#define %s %s\n', k, v)
-- end
local fcts = {}
local vars = {}
for _,file in ipairs(files) do
	local f = assert(io.open(file, "r"))
	local data = f:read("*all")
	data = data:gsub('#ifdef __cplusplus\nextern "C" {\n#endif', "") -- remove extern C
	data = data:gsub('#ifdef __cplusplus\n}\n#endif', "") -- remove extern C
	data = data:gsub("/%*%*%*.-%*%*%*/", function(a) out('\n%s\n',a) return "" end) -- exports copyright
	data = data:gsub("/%*.-%*/", "") -- remove comments to simplify parsing
	data = data:gsub('#include ".-\n', "") -- remove local includes
	data = data:gsub("^%s*#ifndef [%w_]+\n#define [%w_]+\n(.*)#endif", "%1") -- removes header sentinel
	data = data:gsub("#if defined%(LUA_COMPAT_GETN%)\n(.-)#else\n(.-)#endif", 
		function (doif, doelse)
			if  conf_defines.LUA_COMPAT_GETN then return doif else return doelse end
		end)

	data = data:gsub("#if !?defined(.-)#endif", "")
	data = data:gsub("\n\n+", "\n\n") -- remove extra new lines 
	-- if conf_defines.LUA_COMPAT_OPENLIB then
	-- 	data = data:gsub("luaI_openlib", "luaL_openlib") -- remapped function: care!
	-- end
	-- Some function declarations lacks the extra parenthesis around their name
	data = data:gsub("(LUA_API%s[^%(%)%;]*)(luaL?_%w+)%s*(%([^%(%)]+%);)", "%1 (%2) %3")
	-- Detect external functions and transforms them into typedefs
	-- data = data:gsub("LUA%w*_API%s+([%w_%*%s]-)%s*%(([%w_]+)%)%s*%((.-)%);", 
	data = data:gsub("LUA%w*_API%s+([%w_%*%s]-)%s*([%(%w_%)]+)%s*%((.-)%);", function(type, name, args)
		if #type == 0 then
			type = name
			__, __, name, args = args:find("([%w_]-)%)%s*%((.*)")
		end
		name = name:match("%((.-)%)") or name
		fcts[#fcts+1] = name
		return string.format("typedef %s (__cdecl *%s_t) (%s);", type, name, args)
	end)
	for type, name in data:gmatch("LUA_API%s+([%w_%*%s]-)%s*([%w_]-);") do
		vars[#vars+1] = { ["type"] = type, ["name"] = name }
	end
	out("%s\n", data)
	f:close()
end
table.sort(fcts)
for _,f in pairs(fcts) do
	out('#define %-23s LUA_PREFIX.%s\n', f, suffix(f))
end
for _,var in pairs(vars) do
	out('#define %-23s (*LUA_PREFIX.var_%s)\n', var.name, var.name)
end
out '\ntypedef struct lua_All_functions\n{\n'
for _,f in pairs(fcts) do
	out('  %-23s %s;\n', f.."_t", suffix(f))
end
for _,var in pairs(vars) do
	out('  %-23s *var_%s;\n', var.type, var.name)
end
-- Write footer (verbatim string)
out '} lua_All_functions;\n\n'
out 'extern lua_All_functions staticlua;\n\n'

out '#ifdef __cplusplus\n}\n#endif\n#endif\n'

outfile:close()

outfile = assert(io.open(dstCfile, "w"))
-- Write C loader function (verbatim)
local var_names = {}
for _, var in ipairs(vars) do
   var_names[#var_names+1] = var.name
end
out ([[
extern "C" static const char* const LuaFunctionNames[] = 
{
	"%s"
};
]], table.concat(fcts, '",\n\t"')..'",\n\t"'..table.concat(var_names, '",\n\t"'), "")
out("lua_All_functions staticlua;")
outfile:close()

print("Generated "..dstHfile.." and "..dstCfile)
