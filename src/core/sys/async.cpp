/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | async.cpp | LuaRT async module
*/

#define LUA_LIB

#include <list>
#include "async.h"

extern "C" {
	LUA_API lua_Integer idleThreshold = 1;
	
}

static std::list<Task *> Tasks;
static lua_CFunction lua_update = NULL;

void close_task(lua_State *L, Task *t) {
	luaL_unref(L, LUA_REGISTRYINDEX, t->ref);
	luaL_unref(L, LUA_REGISTRYINDEX, t->taskref);
	t->ref = LUA_NOREF;
	lua_closethread(t->L, NULL);
	Tasks.remove(t);
}

void set_lua_update(lua_CFunction func) {
	lua_update = func;
}

static void hookf (lua_State *L, lua_Debug *ar) {
  static const char *const hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, "_HOOKKEY");
  lua_pushthread(L);
  if (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(int)ar->event]);  /* push event name */
    if (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    else lua_pushnil(L);
    lua_getinfo(L, "lS", ar);
    lua_pcall(L, 2, 0, 0);  /* call hook function */
  }
}

Task *create_task(lua_State *L) {
	Task *tt, *t = (Task*)calloc(1, sizeof(Task));

	t->L = lua_newthread(L);
	t->status = TCreated;
	t->ref = luaL_ref(L, LUA_REGISTRYINDEX);
	if ((tt = search_task(L)))
		t->from = tt;
	luaL_checktype(L, 2, LUA_TFUNCTION);
	lua_pushvalue(L, 2);
	lua_xmove(L, t->L, 1);
	Tasks.push_back(t);
	if (lua_getfield(L, LUA_REGISTRYINDEX, "_TASKHOOK")) {
		int mask, count;
		lua_getfield(L, -1, "mask");
		mask = lua_tointeger(L, -1);
		lua_getfield(L, -2, "count");
		count = lua_tointeger(L, -1);
		lua_pop(L, 2);
		if (!luaL_getsubtable(L, LUA_REGISTRYINDEX, "_HOOKKEY")) {
			lua_pushliteral(L, "k");
			lua_setfield(L, -2, "__mode"); 
			lua_pushvalue(L, -1);
			lua_setmetatable(L, -2);
		}
		if (L != t->L && !lua_checkstack(t->L, 1))
    		luaL_error(L, "stack overflow");
		lua_pushthread(t->L); 
		lua_xmove(t->L, L, 1); 
		lua_getfield(L, -3, "hook");  
		lua_rawset(L, -3); 
		lua_sethook(t->L, hookf, mask, count);
	}
	lua_pop(L, 1);
	return t;	
}

//-------- Search for the current running Task
Task *search_task(lua_State *L) {
	for (auto it = Tasks.begin(); it != Tasks.end(); ++it) 
		if ((*it)->L == L)
			return (*it);		
	return NULL;
} 

//-------- Disable debug hoook on Tasks
void unhook_tasks(lua_State *L) {
	for (auto it = Tasks.begin(); it != Tasks.end(); ++it) 
		lua_sethook((*it)->L, hookf, 0, 0);
} 

//-------- Start a created Task
int start_task(lua_State *L, Task *t, int nargs) {	
	if (nargs) {
		for (int i = 2; i <= nargs+1; i++)
			lua_pushvalue(L, i);
		lua_xmove(L, t->L, nargs);
	}
	t->status = TRunning;
	nargs = resume_task(L, t, nargs);
    if (!nargs) {
		lua_xmove(t->L, L, 1);
		lua_error(L);
	}
	return t->status == TTerminated ? t->nresults : 0;
}

//-------- Resume a Task
BOOL resume_task(lua_State *L, Task *t, int args) {
	int nresults = 0, status;
	int nargs = args != -1 ? args : lua_gettop(t->L)-1;

	if ( (t->status != TTerminated) && (status = lua_resume(t->L, L, t->status == TSleep ? 0 : nargs, &nresults)) > 1 )
		return false;
	if (status == LUA_YIELD) {
		lua_xmove(t->L, L, nresults);
		t->status = TSleep;
	} else if (status == LUA_OK)
		t->status = TTerminated;
	t->nresults = nresults;
	return true;
}

//-------- Task scheduler
BOOL update_tasks(lua_State *L) {
    Tasks.sort([](Task* a, Task* b) {
        return a->priority > b->priority;
    });

    for (auto it = Tasks.begin(); it != Tasks.end(); ++it) {
        Task *t = *it;
        if (t->status == TSleep) {
            if (t->sleep <= GetTickCount64()) {
                t->sleep = 0;
                t->status = TRunning;
            }
        }
    }

    for (auto &t : Tasks) {
        if (t->timeout > 0 && GetTickCount64() >= t->timeout)
            t->status = TTerminated;
    }

    // Process runnable tasks
    for (auto it = Tasks.begin(); it != Tasks.end();) {
        Task *t = *it;
        if (t->status == TRunning) {
            if (lua_status(t->L) == LUA_YIELD) {
                if (!resume_task(L, t, -1)) {
				lua_xmove(t->L, L, 1);
                    return false;
                }
			}
        }

        if (t->status == TTerminated && !t->iswaiting) {
            it = Tasks.erase(it);
            if (lua_rawgeti(t->L, LUA_REGISTRYINDEX, t->taskref)) {
                if (lua_getfield(t->L, -1, "after") == LUA_TFUNCTION) {
                    lua_insert(t->L, -t->nresults - 2);
                    lua_insert(t->L, -t->nresults - 2);
                    if (lua_pcall(t->L, t->nresults, LUA_MULTRET, 0) != LUA_OK) {
                        lua_xmove(t->L, L, 1);
                        return lua_error(L);
                    }
                }
            }
            if (!t->timeout)
                close_task(L, t);
        } else
            ++it;
    }
	static lua_Integer idle = idleThreshold;
	if (idleThreshold)
		if (--idle == 0) {
			Sleep(1); 
			idle = idleThreshold;
		}
    return true;
}

//-------- Wait for a Task
int waitfor_task(lua_State *L, Task *t) {
	t->iswaiting = TRUE;
	do {
		if (!update_tasks(L))
			lua_error(L);	
	} while(t->status != TTerminated);
	if (t->nresults)
		lua_xmove(t->L, L, t->nresults);	
	int nresults = t->nresults;
	close_task(L, t);
	return nresults;
}

//--------- Pause/resume Task
void pause_Task(Task *t) {
    if (t->status == TRunning || t->status == TSleep)
        t->status = TPaused;
}

void resume_Task(Task *t) {
    if (t->status == TPaused)
        t->status = TRunning;
}

//--------- Get active Tasks count
int task_count() {
	int count = 0;
    for (auto &t : Tasks) {
        if (t->status != TTerminated)
            ++count;
    }
    return count;
}

//-------- Returns all actual Task objects in a table, returning the count as well
int get_alltasks(lua_State *L) {
	lua_createtable(L, Tasks.size(), 0);
	int i = 0;
	for (auto &t : Tasks) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, t->taskref);
		lua_rawseti(L, -2, ++i);
	}
	return i;
}

//-------- Wait for all Tasks
int waitall_tasks(lua_State *L) {
	int nargs = lua_gettop(L);
	if (!nargs) {
		do
			if (!update_tasks(L))
				lua_error(L);
		while (Tasks.size() > 1);	
		return 0;
	} else {
		int i = 1;
		lua_createtable(L, nargs, 0);
		while (i <= nargs) {
			Task *t = lua_self(L, i, Task);
			lua_createtable(L, 0, 0);
			t->iswaiting = TRUE;
			do {
				if (!update_tasks(L))
					lua_error(L);	
			} while(t->status != TTerminated);
			if (t->nresults) {
				lua_xmove(t->L, L, t->nresults);	
				for (int j = t->nresults; j > 0; j--)
					lua_rawseti(L, -j-1, j);
			}
			lua_rawseti(L, -2, i);
			i++;
			close_task(L, t);
		}
	}
    return 1;
}
