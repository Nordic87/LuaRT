/* Lua language header file, designed for dynamic loading. 
   This file is not part of the standard distribution, but was
   generated from original files ./lua/lua.h, ./lua/lauxlib.h, ./lua/lualib.h, ../../include/luart_dynamic.h */

#ifndef LUA_DYNAMIC_H
#define LUA_DYNAMIC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lua\luaconf.h"

#define LUA_PREFIX staticlua

/******************************************************************************
* Copyright (C) 1994-2026 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include <stdarg.h>
#include <stddef.h>

#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"

#define LUA_VERSION_MAJOR_N	5
#define LUA_VERSION_MINOR_N	5
#define LUA_VERSION_RELEASE_N	0

#define LUA_VERSION_NUM  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_NUM  (LUA_VERSION_NUM * 100 + LUA_VERSION_RELEASE_N)

#define LUA_SIGNATURE	"\x1bLua"

#define LUA_MULTRET	(-1)

#define LUA_REGISTRYINDEX	(-(INT_MAX/2 + 1000))
#define lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))

#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5

typedef struct lua_State lua_State;

#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_NUMTYPES		9

#define LUA_MINSTACK	20

#define LUA_RIDX_GLOBALS	2
#define LUA_RIDX_MAINTHREAD	3
#define LUA_RIDX_LAST		3

typedef LUA_NUMBER lua_Number;

typedef LUA_INTEGER lua_Integer;

typedef LUA_UNSIGNED lua_Unsigned;

typedef LUA_KCONTEXT lua_KContext;

typedef int (*lua_CFunction) (lua_State *L);

typedef int (*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);

typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void *p, size_t sz, void *ud);

typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);

typedef void (*lua_WarnFunction) (void *ud, const char *msg, int tocont);

typedef struct lua_Debug lua_Debug;

typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);

extern const char lua_ident[];

typedef lua_State * (__cdecl *lua_newstate_t) (lua_Alloc f, void *ud, unsigned seed);
typedef void (__cdecl *lua_close_t) (lua_State *L);
typedef lua_State * (__cdecl *lua_newthread_t) (lua_State *L);
typedef int (__cdecl *lua_closethread_t) (lua_State *L, lua_State *from);

typedef lua_CFunction (__cdecl *lua_atpanic_t) (lua_State *L, lua_CFunction panicf);

typedef lua_Number (__cdecl *lua_version_t) (lua_State *L);

typedef int (__cdecl *lua_absindex_t) (lua_State *L, int idx);
typedef int (__cdecl *lua_gettop_t) (lua_State *L);
typedef void (__cdecl *lua_settop_t) (lua_State *L, int idx);
typedef void (__cdecl *lua_pushvalue_t) (lua_State *L, int idx);
typedef void (__cdecl *lua_rotate_t) (lua_State *L, int idx, int n);
typedef void (__cdecl *lua_copy_t) (lua_State *L, int fromidx, int toidx);
typedef int (__cdecl *lua_checkstack_t) (lua_State *L, int n);

typedef void (__cdecl *lua_xmove_t) (lua_State *from, lua_State *to, int n);

typedef int (__cdecl *lua_isnumber_t) (lua_State *L, int idx);
typedef int (__cdecl *lua_isstring_t) (lua_State *L, int idx);
typedef int (__cdecl *lua_iscfunction_t) (lua_State *L, int idx);
typedef int (__cdecl *lua_isinteger_t) (lua_State *L, int idx);
typedef int (__cdecl *lua_isuserdata_t) (lua_State *L, int idx);
typedef int (__cdecl *lua_type_t) (lua_State *L, int idx);
typedef const char     * (__cdecl *lua_typename_t) (lua_State *L, int tp);

typedef lua_Number (__cdecl *lua_tonumberx_t) (lua_State *L, int idx, int *isnum);
typedef lua_Integer (__cdecl *lua_tointegerx_t) (lua_State *L, int idx, int *isnum);
typedef int (__cdecl *lua_toboolean_t) (lua_State *L, int idx);
typedef const char     * (__cdecl *lua_tolstring_t) (lua_State *L, int idx, size_t *len);
typedef lua_Unsigned (__cdecl *lua_rawlen_t) (lua_State *L, int idx);
typedef lua_CFunction (__cdecl *lua_tocfunction_t) (lua_State *L, int idx);
typedef void	       * (__cdecl *lua_touserdata_t) (lua_State *L, int idx);
typedef lua_State      * (__cdecl *lua_tothread_t) (lua_State *L, int idx);
typedef const void     * (__cdecl *lua_topointer_t) (lua_State *L, int idx);

#define LUA_OPADD	0	
#define LUA_OPSUB	1
#define LUA_OPMUL	2
#define LUA_OPMOD	3
#define LUA_OPPOW	4
#define LUA_OPDIV	5
#define LUA_OPIDIV	6
#define LUA_OPBAND	7
#define LUA_OPBOR	8
#define LUA_OPBXOR	9
#define LUA_OPSHL	10
#define LUA_OPSHR	11
#define LUA_OPUNM	12
#define LUA_OPBNOT	13

typedef void (__cdecl *lua_arith_t) (lua_State *L, int op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

typedef int (__cdecl *lua_rawequal_t) (lua_State *L, int idx1, int idx2);
typedef int (__cdecl *lua_compare_t) (lua_State *L, int idx1, int idx2, int op);

typedef void (__cdecl *lua_pushnil_t) (lua_State *L);
typedef void (__cdecl *lua_pushnumber_t) (lua_State *L, lua_Number n);
typedef void (__cdecl *lua_pushinteger_t) (lua_State *L, lua_Integer n);
typedef const char * (__cdecl *lua_pushlstring_t) (lua_State *L, const char *s, size_t len);
typedef const char * (__cdecl *lua_pushexternalstring_t) (lua_State *L,
		const char *s, size_t len, lua_Alloc falloc, void *ud);
typedef const char * (__cdecl *lua_pushstring_t) (lua_State *L, const char *s);
typedef const char * (__cdecl *lua_pushvfstring_t) (lua_State *L, const char *fmt,
                                                      va_list argp);
typedef const char * (__cdecl *lua_pushfstring_t) (lua_State *L, const char *fmt, ...);
typedef void (__cdecl *lua_pushcclosure_t) (lua_State *L, lua_CFunction fn, int n);
typedef void (__cdecl *lua_pushboolean_t) (lua_State *L, int b);
typedef void (__cdecl *lua_pushlightuserdata_t) (lua_State *L, void *p);
typedef int (__cdecl *lua_pushthread_t) (lua_State *L);

typedef int (__cdecl *lua_getglobal_t) (lua_State *L, const char *name);
typedef int (__cdecl *lua_gettable_t) (lua_State *L, int idx);
typedef int (__cdecl *lua_getfield_t) (lua_State *L, int idx, const char *k);
typedef int (__cdecl *lua_geti_t) (lua_State *L, int idx, lua_Integer n);
typedef int (__cdecl *lua_rawget_t) (lua_State *L, int idx);
typedef int (__cdecl *lua_rawgeti_t) (lua_State *L, int idx, lua_Integer n);
typedef int (__cdecl *lua_rawgetp_t) (lua_State *L, int idx, const void *p);

typedef void (__cdecl *lua_createtable_t) (lua_State *L, int narr, int nrec);
typedef void * (__cdecl *lua_newuserdatauv_t) (lua_State *L, size_t sz, int nuvalue);
typedef int (__cdecl *lua_getmetatable_t) (lua_State *L, int objindex);
typedef int (__cdecl *lua_getiuservalue_t) (lua_State *L, int idx, int n);

typedef void (__cdecl *lua_setglobal_t) (lua_State *L, const char *name);
typedef void (__cdecl *lua_settable_t) (lua_State *L, int idx);
typedef void (__cdecl *lua_setfield_t) (lua_State *L, int idx, const char *k);
typedef void (__cdecl *lua_seti_t) (lua_State *L, int idx, lua_Integer n);
typedef void (__cdecl *lua_rawset_t) (lua_State *L, int idx);
typedef void (__cdecl *lua_rawseti_t) (lua_State *L, int idx, lua_Integer n);
typedef void (__cdecl *lua_rawsetp_t) (lua_State *L, int idx, const void *p);
typedef int (__cdecl *lua_setmetatable_t) (lua_State *L, int objindex);
typedef int (__cdecl *lua_setiuservalue_t) (lua_State *L, int idx, int n);

typedef void (__cdecl *lua_callk_t) (lua_State *L, int nargs, int nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, NULL)

typedef int (__cdecl *lua_pcallk_t) (lua_State *L, int nargs, int nresults, int errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, NULL)

typedef int (__cdecl *lua_load_t) (lua_State *L, lua_Reader reader, void *dt,
                          const char *chunkname, const char *mode);

typedef int (__cdecl *lua_dump_t) (lua_State *L, lua_Writer writer, void *data, int strip);

typedef int (__cdecl *lua_yieldk_t) (lua_State *L, int nresults, lua_KContext ctx,
                               lua_KFunction k);
typedef int (__cdecl *lua_resume_t) (lua_State *L, lua_State *from, int narg,
                               int *nres);
typedef int (__cdecl *lua_status_t) (lua_State *L);
typedef int (__cdecl *lua_isyieldable_t) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, NULL)

typedef void (__cdecl *lua_setwarnf_t) (lua_State *L, lua_WarnFunction f, void *ud);
typedef void (__cdecl *lua_warning_t) (lua_State *L, const char *msg, int tocont);

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCISRUNNING		6
#define LUA_GCGEN		7
#define LUA_GCINC		8
#define LUA_GCPARAM		9

#define LUA_GCPMINORMUL		0  
#define LUA_GCPMAJORMINOR	1  
#define LUA_GCPMINORMAJOR	2  

#define LUA_GCPPAUSE		3  
#define LUA_GCPSTEPMUL		4  
#define LUA_GCPSTEPSIZE		5  

#define LUA_GCPN		6

typedef int (__cdecl *lua_gc_t) (lua_State *L, int what, ...);

typedef int (__cdecl *lua_error_t) (lua_State *L);

typedef int (__cdecl *lua_next_t) (lua_State *L, int idx);

typedef void (__cdecl *lua_concat_t) (lua_State *L, int n);
typedef void (__cdecl *lua_len_t) (lua_State *L, int idx);

#define LUA_N2SBUFFSZ	64
typedef unsigned (__cdecl *lua_numbertocstring_t) (lua_State *L, int idx, char *buff);
typedef size_t (__cdecl *lua_stringtonumber_t) (lua_State *L, const char *s);

typedef lua_Alloc (__cdecl *lua_getallocf_t) (lua_State *L, void **ud);
typedef void (__cdecl *lua_setallocf_t) (lua_State *L, lua_Alloc f, void *ud);

typedef void (__cdecl *lua_toclose_t) (lua_State *L, int idx);
typedef void (__cdecl *lua_closeslot_t) (lua_State *L, int idx);

#define lua_getextraspace(L)	((void *)((char *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),NULL)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),NULL)

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#define lua_pushglobaltable(L)  \
	((void)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)

#define lua_insert(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_pop(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_pop(L, 1))

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,NULL)

#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILCALL 4

#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef int (__cdecl *lua_getstack_t) (lua_State *L, int level, lua_Debug *ar);
typedef int (__cdecl *lua_getinfo_t) (lua_State *L, const char *what, lua_Debug *ar);
typedef const char * (__cdecl *lua_getlocal_t) (lua_State *L, const lua_Debug *ar, int n);
typedef const char * (__cdecl *lua_setlocal_t) (lua_State *L, const lua_Debug *ar, int n);
typedef const char * (__cdecl *lua_getupvalue_t) (lua_State *L, int funcindex, int n);
typedef const char * (__cdecl *lua_setupvalue_t) (lua_State *L, int funcindex, int n);

typedef void * (__cdecl *lua_upvalueid_t) (lua_State *L, int fidx, int n);
typedef void (__cdecl *lua_upvaluejoin_t) (lua_State *L, int fidx1, int n1,
                                               int fidx2, int n2);

typedef void (__cdecl *lua_sethook_t) (lua_State *L, lua_Hook func, int mask, int count);
typedef lua_Hook (__cdecl *lua_gethook_t) (lua_State *L);
typedef int (__cdecl *lua_gethookmask_t) (lua_State *L);
typedef int (__cdecl *lua_gethookcount_t) (lua_State *L);

struct lua_Debug {
  int event;
  const char *name;	
  const char *namewhat;	
  const char *what;	
  const char *source;	
  size_t srclen;	
  int currentline;	
  int linedefined;	
  int lastlinedefined;	
  unsigned char nups;	
  unsigned char nparams;
  char isvararg;        
  unsigned char extraargs;  
  char istailcall;	
  int ftransfer;   
  int ntransfer;   
  char short_src[LUA_IDSIZE]; 
  
  struct CallInfo *i_ci;  
};

#define LUAI_TOSTRAUX(x)	#x
#define LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#define LUA_VERSION_MAJOR	LUAI_TOSTR(LUA_VERSION_MAJOR_N)
#define LUA_VERSION_MINOR	LUAI_TOSTR(LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE	LUAI_TOSTR(LUA_VERSION_RELEASE_N)

#define LUA_VERSION	"Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#define LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE




#include <stddef.h>
#include <stdio.h>

#define LUA_GNAME	"_G"

typedef struct luaL_Buffer luaL_Buffer;

#define LUA_ERRFILE     (LUA_ERRERR+1)

#define LUA_LOADED_TABLE	"_LOADED"

#define LUA_PRELOAD_TABLE	"_PRELOAD"

typedef struct luaL_Reg {
  const char *name;
  lua_CFunction func;
} luaL_Reg;

#define LUAL_NUMSIZES	(sizeof(lua_Integer)*16 + sizeof(lua_Number))

typedef void (__cdecl *luaL_checkversion__t) (lua_State *L, lua_Number ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_NUM, LUAL_NUMSIZES)

typedef int (__cdecl *luaL_getmetafield_t) (lua_State *L, int obj, const char *e);
typedef int (__cdecl *luaL_callmeta_t) (lua_State *L, int obj, const char *e);
typedef const char * (__cdecl *luaL_tolstring_t) (lua_State *L, int idx, size_t *len);
typedef int (__cdecl *luaL_argerror_t) (lua_State *L, int arg, const char *extramsg);
typedef int (__cdecl *luaL_typeerror_t) (lua_State *L, int arg, const char *tname);
typedef const char * (__cdecl *luaL_checklstring_t) (lua_State *L, int arg,
                                                          size_t *l);
typedef const char * (__cdecl *luaL_optlstring_t) (lua_State *L, int arg,
                                          const char *def, size_t *l);
typedef lua_Number (__cdecl *luaL_checknumber_t) (lua_State *L, int arg);
typedef lua_Number (__cdecl *luaL_optnumber_t) (lua_State *L, int arg, lua_Number def);

typedef lua_Integer (__cdecl *luaL_checkinteger_t) (lua_State *L, int arg);
typedef lua_Integer (__cdecl *luaL_optinteger_t) (lua_State *L, int arg,
                                          lua_Integer def);

typedef void (__cdecl *luaL_checkstack_t) (lua_State *L, int sz, const char *msg);
typedef void (__cdecl *luaL_checktype_t) (lua_State *L, int arg, int t);
typedef void (__cdecl *luaL_checkany_t) (lua_State *L, int arg);

typedef int (__cdecl *luaL_newmetatable_t) (lua_State *L, const char *tname);
typedef void (__cdecl *luaL_setmetatable_t) (lua_State *L, const char *tname);
typedef void * (__cdecl *luaL_testudata_t) (lua_State *L, int ud, const char *tname);
typedef void * (__cdecl *luaL_checkudata_t) (lua_State *L, int ud, const char *tname);

typedef void (__cdecl *luaL_where_t) (lua_State *L, int lvl);
typedef int (__cdecl *luaL_error_t) (lua_State *L, const char *fmt, ...);

typedef int (__cdecl *luaL_checkoption_t) (lua_State *L, int arg, const char *def,
                                   const char *const lst[]);

typedef int (__cdecl *luaL_fileresult_t) (lua_State *L, int stat, const char *fname);
typedef int (__cdecl *luaL_execresult_t) (lua_State *L, int stat);

typedef void * (__cdecl *luaL_alloc_t) (void *ud, void *ptr, size_t osize,
                                                  size_t nsize);

#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

typedef int (__cdecl *luaL_ref_t) (lua_State *L, int t);
typedef void (__cdecl *luaL_unref_t) (lua_State *L, int t, int ref);

typedef int (__cdecl *luaL_loadfilex_t) (lua_State *L, const char *filename,
                                               const char *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,NULL)

typedef int (__cdecl *luaL_loadbufferx_t) (lua_State *L, const char *buff, size_t sz,
                                   const char *name, const char *mode);
typedef int (__cdecl *luaL_loadstring_t) (lua_State *L, const char *s);

typedef lua_State * (__cdecl *luaL_newstate_t) (void);

typedef unsigned (__cdecl *luaL_makeseed_t) (lua_State *L);

typedef lua_Integer (__cdecl *luaL_len_t) (lua_State *L, int idx);

typedef void (__cdecl *luaL_addgsub_t) (luaL_Buffer *b, const char *s,
                                     const char *p, const char *r);
typedef const char * (__cdecl *luaL_gsub_t) (lua_State *L, const char *s,
                                    const char *p, const char *r);

typedef void (__cdecl *luaL_setfuncs_t) (lua_State *L, const luaL_Reg *l, int nup);

typedef int (__cdecl *luaL_getsubtable_t) (lua_State *L, int idx, const char *fname);

typedef void (__cdecl *luaL_traceback_t) (lua_State *L, lua_State *L1,
                                  const char *msg, int level);

typedef void (__cdecl *luaL_requiref_t) (lua_State *L, const char *modname,
                                 lua_CFunction openf, int glb);

#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((void)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((void)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,NULL)

#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))

struct luaL_Buffer {
  char *b;  
  size_t size;  
  size_t n;  
  lua_State *L;
  union {
    LUAI_MAXALIGN;  
    char b[LUAL_BUFFERSIZE];  
  } init;
};

#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)

#define luaL_addchar(B,c) \
  ((void)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

typedef void (__cdecl *luaL_buffinit_t) (lua_State *L, luaL_Buffer *B);
typedef char * (__cdecl *luaL_prepbuffsize_t) (luaL_Buffer *B, size_t sz);
typedef void (__cdecl *luaL_addlstring_t) (luaL_Buffer *B, const char *s, size_t l);
typedef void (__cdecl *luaL_addstring_t) (luaL_Buffer *B, const char *s);
typedef void (__cdecl *luaL_addvalue_t) (luaL_Buffer *B);
typedef void (__cdecl *luaL_pushresult_t) (luaL_Buffer *B);
typedef void (__cdecl *luaL_pushresultsize_t) (luaL_Buffer *B, size_t sz);
typedef char * (__cdecl *luaL_buffinitsize_t) (lua_State *L, luaL_Buffer *B, size_t sz);

#define luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

#define LUA_FILEHANDLE          "FILE*"

typedef struct luaL_Stream {
  FILE *f;  
  lua_CFunction closef;  
} luaL_Stream;




#define LUA_VERSUFFIX          "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

#define LUA_GLIBK		1
typedef int (__cdecl *luaopen_base_t) (lua_State *L);

#define LUA_LOADLIBNAME	"package"
#define LUA_LOADLIBK	(LUA_GLIBK << 1)
typedef int (__cdecl *luaopen_package_t) (lua_State *L);

#define LUA_COLIBNAME	"coroutine"
#define LUA_COLIBK	(LUA_LOADLIBK << 1)
typedef int (__cdecl *luaopen_coroutine_t) (lua_State *L);

#define LUA_DBLIBNAME	"debug"
#define LUA_DBLIBK	(LUA_COLIBK << 1)
typedef int (__cdecl *luaopen_debug_t) (lua_State *L);

#define LUA_IOLIBNAME	"io"
#define LUA_IOLIBK	(LUA_DBLIBK << 1)
typedef int (__cdecl *luaopen_io_t) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
#define LUA_MATHLIBK	(LUA_IOLIBK << 1)
typedef int (__cdecl *luaopen_math_t) (lua_State *L);

#define LUA_OSLIBNAME	"os"
#define LUA_OSLIBK	(LUA_MATHLIBK << 1)
typedef int (__cdecl *luaopen_os_t) (lua_State *L);

#define LUA_STRLIBNAME	"string"
#define LUA_STRLIBK	(LUA_OSLIBK << 1)
typedef int (__cdecl *luaopen_string_t) (lua_State *L);

#define LUA_TABLIBNAME	"table"
#define LUA_TABLIBK	(LUA_STRLIBK << 1)
typedef int (__cdecl *luaopen_table_t) (lua_State *L);

#define LUA_UTF8LIBNAME	"utf8"
#define LUA_UTF8LIBK	(LUA_TABLIBK << 1)
typedef int (__cdecl *luaopen_utf8_t) (lua_State *L);

typedef void (__cdecl *luaL_openselectedlibs_t) (lua_State *L, int load, int preload);

#define luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)




#pragma once

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
typedef int (__cdecl *lua_registerobject_t) (lua_State *L, luart_type *type, const char *_typename, lua_CFunction constructor, const luaL_Reg *methods, const luaL_Reg *mt);

//--- Object registration macros
#define lua_regobjectmt(L, _typename) lua_registerobject(L, &T##_typename, #_typename, _typename##_constructor, _typename##_methods, _typename##_metafields)
#define lua_regobject(L, _typename) lua_registerobject(L, &T##_typename, #_typename, _typename##_constructor, _typename##_methods, NULL)

//--------------------------------------------------| Lua object manipulation

//--- Push a new instance from a Lua object at index idx on stack
typedef int (__cdecl *lua_createinstance_t) (lua_State *L, int idx);

//--- Push a new Lua object instance of a 'objectname' object
typedef void (__cdecl *lua_checkinstance_t) (lua_State *L, int idx, const char *objectname);

//--- Check if value at index idx is a Lua instance, setting its name to objectname
typedef int (__cdecl *lua_isinstance_t) (lua_State *L, int idx, const char **objectname);

//--- Check if value at index idx is a Lua object
typedef int (__cdecl *lua_isobject_t) (lua_State *L, int idx);

//--------------------------------------------------| C object manipulation

//--- Initialize a new C object instance, to be called only in the constructor
typedef void (__cdecl *lua_createcinstance_t) (lua_State *L, void *t, luart_type type);
#define lua_newinstance(L, t, _type) lua_createcinstance(L, t, T##_type)

//--- Returns any instance at index position or error with type t expected
typedef void * (__cdecl *lua_toself_t) (lua_State *L, int idx, luart_type t);

//--- Returns object at specified index, and gets its type
typedef void * (__cdecl *lua_tocinstance_t) (lua_State *L, int idx, luart_type *t);

//--- Returns instance of specified type at index or NULL
typedef void * (__cdecl *lua_iscinstance_t) (lua_State *L, int idx, luart_type t);

//--- Returns object of specified type at index or throws an error
typedef void * (__cdecl *lua_checkcinstance_t) (lua_State *L, int idx, luart_type t);
#define luaL_checkcinstance(L, idx, type) ((type *)lua_checkcinstance(L, idx, T##type))

//--- Gets object at specified index without checking its type
#define lua_self(L, idx, type) ((type*)lua_toself(L, idx, T##type))

//--- Pushes a new instance from object name with the lasts narg arguments on stack as constructor arguments
typedef void * (__cdecl *lua_pushnewinstancebyname_t) (lua_State *L, luart_type type, int narg);
typedef void * (__cdecl *lua_pushnewinstance_t) (lua_State *L, const char *_typename, int narg);
#define lua_pushinstance(L, type, narg) lua_pushnewinstance(L, #type, narg)

//--- Gets value type name at the specified index (supporting objects type names)
typedef const char * (__cdecl *lua_objectname_t) (lua_State *L, int idx);

//--- Uses lua_objectname for luaL_Typename implementation
#undef luaL_typename
#define luaL_typename(L, i) lua_objectname(L, i)

//--- Returns the inherited Object
typedef int (__cdecl *lua_super_t) (lua_State *L, int idx);

//--------------------------------------------------| Module registration

//--- Registers module and pushes it on the stack
typedef void (__cdecl *lua_registermodule_t) (lua_State *L, const char *name, const luaL_Reg *functions, const luaL_Reg *properties, lua_CFunction finalizer);

//--- Module registration macros with or without a finalizer (called upon module garbage collection)
#define lua_regmodule(L, m) lua_registermodule(L, #m, m##lib, m##_properties, NULL)
#define lua_regmodulefinalize(L, m) lua_registermodule(L, #m, m##lib, m##_properties, m##_finalize)

//--------------------------------------------------| Wide string functions

//--- Checks for a string at the specified index and get it as a wide string
typedef wchar_t * (__cdecl *lua_tolwstring_t) (lua_State *L, int idx, int *len);

//--- Pushes a wide string onto the stack
typedef void (__cdecl *lua_pushlwstring_t) (lua_State *L, const wchar_t *str, int len);

//--- Wide strings macros
#define lua_towstring(L, i)		lua_tolwstring(L, i, NULL)
#define lua_pushwstring(L, s)	lua_pushlwstring(L, s, -1)

//--------------------------------------------------| Preprocessor functions

//--- C-compatible wrapper around luaL_loadfilex with preprocessing
typedef int (__cdecl *luaL_loadfilep_t) (lua_State* L, const char* filename, const char* mode);

//--------------------------------------------------| Various utility functions

//--- Returns index of string at specfied index, in a an array of options strings, or the default specified index
typedef int (__cdecl *lua_optstring_t) (lua_State *L, int idx, const char *options[], int def);

//-- Close Lua state and call atexit function if exists
typedef void (__cdecl *lua_stop_t) (void);

//--------------------------------------------------| Open modules function
typedef void (__cdecl *lua_openmodules_t) (lua_State *L);

//--- Returns an array of bytes if the specified executable has embedded content or NULL otherwise, and loads global "embed" module
typedef BYTE * (__cdecl *luaL_embedopen_t) (lua_State *L);

//--- Closes embedded content previously opened with luaL_embedopen()
typedef int (__cdecl *luaL_embedclose_t) (lua_State *L);

//--- luaL_setfuncs() alternative with lua_rawset() and without upvalues
typedef void (__cdecl *luaL_setrawfuncs_t) (lua_State *L, const luaL_Reg *l);

//--- luaL_require() alternative to luaL_requiref()
typedef void (__cdecl *luaL_require_t) (lua_State *L, const char *modname);

typedef struct Task Task;

//--- Push a task with the provided continuation C function and starts it, with a context and optional cleanup lua_CFunction
//--- Always returns 1
typedef Task * (__cdecl *lua_pushtask_t) (lua_State *L, lua_KFunction taskfunc, void *userdata, lua_CFunction gc);

//--- Sleeps the current task or the current Lua state for the provided delay
typedef void (__cdecl *lua_sleep_t) (lua_State *L, lua_Integer delay);

//--- Throws a new event from the current object at the top of the stack, followed by its parameters
typedef int (__cdecl *lua_throwevent_t) (lua_State *L, const char *name, int nparams);

//--- Wait for the Task at index idx to terminate
typedef int (__cdecl *lua_wait_t) (lua_State *L, Task *t);

//--- Get the number of current active Tasks
typedef int (__cdecl *lua_taskcount_t) ();

//--- Get the current executing Task
typedef Task * (__cdecl *lua_gettask_t) (lua_State *L);

//--- Schedule all current tasks and return the number of results is a Task is terminated
typedef int (__cdecl *lua_schedule_t) (lua_State *L);

//--------------------------------------------------| LuaRT utility functions

//--- Pushes Windows system error string on stack
typedef int (__cdecl *luaL_getlasterror_t) (lua_State *L, DWORD err);

//--- Get a filename at specified index, from a string or from a File object
//--- result must be manually freed once used
typedef wchar_t * (__cdecl *luaL_checkFilename_t) (lua_State *L, int idx);

//--- Get a filename at specified index, from a string or from a Directory object
//--- result must be manually freed once used
typedef wchar_t * (__cdecl *luaL_checkDirname_t) (lua_State *L, int idx);

//--------------- Buffer functions

//--- Push a Buffer on the stack given a pointer and the size of the buffer
typedef void (__cdecl *lua_pushBuffer_t) (lua_State *L, void *p, size_t len);

//--- Converts the Object at the specified stack index to a Buffer pointer
typedef struct Buffer Buffer;

typedef Buffer * (__cdecl *lua_toBuffer_t) (lua_State *L, int idx);

//--------------- zip decompression functions
struct zip_t;
#ifdef _WIN64
typedef long long ssize_t; 
#else
typedef long ssize_t; 
#endif
typedef ssize_t (__cdecl *zip_entry_read_t) (struct zip_t *zip, void **buf, size_t *bufsize);
typedef int (__cdecl *zip_entry_open_t) (struct zip_t *zip, const char *entryname);
typedef int (__cdecl *zip_entry_close_t) (struct zip_t *zip);
typedef int (__cdecl *zip_entry_fread_t) (struct zip_t *zip, const char *filename);

LUA_API luart_type TTask;
LUA_API luart_type TFile;
LUA_API luart_type TBuffer;
LUA_API luart_type TCOM;
LUA_API luart_type TDatetime;
LUA_API luart_type TPipe;
LUA_API luart_type TZip;
LUA_API luart_type TDirectory;

//-------------------- Lua Clipboard format
typedef HGLOBAL (__cdecl *table_to_HDROPFormat_t) (lua_State *L, int idx);


#define lua_absindex            LUA_PREFIX.Absindex
#define lua_arith               LUA_PREFIX.Arith
#define lua_atpanic             LUA_PREFIX.Atpanic
#define lua_callk               LUA_PREFIX.Callk
#define lua_checkcinstance      LUA_PREFIX.Checkcinstance
#define lua_checkinstance       LUA_PREFIX.Checkinstance
#define lua_checkstack          LUA_PREFIX.Checkstack
#define lua_close               LUA_PREFIX.Close
#define lua_closeslot           LUA_PREFIX.Closeslot
#define lua_closethread         LUA_PREFIX.Closethread
#define lua_compare             LUA_PREFIX.Compare
#define lua_concat              LUA_PREFIX.Concat
#define lua_copy                LUA_PREFIX.Copy
#define lua_createcinstance     LUA_PREFIX.Createcinstance
#define lua_createinstance      LUA_PREFIX.Createinstance
#define lua_createtable         LUA_PREFIX.Createtable
#define lua_dump                LUA_PREFIX.Dump
#define lua_error               LUA_PREFIX.Error
#define lua_gc                  LUA_PREFIX.Gc
#define lua_getallocf           LUA_PREFIX.Getallocf
#define lua_getfield            LUA_PREFIX.Getfield
#define lua_getglobal           LUA_PREFIX.Getglobal
#define lua_gethook             LUA_PREFIX.Gethook
#define lua_gethookcount        LUA_PREFIX.Gethookcount
#define lua_gethookmask         LUA_PREFIX.Gethookmask
#define lua_geti                LUA_PREFIX.Geti
#define lua_getinfo             LUA_PREFIX.Getinfo
#define lua_getiuservalue       LUA_PREFIX.Getiuservalue
#define lua_getlocal            LUA_PREFIX.Getlocal
#define lua_getmetatable        LUA_PREFIX.Getmetatable
#define lua_getstack            LUA_PREFIX.Getstack
#define lua_gettable            LUA_PREFIX.Gettable
#define lua_gettask             LUA_PREFIX.Gettask
#define lua_gettop              LUA_PREFIX.Gettop
#define lua_getupvalue          LUA_PREFIX.Getupvalue
#define lua_iscfunction         LUA_PREFIX.Iscfunction
#define lua_iscinstance         LUA_PREFIX.Iscinstance
#define lua_isinstance          LUA_PREFIX.Isinstance
#define lua_isinteger           LUA_PREFIX.Isinteger
#define lua_isnumber            LUA_PREFIX.Isnumber
#define lua_isobject            LUA_PREFIX.Isobject
#define lua_isstring            LUA_PREFIX.Isstring
#define lua_isuserdata          LUA_PREFIX.Isuserdata
#define lua_isyieldable         LUA_PREFIX.Isyieldable
#define lua_len                 LUA_PREFIX.Len
#define lua_load                LUA_PREFIX.Load
#define lua_newstate            LUA_PREFIX.Newstate
#define lua_newthread           LUA_PREFIX.Newthread
#define lua_newuserdatauv       LUA_PREFIX.Newuserdatauv
#define lua_next                LUA_PREFIX.Next
#define lua_numbertocstring     LUA_PREFIX.Numbertocstring
#define lua_objectname          LUA_PREFIX.Objectname
#define lua_openmodules         LUA_PREFIX.Openmodules
#define lua_optstring           LUA_PREFIX.Optstring
#define lua_pcallk              LUA_PREFIX.Pcallk
#define lua_pushboolean         LUA_PREFIX.Pushboolean
#define lua_pushBuffer          LUA_PREFIX.PushBuffer
#define lua_pushcclosure        LUA_PREFIX.Pushcclosure
#define lua_pushexternalstring  LUA_PREFIX.Pushexternalstring
#define lua_pushfstring         LUA_PREFIX.Pushfstring
#define lua_pushinteger         LUA_PREFIX.Pushinteger
#define lua_pushlightuserdata   LUA_PREFIX.Pushlightuserdata
#define lua_pushlstring         LUA_PREFIX.Pushlstring
#define lua_pushlwstring        LUA_PREFIX.Pushlwstring
#define lua_pushnewinstance     LUA_PREFIX.Pushnewinstance
#define lua_pushnewinstancebyname LUA_PREFIX.Pushnewinstancebyname
#define lua_pushnil             LUA_PREFIX.Pushnil
#define lua_pushnumber          LUA_PREFIX.Pushnumber
#define lua_pushstring          LUA_PREFIX.Pushstring
#define lua_pushtask            LUA_PREFIX.Pushtask
#define lua_pushthread          LUA_PREFIX.Pushthread
#define lua_pushvalue           LUA_PREFIX.Pushvalue
#define lua_pushvfstring        LUA_PREFIX.Pushvfstring
#define lua_rawequal            LUA_PREFIX.Rawequal
#define lua_rawget              LUA_PREFIX.Rawget
#define lua_rawgeti             LUA_PREFIX.Rawgeti
#define lua_rawgetp             LUA_PREFIX.Rawgetp
#define lua_rawlen              LUA_PREFIX.Rawlen
#define lua_rawset              LUA_PREFIX.Rawset
#define lua_rawseti             LUA_PREFIX.Rawseti
#define lua_rawsetp             LUA_PREFIX.Rawsetp
#define lua_registermodule      LUA_PREFIX.Registermodule
#define lua_registerobject      LUA_PREFIX.Registerobject
#define lua_resume              LUA_PREFIX.Resume
#define lua_rotate              LUA_PREFIX.Rotate
#define lua_schedule            LUA_PREFIX.Schedule
#define lua_setallocf           LUA_PREFIX.Setallocf
#define lua_setfield            LUA_PREFIX.Setfield
#define lua_setglobal           LUA_PREFIX.Setglobal
#define lua_sethook             LUA_PREFIX.Sethook
#define lua_seti                LUA_PREFIX.Seti
#define lua_setiuservalue       LUA_PREFIX.Setiuservalue
#define lua_setlocal            LUA_PREFIX.Setlocal
#define lua_setmetatable        LUA_PREFIX.Setmetatable
#define lua_settable            LUA_PREFIX.Settable
#define lua_settop              LUA_PREFIX.Settop
#define lua_setupvalue          LUA_PREFIX.Setupvalue
#define lua_setwarnf            LUA_PREFIX.Setwarnf
#define lua_sleep               LUA_PREFIX.Sleep
#define lua_status              LUA_PREFIX.Status
#define lua_stop                LUA_PREFIX.Stop
#define lua_stringtonumber      LUA_PREFIX.Stringtonumber
#define lua_super               LUA_PREFIX.Super
#define lua_taskcount           LUA_PREFIX.Taskcount
#define lua_throwevent          LUA_PREFIX.Throwevent
#define lua_toboolean           LUA_PREFIX.Toboolean
#define lua_toBuffer            LUA_PREFIX.ToBuffer
#define lua_tocfunction         LUA_PREFIX.Tocfunction
#define lua_tocinstance         LUA_PREFIX.Tocinstance
#define lua_toclose             LUA_PREFIX.Toclose
#define lua_tointegerx          LUA_PREFIX.Tointegerx
#define lua_tolstring           LUA_PREFIX.Tolstring
#define lua_tolwstring          LUA_PREFIX.Tolwstring
#define lua_tonumberx           LUA_PREFIX.Tonumberx
#define lua_topointer           LUA_PREFIX.Topointer
#define lua_toself              LUA_PREFIX.Toself
#define lua_tothread            LUA_PREFIX.Tothread
#define lua_touserdata          LUA_PREFIX.Touserdata
#define lua_type                LUA_PREFIX.Type
#define lua_typename            LUA_PREFIX.Typename
#define lua_upvalueid           LUA_PREFIX.Upvalueid
#define lua_upvaluejoin         LUA_PREFIX.Upvaluejoin
#define lua_version             LUA_PREFIX.Version
#define lua_wait                LUA_PREFIX.Wait
#define lua_warning             LUA_PREFIX.Warning
#define lua_xmove               LUA_PREFIX.Xmove
#define lua_yieldk              LUA_PREFIX.Yieldk
#define luaL_addgsub            LUA_PREFIX.AddgsubL
#define luaL_addlstring         LUA_PREFIX.AddlstringL
#define luaL_addstring          LUA_PREFIX.AddstringL
#define luaL_addvalue           LUA_PREFIX.AddvalueL
#define luaL_alloc              LUA_PREFIX.AllocL
#define luaL_argerror           LUA_PREFIX.ArgerrorL
#define luaL_buffinit           LUA_PREFIX.BuffinitL
#define luaL_buffinitsize       LUA_PREFIX.BuffinitsizeL
#define luaL_callmeta           LUA_PREFIX.CallmetaL
#define luaL_checkany           LUA_PREFIX.CheckanyL
#define luaL_checkDirname       LUA_PREFIX.CheckDirnameL
#define luaL_checkFilename      LUA_PREFIX.CheckFilenameL
#define luaL_checkinteger       LUA_PREFIX.CheckintegerL
#define luaL_checklstring       LUA_PREFIX.ChecklstringL
#define luaL_checknumber        LUA_PREFIX.ChecknumberL
#define luaL_checkoption        LUA_PREFIX.CheckoptionL
#define luaL_checkstack         LUA_PREFIX.CheckstackL
#define luaL_checktype          LUA_PREFIX.ChecktypeL
#define luaL_checkudata         LUA_PREFIX.CheckudataL
#define luaL_checkversion_      LUA_PREFIX.Checkversion_L
#define luaL_embedclose         LUA_PREFIX.EmbedcloseL
#define luaL_embedopen          LUA_PREFIX.EmbedopenL
#define luaL_error              LUA_PREFIX.ErrorL
#define luaL_execresult         LUA_PREFIX.ExecresultL
#define luaL_fileresult         LUA_PREFIX.FileresultL
#define luaL_getlasterror       LUA_PREFIX.GetlasterrorL
#define luaL_getmetafield       LUA_PREFIX.GetmetafieldL
#define luaL_getsubtable        LUA_PREFIX.GetsubtableL
#define luaL_gsub               LUA_PREFIX.GsubL
#define luaL_len                LUA_PREFIX.LenL
#define luaL_loadbufferx        LUA_PREFIX.LoadbufferxL
#define luaL_loadfilep          LUA_PREFIX.LoadfilepL
#define luaL_loadfilex          LUA_PREFIX.LoadfilexL
#define luaL_loadstring         LUA_PREFIX.LoadstringL
#define luaL_makeseed           LUA_PREFIX.MakeseedL
#define luaL_newmetatable       LUA_PREFIX.NewmetatableL
#define luaL_newstate           LUA_PREFIX.NewstateL
#define luaL_openselectedlibs   LUA_PREFIX.OpenselectedlibsL
#define luaL_optinteger         LUA_PREFIX.OptintegerL
#define luaL_optlstring         LUA_PREFIX.OptlstringL
#define luaL_optnumber          LUA_PREFIX.OptnumberL
#define luaL_prepbuffsize       LUA_PREFIX.PrepbuffsizeL
#define luaL_pushresult         LUA_PREFIX.PushresultL
#define luaL_pushresultsize     LUA_PREFIX.PushresultsizeL
#define luaL_ref                LUA_PREFIX.RefL
#define luaL_require            LUA_PREFIX.RequireL
#define luaL_requiref           LUA_PREFIX.RequirefL
#define luaL_setfuncs           LUA_PREFIX.SetfuncsL
#define luaL_setmetatable       LUA_PREFIX.SetmetatableL
#define luaL_setrawfuncs        LUA_PREFIX.SetrawfuncsL
#define luaL_testudata          LUA_PREFIX.TestudataL
#define luaL_tolstring          LUA_PREFIX.TolstringL
#define luaL_traceback          LUA_PREFIX.TracebackL
#define luaL_typeerror          LUA_PREFIX.TypeerrorL
#define luaL_unref              LUA_PREFIX.UnrefL
#define luaL_where              LUA_PREFIX.WhereL
#define luaopen_base            LUA_PREFIX.Open_base
#define luaopen_coroutine       LUA_PREFIX.Open_coroutine
#define luaopen_debug           LUA_PREFIX.Open_debug
#define luaopen_io              LUA_PREFIX.Open_io
#define luaopen_math            LUA_PREFIX.Open_math
#define luaopen_os              LUA_PREFIX.Open_os
#define luaopen_package         LUA_PREFIX.Open_package
#define luaopen_string          LUA_PREFIX.Open_string
#define luaopen_table           LUA_PREFIX.Open_table
#define luaopen_utf8            LUA_PREFIX.Open_utf8
#define table_to_HDROPFormat    LUA_PREFIX.Le_to_HDROPFormat
#define zip_entry_close         LUA_PREFIX._entry_close
#define zip_entry_fread         LUA_PREFIX._entry_fread
#define zip_entry_open          LUA_PREFIX._entry_open
#define zip_entry_read          LUA_PREFIX._entry_read
#define TTask                   (*LUA_PREFIX.var_TTask)
#define TFile                   (*LUA_PREFIX.var_TFile)
#define TBuffer                 (*LUA_PREFIX.var_TBuffer)
#define TCOM                    (*LUA_PREFIX.var_TCOM)
#define TDatetime               (*LUA_PREFIX.var_TDatetime)
#define TPipe                   (*LUA_PREFIX.var_TPipe)
#define TZip                    (*LUA_PREFIX.var_TZip)
#define TDirectory              (*LUA_PREFIX.var_TDirectory)

typedef struct lua_All_functions
{
  lua_absindex_t          Absindex;
  lua_arith_t             Arith;
  lua_atpanic_t           Atpanic;
  lua_callk_t             Callk;
  lua_checkcinstance_t    Checkcinstance;
  lua_checkinstance_t     Checkinstance;
  lua_checkstack_t        Checkstack;
  lua_close_t             Close;
  lua_closeslot_t         Closeslot;
  lua_closethread_t       Closethread;
  lua_compare_t           Compare;
  lua_concat_t            Concat;
  lua_copy_t              Copy;
  lua_createcinstance_t   Createcinstance;
  lua_createinstance_t    Createinstance;
  lua_createtable_t       Createtable;
  lua_dump_t              Dump;
  lua_error_t             Error;
  lua_gc_t                Gc;
  lua_getallocf_t         Getallocf;
  lua_getfield_t          Getfield;
  lua_getglobal_t         Getglobal;
  lua_gethook_t           Gethook;
  lua_gethookcount_t      Gethookcount;
  lua_gethookmask_t       Gethookmask;
  lua_geti_t              Geti;
  lua_getinfo_t           Getinfo;
  lua_getiuservalue_t     Getiuservalue;
  lua_getlocal_t          Getlocal;
  lua_getmetatable_t      Getmetatable;
  lua_getstack_t          Getstack;
  lua_gettable_t          Gettable;
  lua_gettask_t           Gettask;
  lua_gettop_t            Gettop;
  lua_getupvalue_t        Getupvalue;
  lua_iscfunction_t       Iscfunction;
  lua_iscinstance_t       Iscinstance;
  lua_isinstance_t        Isinstance;
  lua_isinteger_t         Isinteger;
  lua_isnumber_t          Isnumber;
  lua_isobject_t          Isobject;
  lua_isstring_t          Isstring;
  lua_isuserdata_t        Isuserdata;
  lua_isyieldable_t       Isyieldable;
  lua_len_t               Len;
  lua_load_t              Load;
  lua_newstate_t          Newstate;
  lua_newthread_t         Newthread;
  lua_newuserdatauv_t     Newuserdatauv;
  lua_next_t              Next;
  lua_numbertocstring_t   Numbertocstring;
  lua_objectname_t        Objectname;
  lua_openmodules_t       Openmodules;
  lua_optstring_t         Optstring;
  lua_pcallk_t            Pcallk;
  lua_pushboolean_t       Pushboolean;
  lua_pushBuffer_t        PushBuffer;
  lua_pushcclosure_t      Pushcclosure;
  lua_pushexternalstring_t Pushexternalstring;
  lua_pushfstring_t       Pushfstring;
  lua_pushinteger_t       Pushinteger;
  lua_pushlightuserdata_t Pushlightuserdata;
  lua_pushlstring_t       Pushlstring;
  lua_pushlwstring_t      Pushlwstring;
  lua_pushnewinstance_t   Pushnewinstance;
  lua_pushnewinstancebyname_t Pushnewinstancebyname;
  lua_pushnil_t           Pushnil;
  lua_pushnumber_t        Pushnumber;
  lua_pushstring_t        Pushstring;
  lua_pushtask_t          Pushtask;
  lua_pushthread_t        Pushthread;
  lua_pushvalue_t         Pushvalue;
  lua_pushvfstring_t      Pushvfstring;
  lua_rawequal_t          Rawequal;
  lua_rawget_t            Rawget;
  lua_rawgeti_t           Rawgeti;
  lua_rawgetp_t           Rawgetp;
  lua_rawlen_t            Rawlen;
  lua_rawset_t            Rawset;
  lua_rawseti_t           Rawseti;
  lua_rawsetp_t           Rawsetp;
  lua_registermodule_t    Registermodule;
  lua_registerobject_t    Registerobject;
  lua_resume_t            Resume;
  lua_rotate_t            Rotate;
  lua_schedule_t          Schedule;
  lua_setallocf_t         Setallocf;
  lua_setfield_t          Setfield;
  lua_setglobal_t         Setglobal;
  lua_sethook_t           Sethook;
  lua_seti_t              Seti;
  lua_setiuservalue_t     Setiuservalue;
  lua_setlocal_t          Setlocal;
  lua_setmetatable_t      Setmetatable;
  lua_settable_t          Settable;
  lua_settop_t            Settop;
  lua_setupvalue_t        Setupvalue;
  lua_setwarnf_t          Setwarnf;
  lua_sleep_t             Sleep;
  lua_status_t            Status;
  lua_stop_t              Stop;
  lua_stringtonumber_t    Stringtonumber;
  lua_super_t             Super;
  lua_taskcount_t         Taskcount;
  lua_throwevent_t        Throwevent;
  lua_toboolean_t         Toboolean;
  lua_toBuffer_t          ToBuffer;
  lua_tocfunction_t       Tocfunction;
  lua_tocinstance_t       Tocinstance;
  lua_toclose_t           Toclose;
  lua_tointegerx_t        Tointegerx;
  lua_tolstring_t         Tolstring;
  lua_tolwstring_t        Tolwstring;
  lua_tonumberx_t         Tonumberx;
  lua_topointer_t         Topointer;
  lua_toself_t            Toself;
  lua_tothread_t          Tothread;
  lua_touserdata_t        Touserdata;
  lua_type_t              Type;
  lua_typename_t          Typename;
  lua_upvalueid_t         Upvalueid;
  lua_upvaluejoin_t       Upvaluejoin;
  lua_version_t           Version;
  lua_wait_t              Wait;
  lua_warning_t           Warning;
  lua_xmove_t             Xmove;
  lua_yieldk_t            Yieldk;
  luaL_addgsub_t          AddgsubL;
  luaL_addlstring_t       AddlstringL;
  luaL_addstring_t        AddstringL;
  luaL_addvalue_t         AddvalueL;
  luaL_alloc_t            AllocL;
  luaL_argerror_t         ArgerrorL;
  luaL_buffinit_t         BuffinitL;
  luaL_buffinitsize_t     BuffinitsizeL;
  luaL_callmeta_t         CallmetaL;
  luaL_checkany_t         CheckanyL;
  luaL_checkDirname_t     CheckDirnameL;
  luaL_checkFilename_t    CheckFilenameL;
  luaL_checkinteger_t     CheckintegerL;
  luaL_checklstring_t     ChecklstringL;
  luaL_checknumber_t      ChecknumberL;
  luaL_checkoption_t      CheckoptionL;
  luaL_checkstack_t       CheckstackL;
  luaL_checktype_t        ChecktypeL;
  luaL_checkudata_t       CheckudataL;
  luaL_checkversion__t    Checkversion_L;
  luaL_embedclose_t       EmbedcloseL;
  luaL_embedopen_t        EmbedopenL;
  luaL_error_t            ErrorL;
  luaL_execresult_t       ExecresultL;
  luaL_fileresult_t       FileresultL;
  luaL_getlasterror_t     GetlasterrorL;
  luaL_getmetafield_t     GetmetafieldL;
  luaL_getsubtable_t      GetsubtableL;
  luaL_gsub_t             GsubL;
  luaL_len_t              LenL;
  luaL_loadbufferx_t      LoadbufferxL;
  luaL_loadfilep_t        LoadfilepL;
  luaL_loadfilex_t        LoadfilexL;
  luaL_loadstring_t       LoadstringL;
  luaL_makeseed_t         MakeseedL;
  luaL_newmetatable_t     NewmetatableL;
  luaL_newstate_t         NewstateL;
  luaL_openselectedlibs_t OpenselectedlibsL;
  luaL_optinteger_t       OptintegerL;
  luaL_optlstring_t       OptlstringL;
  luaL_optnumber_t        OptnumberL;
  luaL_prepbuffsize_t     PrepbuffsizeL;
  luaL_pushresult_t       PushresultL;
  luaL_pushresultsize_t   PushresultsizeL;
  luaL_ref_t              RefL;
  luaL_require_t          RequireL;
  luaL_requiref_t         RequirefL;
  luaL_setfuncs_t         SetfuncsL;
  luaL_setmetatable_t     SetmetatableL;
  luaL_setrawfuncs_t      SetrawfuncsL;
  luaL_testudata_t        TestudataL;
  luaL_tolstring_t        TolstringL;
  luaL_traceback_t        TracebackL;
  luaL_typeerror_t        TypeerrorL;
  luaL_unref_t            UnrefL;
  luaL_where_t            WhereL;
  luaopen_base_t          Open_base;
  luaopen_coroutine_t     Open_coroutine;
  luaopen_debug_t         Open_debug;
  luaopen_io_t            Open_io;
  luaopen_math_t          Open_math;
  luaopen_os_t            Open_os;
  luaopen_package_t       Open_package;
  luaopen_string_t        Open_string;
  luaopen_table_t         Open_table;
  luaopen_utf8_t          Open_utf8;
  table_to_HDROPFormat_t  Le_to_HDROPFormat;
  zip_entry_close_t       _entry_close;
  zip_entry_fread_t       _entry_fread;
  zip_entry_open_t        _entry_open;
  zip_entry_read_t        _entry_read;
  luart_type              *var_TTask;
  luart_type              *var_TFile;
  luart_type              *var_TBuffer;
  luart_type              *var_TCOM;
  luart_type              *var_TDatetime;
  luart_type              *var_TPipe;
  luart_type              *var_TZip;
  luart_type              *var_TDirectory;
} lua_All_functions;

extern lua_All_functions staticlua;

#ifdef __cplusplus
}
#endif
#endif
