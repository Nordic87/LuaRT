/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Zip.h | LuaRT Zip object header
*/


#pragma once

#include <luart.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char *checkFilename(lua_State *L, int idx);

//---------------------------------------- Zip type

typedef struct {
	luart_type		type;
	struct zip_t	*zip;
	char			*fname;
	int				level;
	char			mode;
} Zip;

LUA_CONSTRUCTOR(Zip);
extern const luaL_Reg Zip_methods[];
extern const luaL_Reg Zip_metafields[];

extern char *checkEntry(lua_State *L, int idx, luart_type t);

#define checkFilename(L, i) checkEntry(L, i, TFile)
#define checkDirectory(L, i) checkEntry(L, i, TDirectory)

#ifdef __cplusplus
}
#endif