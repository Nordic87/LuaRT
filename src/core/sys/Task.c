/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Task.c | LuaRT Task object implementation
*/

#define LUA_LIB

#include <luart.h>
#include <Task.h>

#include "async.h"

luart_type TTask;

const char *status[] = { "running", "created", "sleeping", "waiting", "paused", "terminated"};

//----------------------------------[ Task constructor ]
LUA_CONSTRUCTOR(Task) {
	Task *t = create_task(L);
	lua_newinstance(L, t, Task);
	lua_pushvalue(L, -1);
	t->taskref = luaL_ref(L, LUA_REGISTRYINDEX);
	return 1;
}

//----------------------------------[ Task.cancel() ]
LUA_METHOD(Task, cancel) {
	Task *t = lua_self(L, 1, Task);
	if (t->status < TTerminated) {
		lua_pushboolean(L, TRUE);
		t->status = TTerminated;
	} else lua_pushboolean(L, FALSE); 
	return 1;
}

//----------------------------------[ Task.wait() method ]
LUA_METHOD(Task, wait) {
	return waitfor_task(L, lua_self(L, 1, Task));
}

//----------------------------------[ Task.pause() method ]
LUA_METHOD(Task, pause) {
	pause_Task(lua_self(L, 1, Task));
	return 0;
}

//----------------------------------[ Task.resume() method ]
LUA_METHOD(Task, resume) {
	resume_Task(lua_self(L, 1, Task));
	return 0;
}

//----------------------------------[ Task.terminated property ]
LUA_PROPERTY_GET(Task, terminated) {
	lua_pushboolean(L, lua_self(L, 1, Task)->status == TTerminated);
	return 1;
}

//----------------------------------[ Task.status property ]
LUA_PROPERTY_GET(Task, status) {
	lua_pushstring(L, status[lua_self(L, 1, Task)->status]);
	return 1;
}

//----------------------------------[ Task.expired property ]
LUA_PROPERTY_GET(Task, expired) {
	Task *t = lua_self(L, 1, Task);
	lua_pushboolean(L, t->timeout > 0 && t->status == TTerminated);
	return 1;
}

//----------------------------------[ Task.timeout property ]
LUA_PROPERTY_SET(Task, timeout) {
	lua_self(L, 1, Task)->timeout = GetTickCount64() + luaL_checkinteger(L, 2);
	return 0;
}

LUA_PROPERTY_GET(Task, timeout) {
	lua_pushinteger(L, lua_self(L, 1, Task)->timeout - GetTickCount64());
	return 1;
}

//----------------------------------[ Task.priority property ]
LUA_PROPERTY_SET(Task, priority) {
	lua_self(L, 1, Task)->priority = luaL_checkinteger(L, 2);
	return 0;
}

LUA_PROPERTY_GET(Task, priority) {
	lua_pushinteger(L, lua_self(L, 1, Task)->priority);
	return 1;
}

//----------------------------------[ Task object definition ]
OBJECT_MEMBERS(Task)
	READONLY_PROPERTY(Task, terminated)
	READONLY_PROPERTY(Task, status)
	READONLY_PROPERTY(Task, expired)
	READWRITE_PROPERTY(Task, timeout)
	READWRITE_PROPERTY(Task, priority)
	METHOD(Task, cancel)
	METHOD(Task, pause)
	METHOD(Task, resume)
	METHOD(Task, wait)
END

//----------------------------------[ Task instance call ]
LUA_METHOD(Task, __call) {
	return start_task(L, lua_self(L, 1, Task), lua_gettop(L));
}

//----------------------------------[ Task destructor ]
LUA_METHOD(Task, __gc) {
	Task *t = lua_self(L, 1, Task);
	if (t->gc_func)
		t->gc_func(L);
	if (t->ref != LUA_NOREF)
		close_task(L, t);
	free(t);
	return 0;
}

OBJECT_METAFIELDS(Task)
	METHOD(Task, __call)
	METHOD(Task, __gc)
END