/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Task.h | LuaRT Task object header
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <luart.h>
#include <windows.h>


typedef enum { TRunning, TCreated, TSleep, TWaiting, TPaused, TTerminated } TaskStatus;

struct Task {
	luart_type	type;
	lua_State 	*L;
	Task		*from;
	TaskStatus	status;
	int			ref;
	int 		taskref;
	int			nresults;
	BOOL 		iswaiting;
	ULONGLONG	sleep;
	ULONGLONG	timeout;
	int			priority;
	void		*userdata;
	lua_CFunction gc_func;
 };

//---------------------------------------- Task object
LUA_CONSTRUCTOR(Task);
extern const luaL_Reg Task_methods[];
extern const luaL_Reg Task_metafields[];

#ifdef __cplusplus
}
#endif