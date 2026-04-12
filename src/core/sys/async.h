/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | async.h | LuaRT async functions header
*/


#pragma once


#ifdef __cplusplus
extern "C" {
#endif
    
    #include <Task.h>

    extern const char *status[];
    
    int start_task(lua_State *L, Task *t, int nargs);

    Task *create_task(lua_State *L);

    void set_lua_update(lua_CFunction func);

    //-------- Start a created Task
    int start_task(lua_State *L, Task *t, int nargs);

    //-------- Search for the current running Task
    Task *search_task(lua_State *L);

    //-------- Disable Tasks debug hook
    void unhook_tasks(lua_State *L);

    //-------- Close a Task
    void close_task(lua_State *L, Task *t);
    
    //-------- Returns all actual Task objects in a table, returning the count as well
    int get_alltasks(lua_State *L);

    //-------- Resume a Task
    BOOL resume_task(lua_State *L, Task *t, int args);

    //-------- Count active Tasks
    int task_count();

    //-------- Pause Task
    void pause_Task(Task *t);
    
    //-------- Resume Task
    void resume_Task(Task *t);

    //-------- Task scheduler
    BOOL update_tasks(lua_State *L);

    //-------- Wait for a Task at specific index
    int waitfor_task(lua_State *L, Task *t);

    //-------- Wait for all tasks
    int waitall_tasks(lua_State *L);


#ifdef __cplusplus
}
#endif