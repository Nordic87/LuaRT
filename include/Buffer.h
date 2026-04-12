/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Buffer.h | LuaRT Buffer object header
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <luart.h>
#include <stdlib.h>


struct Buffer {
	luart_type		type;
	size_t			size;
	BYTE			*bytes;
	int				encoding;
};

LUA_CONSTRUCTOR(Buffer);
extern const luaL_Reg Buffer_methods[];
extern const luaL_Reg Buffer_metafields[];

int base64_encode(lua_State *L, Buffer *b);

#ifdef __cplusplus
}
#endif