/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | luaRT.h | LuaRT API header
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lua\lua.h"
#include "lua\lauxlib.h"
#include "lua\lualib.h"

#include <windows.h>
#include <CommCtrl.h>

//--------------------------------------------------| LuaRT _VERSION
#define xstr(s) ___str(s)
#define ___str(s) #s
#undef LUA_VERSION_MAJOR
#undef LUA_VERSION_MINOR
#undef LUA_VERSION_RELEASE
#define LUA_VERSION_MAJOR	xstr(LUART_MAJOR)
#define LUA_VERSION_MINOR	xstr(LUART_MINOR)
#define LUA_VERSION_RELEASE	xstr(LUART_RELEASE)

#undef LUA_VERSION
#define LUA_VERSION	"LuaRT " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE

//--------------------------------------------------| Object declaration macros

//--- Define object method
#define LUA_METHOD(t, n) int t##_##n(lua_State *L)

//--- Define object constructor
#define LUA_CONSTRUCTOR(t) int t##_constructor(lua_State *L)

//--- Define object get/set properties
#define LUA_PROPERTY_GET(t, n) int t##_get##n(lua_State *L)
#define LUA_PROPERTY_SET(t, n) int t##_set##n(lua_State *L)

//--- Register property
#define READWRITE_PROPERTY(obj, property) \
    {"get_" #property,  obj##_get##property}, \
    {"set_" #property,  obj##_set##property},

#define READONLY_PROPERTY(obj, property) \
    {"get_" #property,  obj##_get##property},

//--- Register method
#define METHOD(obj, method) \
    {#method,  obj##_##method},

//--- Register module methods
#define MODULE_FUNCTIONS(module) \
static const luaL_Reg module##lib[] = {

#define MODULE_PROPERTIES(module) \
const luaL_Reg module##_properties[] = {

//--- Register object methods
#define OBJECT_MEMBERS(obj) \
const luaL_Reg obj##_methods[] = {

//--- Register object metefields
#define OBJECT_METAFIELDS(obj) \
const luaL_Reg obj##_metafields[] = {


#define END \
    {NULL, NULL} \
};

//--------------------------------------------------| Internal Object management

//--- Internal object type id
typedef int luart_type;
#define LUART_OBJECTS "LuaRT Objects"

//--------------------------------------------------| Object registration

//--- Register object function
LUA_API int lua_registerobject(lua_State *L, luart_type *type, const char *_typename, lua_CFunction constructor, const luaL_Reg *methods, const luaL_Reg *mt);

//--- Object registration macros
#define lua_regobjectmt(L, _typename) lua_registerobject(L, &T##_typename, #_typename, _typename##_constructor, _typename##_methods, _typename##_metafields)
#define lua_regobject(L, _typename) lua_registerobject(L, &T##_typename, #_typename, _typename##_constructor, _typename##_methods, NULL)

//--------------------------------------------------| Lua object manipulation

//--- Push a new instance from a Lua object at index idx on stack
LUA_API int lua_createinstance(lua_State *L, int idx);

//--- Push a new Lua object instance of a 'objectname' object
LUA_API void lua_checkinstance(lua_State *L, int idx, const char *objectname);

//--- Check if value at index idx is a Lua instance, setting its name to objectname
LUA_API int lua_isinstance(lua_State *L, int idx, const char **objectname);

//--- Check if value at index idx is a Lua object
LUA_API int lua_isobject(lua_State *L, int idx);

//--------------------------------------------------| C object manipulation

//--- Initialize a new C object instance, to be called only in the constructor
LUA_API void lua_createcinstance(lua_State *L, void *t, luart_type type);
#define lua_newinstance(L, t, _type) lua_createcinstance(L, t, T##_type)

//--- Returns any instance at index position or error with type t expected
LUA_API void *lua_toself(lua_State *L, int idx, luart_type t);

//--- Returns object at specified index, and gets its type
LUA_API void *lua_tocinstance(lua_State *L, int idx, luart_type *t);

//--- Returns instance of specified type at index or NULL
LUA_API void *lua_iscinstance(lua_State *L, int idx, luart_type t);

//--- Returns object of specified type at index or throws an error
LUA_API void *lua_checkcinstance(lua_State *L, int idx, luart_type t);
#define luaL_checkcinstance(L, idx, type) ((type *)lua_checkcinstance(L, idx, T##type))

//--- Gets object at specified index without checking its type
#define lua_self(L, idx, type) ((type*)lua_toself(L, idx, T##type))

//--- Pushes a new instance from object name with the lasts narg arguments on stack as constructor arguments
LUA_API void *lua_pushnewinstancebyname(lua_State *L, luart_type type, int narg);
LUA_API void *lua_pushnewinstance(lua_State *L, const char *_typename, int narg);
#define lua_pushinstance(L, type, narg) lua_pushnewinstance(L, #type, narg)

//--- Gets value type name at the specified index (supporting objects type names)
LUA_API const char *lua_objectname(lua_State *L, int idx);

//--- Uses lua_objectname for luaL_Typename implementation
#undef luaL_typename
#define luaL_typename(L, i) lua_objectname(L, i)

//--- Returns the inherited Object
LUA_API int lua_super(lua_State *L, int idx);

//--------------------------------------------------| Module registration

//--- Registers module and pushes it on the stack
LUA_API void lua_registermodule(lua_State *L, const char *name, const luaL_Reg *functions, const luaL_Reg *properties, lua_CFunction finalizer);

//--- Module registration macros with or without a finalizer (called upon module garbage collection)
#define lua_regmodule(L, m) lua_registermodule(L, #m, m##lib, m##_properties, NULL)
#define lua_regmodulefinalize(L, m) lua_registermodule(L, #m, m##lib, m##_properties, m##_finalize)

//--------------------------------------------------| Wide string functions

//--- Checks for a string at the specified index and get it as a wide string
LUA_API wchar_t *lua_tolwstring(lua_State *L, int idx, int *len);

//--- Pushes a wide string onto the stack
LUA_API void lua_pushlwstring(lua_State *L, const wchar_t *str, int len);

//--- Wide strings macros
#define lua_towstring(L, i)		lua_tolwstring(L, i, NULL)
#define lua_pushwstring(L, s)	lua_pushlwstring(L, s, -1)

//--------------------------------------------------| Preprocessor functions

//--- C-compatible wrapper around luaL_loadfilex with preprocessing
LUA_API int luaL_loadfilep(lua_State* L, const char* filename, const char* mode);

//--------------------------------------------------| Various utility functions

//--- Returns index of string at specfied index, in a an array of options strings, or the default specified index
LUA_API int lua_optstring(lua_State *L, int idx, const char *options[], int def);

//-- Close Lua state and call atexit function if exists
LUA_API void lua_stop(void);

//--------------------------------------------------| Open modules function
LUALIB_API void lua_openmodules(lua_State *L);

//--- Returns an array of bytes if the specified executable has embedded content or NULL otherwise, and loads global "embed" module
LUA_API BYTE *luaL_embedopen(lua_State *L);

//--- Closes embedded content previously opened with luaL_embedopen()
LUA_API int luaL_embedclose(lua_State *L);

//--- luaL_setfuncs() alternative with lua_rawset() and without upvalues
LUA_API void luaL_setrawfuncs(lua_State *L, const luaL_Reg *l);

//--- luaL_require() alternative to luaL_requiref()
LUA_API void luaL_require(lua_State *L, const char *modname);

typedef struct Task Task;

//--- Push a task with the provided continuation C function and starts it, with a context and optional cleanup lua_CFunction
//--- Always returns 1
LUA_API Task *lua_pushtask(lua_State *L, lua_KFunction taskfunc, void *userdata, lua_CFunction gc);

//--- Sleeps the current task or the current Lua state for the provided delay
LUA_API void lua_sleep(lua_State *L, lua_Integer delay);

//--- Throws a new event from the current object at the top of the stack, followed by its parameters
LUA_API int lua_throwevent(lua_State *L, const char *name, int nparams);

//--- Wait for the Task at index idx to terminate
LUA_API int lua_wait(lua_State *L, Task *t);

//--- Get the number of current active Tasks
LUA_API int lua_taskcount();

//--- Get the current executing Task
LUA_API Task *lua_gettask(lua_State *L);

//--- Schedule all current tasks and return the number of results is a Task is terminated
LUA_API int lua_schedule(lua_State *L);

//--------------------------------------------------| LuaRT utility functions

//--- Pushes Windows system error string on stack
LUA_API int luaL_getlasterror(lua_State *L, DWORD err);

//--- Get a filename at specified index, from a string or from a File object
//--- result must be manually freed once used
LUA_API wchar_t *luaL_checkFilename(lua_State *L, int idx);

//--- Get a filename at specified index, from a string or from a Directory object
//--- result must be manually freed once used
LUA_API wchar_t *luaL_checkDirname(lua_State *L, int idx);

//--------------- Buffer functions

//--- Push a Buffer on the stack given a pointer and the size of the buffer
LUA_API void lua_pushBuffer(lua_State *L, void *p, size_t len);

//--- Converts the Object at the specified stack index to a Buffer pointer
typedef struct Buffer Buffer;

LUA_API Buffer *lua_toBuffer(lua_State *L, int idx);

//--------------- zip decompression functions
struct zip_t;
#ifdef _WIN64
typedef long long ssize_t; /* byte count or error */
#else
typedef long ssize_t; /* byte count or error */
#endif
LUA_API ssize_t zip_entry_read(struct zip_t *zip, void **buf, size_t *bufsize);
LUA_API int zip_entry_open(struct zip_t *zip, const char *entryname);
LUA_API int zip_entry_close(struct zip_t *zip);
LUA_API int zip_entry_fread(struct zip_t *zip, const char *filename);

LUA_API luart_type TTask;
LUA_API luart_type TFile;
LUA_API luart_type TBuffer;
LUA_API luart_type TCOM;
LUA_API luart_type TDatetime;
LUA_API luart_type TPipe;
LUA_API luart_type TZip;
LUA_API luart_type TDirectory;

//-------------------- Lua Clipboard format
LUA_API HGLOBAL table_to_HDROPFormat(lua_State *L, int idx);

#ifdef __cplusplus
}
#endif