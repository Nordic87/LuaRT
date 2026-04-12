
/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Zip.c | LuaRT Zip object implementation
*/
#define LUA_LIB

#include "Zip.h"
#include <File.h>
#include <Directory.h>
#include <Buffer.h>
#include "lrtapi.h"
#include <luart.h>
#include <Task.h>

#include <stdio.h>
#include <wchar.h>
#include <shlwapi.h>

#include "lib\zip.h"
#define MINIZ_HEADER_FILE_ONLY
#include "lib\miniz.h"

luart_type TZip;

const char *zip_modes[] = { "read", "append", "write", "delete", NULL };
typedef struct {
	struct zip_t *zip;
	wchar_t       tmp[MAX_PATH];
	HANDLE        hMap;
	HANDLE        hFile;
	uint8_t       *map;
} ZipFs;
ZipFs fs = {0};

char *checkEntry(lua_State *L, int idx, luart_type t) {
	char *str = NULL;
	switch(lua_type(L, idx)) {
		case LUA_TTABLE:	File *f = lua_checkcinstance(L, idx, t);
							int len = -1;
							str = wchar_toutf8(f->fullpath, &len);
							break;

		case LUA_TSTRING:	str = strdup(lua_tostring(L, idx));
							break;
		default:			luaL_argerror(L, idx, "string or File");
	}
	return str;
}

static char *normalize(const char *str) {
	char *result = strdup(str);
	char *ptr = result;
	while (*ptr) {
        if (*ptr == '\\')
            *ptr = '/';
        ptr++;
    }
	return result;
}

/* ------------------------------------------------------------------------ */

LUA_CONSTRUCTOR(Zip) {
	Zip *z;
	struct zip_t *zip;
	int level, idx;
	char *fname;
	char mode;
	
	if (lua_islightuserdata(L, 2)) {
		z = calloc(1, sizeof(Zip));	
		z->zip = lua_touserdata(L, 2);
		z->mode = 'r';
		goto done;
	} else {
		level = luaL_optinteger(L, 4, MZ_DEFAULT_COMPRESSION);
		idx = luaL_checkoption(L, 3, "read", zip_modes);
		fname = checkFilename(L, 2);
		mode = *zip_modes[idx];
		if ( (zip = zip_open(fname, level, mode))) {
			z = calloc(1, sizeof(Zip));	
			z->zip = zip;
			z->mode = mode;
			z->fname = fname;
done:		lua_newinstance(L, z, Zip);
		} else
			luaL_error(L, zip_lasterror(zip));
	}
	return 1;
}

LUA_METHOD(Zip, close) {
	Zip *z = lua_self(L, 1, Zip);
	if (z->fname)
		zip_close(z->zip);
	z->zip = NULL;
	return 0;
}

LUA_METHOD(Zip, __gc) {
	Zip *z = lua_self(L, 1, Zip);
	zip_close(z->zip);
	free(z->fname);
	free(z);
	return 0;
}

LUA_PROPERTY_GET(Zip, count) {
	lua_pushinteger(L, zip_entries_total(lua_self(L, 1, Zip)->zip));
	return 1;
}

LUA_PROPERTY_GET(Zip, size) {
	lua_pushnumber(L, zip_getsize(lua_self(L, 1, Zip)->zip));
	return 1;
}

LUA_PROPERTY_GET(Zip, error){
	lua_pushstring(L, zip_lasterror(lua_self(L, 1, Zip)->zip));
	return 1;
}

LUA_PROPERTY_GET(Zip, iszip64) {
	lua_pushboolean(L, zip_is64(lua_self(L, 1, Zip)->zip));
	return 1;
}

LUA_METHOD(Zip, reopen) {
	Zip *z = lua_self(L, 1, Zip);
	char mode;
	
	if (fs.zip && (z->zip == fs.zip))
		luaL_error(L, "cannot reopen bundled Zip archive");
	mode = *zip_modes[luaL_checkoption(L, 2, "read", zip_modes)];
	zip_close(z->zip);
	z->level = luaL_optinteger(L, 3, MZ_DEFAULT_COMPRESSION);
	if ( (z->zip = zip_open(z->fname, z->level, mode)))
		z->mode = mode;
	else luaL_error(L, strerror(errno));
	return 0;
}

#define max(a,b) (((a) > (b)) ? (a) : (b))

char *remove_trailing_sep(char *str) {
	size_t len = strlen(str);
	while (str[--len] == '/' || str[len] == '\\')
		str[len] = '\0';
	return str;
}

static char *append_path(const char *str, const char* path) {
	size_t len = strlen(str) + strlen(path)+2;
	char *result = calloc(len, 1);
	_snprintf(result, len, "%s/%s", str, path);
	return result;
}

static void make_dir_path(Zip *z, char *dir) {
	char *start = dir;
	char ch;
	while((ch = *dir))
	{
		dir++;
		if (ch == '\\' || ch == '/') {
			ch = *dir;
			*dir = 0;
			if (zip_locatefile(z->zip, start) < 0) {
				zip_entry_open(z->zip, start);
				zip_entry_close(z->zip);
			}
			*dir = ch;
		}
	}
}

static BOOL write_dir(Zip *z, const char *_dir, BOOL isfullpath, const char* dest) {
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	char *path;
	char *dir = strdup(_dir);
	BOOL result = TRUE;

	if (dest) {
		char *trailing = append_path(dest, "");
		size_t s = strlen(trailing);
		make_dir_path(z, trailing);
        free(trailing);
	}
	path = append_path(remove_trailing_sep(dir), "*");
	if ((hFind = FindFirstFile(path, &FindFileData)) != INVALID_HANDLE_VALUE) {
	    do {
			if ( !(FindFileData.cFileName[0] == '.' && (FindFileData.cFileName[1] == '.' || FindFileData.cFileName[1] == 0)) ) {
				char *newpath = append_path(dir, FindFileData.cFileName);
				char *newdest = dest ? append_path(dest, FindFileData.cFileName) : NULL;

				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					result &= write_dir(z, newpath, isfullpath, newdest ? newdest : FindFileData.cFileName);
				else {
					if ((FindFileData.nFileSizeHigh && FindFileData.nFileSizeLow) || (strcmp(FindFileData.cFileName, z->fname) != 0)) {
						result &= !zip_entry_open(z->zip, newdest ? newdest : FindFileData.cFileName) && !zip_entry_fwrite(z->zip, newpath);
	        			zip_entry_close(z->zip);
					}
				}
				free(newpath);
				free(newdest);				
			}
	  	} while (FindNextFile(hFind, &FindFileData));
	    FindClose(hFind);
	}
	free(path);
	free(dir);
	return result;
}

LUA_METHOD(Zip, write) {
	Zip *z = lua_self(L, 1, Zip);
	int is_entry = lua_gettop(L) == 3;
	BOOL result = FALSE;
	DWORD attrib = 0;
	const char *dest = is_entry ? luaL_checkstring(L, 3) : NULL;
	const char *dest_entry = is_entry ? luaL_checkstring(L, 3) : NULL;
	const char *fname = NULL;
	char *entry = NULL;
	
	if (lua_isstring(L, 2)) {
		fname = luaL_checkstring(L, 2);		
		attrib = GetFileAttributes(fname);
		if (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY)) {
			result = write_dir(z, fname, !PathIsRelative(fname), dest);
			goto done;
		}
		else
			result = (zip_entry_open(z->zip, dest_entry ? dest_entry: PathFindFileNameA(fname)) == 0) && (attrib != INVALID_FILE_ATTRIBUTES && ((zip_entry_fwrite(z->zip, fname) == 0) || luaL_error(L, strerror(errno))));	
	} else {
		luart_type t;
		void *obj;
		
		if ( !(obj = lua_tocinstance(L, 2, &t)) )
			luaL_typeerror(L, 1, "String, Buffer, File or Directory");
		if (t == TBuffer) {
			size_t len;
			const char *buff = luaL_tolstring(L, 2, &len);
			if (!dest_entry)
				luaL_argerror(L, 3, "string expected when writing Buffer to Zip archive");
			result = (zip_entry_open(z->zip, dest_entry) == 0) && ((zip_entry_write(z->zip, buff, len) == 0) || luaL_error(L, strerror(errno)));				
		}
		else if (t == TFile) {
			int size = -1;
			entry = wchar_toutf8(((File*)obj)->fullpath, &size);
			result = (zip_entry_open(z->zip, dest_entry ? dest_entry : PathFindFileNameA(entry)) == 0) && ((zip_entry_fwrite(z->zip, entry) == 0) || luaL_error(L, strerror(errno)));
			free(entry);
		}	
		else if (t == TDirectory) {
			char *dir = checkDirectory(L, 2);
			result = write_dir(z, dir, TRUE, dest);
			free(dir);
			goto done;
		}			
	}
	zip_entry_close(z->zip);
done:
	lua_pushboolean(L, result);
	return 1;
}

static size_t on_extract(void *arg, unsigned long long offset, const void *data, size_t size) {
	luaL_addlstring((luaL_Buffer *)arg, data, size);
    return size; 
}

LUA_METHOD(Zip, read) {
	Zip *z = lua_self(L, 1, Zip);
	luaL_Buffer b;
	char *entry = normalize(luaL_checkstring(L, 2));
	
	lua_pushboolean(L, FALSE);
	if (z->mode != 'r')
		luaL_error(L, "cannot read a Zip archive opened in write/append mode");
	
	if (zip_entry_open(z->zip, entry) == 0) {
		if (!zip_entry_isdir(z->zip)) {
			luaL_buffinit(L, &b);
			if (zip_entry_extract(z->zip, on_extract, &b) == 0) {
				luaL_pushresult(&b);
				lua_pushinstance(L, Buffer, 1);
			}
		} else zip_set_last_error(z->zip, MZ_ZIP_FILE_NOT_FOUND);
		zip_entry_close(z->zip);
	}
	free(entry);
	return 1;
}

extern BOOL make_path(wchar_t *folder);

uint64_t extract_zip(struct zip_t *z, const char *dir) {
	int	entry = -1;
	size_t slen;
	size_t size = 0, len = dir ? strlen(dir) : 0, count = 0;
	BOOL success = TRUE;

	while(((zip_entry_openbyindex(z, ++entry) == 0))) {
		const char *name = zip_entry_name(z);
		size = strlen(name);
		char *fname;
		int i;
		
		if (dir && (strncmp(dir, name, len) != 0)) 
			goto next;
		
		fname = malloc(++size);
		strncpy(fname, name, size);
		slen = size;
		fname[slen-1] = 0;
		for (i = 0; i < slen; i++) {
			if (fname[i] == '/') {
				fname[i] = 0;
				if (GetFileAttributes(fname) == INVALID_FILE_ATTRIBUTES)
					CreateDirectory(fname, NULL);
				fname[i] = L'\\';
			}
		}
		if ( !zip_entry_isdir(z) )
			success = (zip_entry_fread(z, fname) == 0);
		count++;
		free(fname);
next:
		zip_entry_close(z);
	}
	return count;
}

static wchar_t *prep_destdir(lua_State *L, int idx) {
	wchar_t *dir = luaL_checkDirname(L, idx);
	wchar_t *oldpath = GetCurrentDir();
	size_t len = wcslen(dir);

	if (dir[len-1] != L'\\') {
		wchar_t * new = (wchar_t *)calloc(len + sizeof(wchar_t), sizeof(wchar_t));
		wcscpy(new, dir);
		free(dir);
		dir = new;
		dir[len] = L'\\';
	}    
	make_path(dir);
	SetCurrentDirectoryW(dir);
	free(dir);
	return oldpath;
}

LUA_METHOD(Zip, extract)
{
	Zip *z = lua_self(L, 1, Zip);
	size_t len;
	int narg = lua_gettop(L);
	char *name = normalize(luaL_checklstring(L, 2, &len));
	BOOL include_path = narg > 3 ? lua_toboolean(L, 4) : TRUE;
	wchar_t *oldpath = NULL, *wfname = lua_towstring(L, 2);
	char *dname = calloc(1, len+2);

	strncpy(dname, name, len);
	dname[len] = '/';	
	if (narg > 2)
		oldpath = prep_destdir(L, 3);
	lua_pushboolean(L, FALSE);
	if (z->mode != 'r')
		luaL_error(L, "cannot extract from a Zip archive opened in write/append mode");
	if (zip_entry_open(z->zip, name) == 0) {
dir:	if (zip_entry_isdir(z->zip)) {
			zip_entry_close(z->zip);
			extract_zip(z->zip, name);		
			lua_pushvalue(L, 2);
			lua_pushinstance(L, Directory, 1);
		}
		else {
			if (include_path) {

				if (make_path(wfname) && (zip_entry_fread(z->zip, name) == 0))
					goto done;
			} else if (zip_entry_fread(z->zip, PathFindFileName(name)) == 0) {
done:			lua_pushstring(L, name);
				lua_pushinstance(L, File, 1);
				zip_entry_close(z->zip);
			}
		} 
	} else if (zip_entry_open(z->zip, dname) == 0)
		goto dir;
	free(wfname);
	free(dname);
	free(name);
	if (oldpath) {
		SetCurrentDirectoryW(oldpath);
		free(oldpath);
	}
	return 1;
}

LUA_METHOD(Zip, remove) {
	Zip *z = lua_self(L, 1, Zip);	
	int i, n = lua_gettop(L)-1;
	char **fnames = malloc(sizeof(char*)*n);
	
	if (z->mode != 'd')
		luaL_error(L, "Zip file must be opened in 'delete' mode to remove entries");
	for (i=0; i < n; i++)
		fnames[i] = (char*)luaL_checkstring(L, i+2);
	lua_pushboolean(L, (n = zip_entries_delete(z->zip, fnames, (size_t)n)) > 0);
	if (n == 0)
		zip_set_last_error(z->zip, MZ_ZIP_FILE_NOT_FOUND);
	free(fnames);
	return 1;
}

typedef struct {
	HANDLE thread;
	struct zip_t* zip;
	uint64_t result;
	char *name;
	BOOL extractall;
	char *dir;
	BOOL find_fname;
	wchar_t *oldpath;
} asyncZip;

static Extract_gc(lua_State *L) {
	asyncZip *z= (asyncZip*)lua_self(L, 1, Task)->userdata;
	CloseHandle(z->thread);
	free(z->oldpath);
	if (z->dir)
		free(z->dir);
	if (z->name)
		free(z->name);
	free(z);
	return 0;
}

static int ZipTaskContinue(lua_State* L, int status, lua_KContext ctx) {
    asyncZip *z = (asyncZip *)ctx;

    if (WaitForSingleObject(z->thread, 0) == WAIT_OBJECT_0) {
		lua_pushboolean(L, FALSE);
		if (z->dir) {
			if (z->result) {
				lua_pushstring(L, z->dir);
				lua_pushinstance(L, Directory, 1);
				free(z->dir);
			}
		} else if (z->name) {
			if (z->result) {
				lua_pushstring(L, z->name);
				lua_pushinstance(L, File, 1);
			}
		} else {
			if (z->result > INT64_MAX)
				lua_pushnumber(L, z->result);
			else
				lua_pushinteger(L, z->result);
		}
		if (z->oldpath) {
			SetCurrentDirectoryW(z->oldpath);
			free(z->oldpath);
		}
		CloseHandle(z->thread);
        return 1;
    }
    return lua_yieldk(L, 0, ctx, ZipTaskContinue);
}

static void push_ZipTask(lua_State *L, asyncZip *z, LPTHREAD_START_ROUTINE thread) {
    lua_pushtask(L, ZipTaskContinue, z, gc_asyncTask);
    lua_pushvalue(L, -1);
    if ((z->thread = CreateThread(NULL, 0, thread, z, 0, NULL)))
        lua_call(L, 0, 0); 
    else {
        luaL_getlasterror(L, GetLastError());
        luaL_error(L, "async error : %s", lua_tostring(L, -1));
    }   
}

static DWORD __stdcall extractThread(LPVOID data) {
    asyncZip *z = (asyncZip*)data;

	if (z->name) {
		z->result = !zip_entry_fread(z->zip, PathFindFileName(z->find_fname ? PathFindFileName(z->name) : z->name));
		zip_entry_close(z->zip);
	} else {
		z->result = extract_zip(z->zip, z->dir);
		if (!z->extractall)
			zip_entry_close(z->zip);
	}
    return 0;
}

LUA_METHOD(Zip, extractall) {
	wchar_t *oldpath = NULL;

	if (lua_gettop(L) > 1)
		oldpath = prep_destdir(L, 2);
	lua_pushinteger(L, extract_zip(lua_self(L, 1, Zip)->zip, NULL));
	if (oldpath) {
		SetCurrentDirectoryW(oldpath);
		free(oldpath);
	}
	return 1;
}

LUA_METHOD(Zip, extractall_async) {
	asyncZip *z = calloc(1, sizeof(asyncZip));

	if (lua_gettop(L) > 1)
		z->oldpath = prep_destdir(L, 2);	
	z->zip = lua_self(L, lua_upvalueindex(1), Zip)->zip;
	z->extractall = TRUE;
	push_ZipTask(L, z, extractThread);
	return 1;
}

LUA_METHOD(Zip, extract_async) {
	asyncZip *z;
	size_t len;
	int narg = lua_gettop(L);
	char *name = normalize(lua_tolstring(L, 1, &len));
	BOOL include_path = narg > 2 ? lua_toboolean(L, 3) : FALSE;
	wchar_t *fname = lua_towstring(L, 1);
	Zip *zip = lua_self(L, lua_upvalueindex(1), Zip);

	if (zip->mode != 'r')
		luaL_error(L, "cannot extract a Zip archive opened in write/append mode");
	z = calloc(1, sizeof(asyncZip));
	z->zip = zip->zip;
	if (narg >= 2)
		z->oldpath = prep_destdir(L, 2);
	if (zip_entry_open(z->zip, name) == 0) {
		if (zip_entry_isdir(z->zip))
			z->dir = strdup(name);
		else {
			z->name = strdup(name);
			if (include_path)
				make_path(fname);
			else z->find_fname = TRUE;
		} 
	} else {
		lua_pushboolean(L, FALSE);
		goto done;
	}
	push_ZipTask(L, z, extractThread);
done:
	free(name);
	free(fname);
	return 1;
}


LUA_METHOD(Zip, isdirectory) {
	Zip *z =lua_self(L, 1, Zip);
	size_t len = 0;
	const char *dir = luaL_checklstring(L, 2, &len);
	char *tmp = malloc(len+2);
	
	snprintf(tmp, len+2, "%s/", dir);
	lua_pushboolean(L, FALSE);
	if (zip_entry_open(z->zip, tmp) == 0) {
		lua_pushboolean(L, zip_entry_isdir(z->zip));
		zip_entry_close(z->zip);
	}
	free(tmp);
	return 1;
}

static const luaL_Reg zip_async[] = {
	{"extractall",	Zip_extractall_async},
	{"extract",		Zip_extract_async},
	{NULL, NULL}
};

LUA_PROPERTY_GET(Zip, async) {
	luaL_newlib(L, zip_async);
	lua_createtable(L, 0, 2);
	lua_pushvalue(L, 1);
	lua_pushcclosure(L, Zip_extractall_async, 1);
	lua_setfield(L, -2, "extractall");
	lua_pushvalue(L, 1);
	lua_pushcclosure(L, Zip_extract_async, 1);
	lua_setfield(L, -2, "extract");
	return 1;
}

LUA_PROPERTY_GET(Zip, file) {
	lua_pushstring(L, lua_self(L, 1, Zip)->fname);
	lua_pushinstance(L, File, 1);
	return 1;
}

static int Zip_iter(lua_State *L) {
	Zip *z = lua_self(L, lua_upvalueindex(1), Zip); 
	int entry = (int)lua_tointeger(L, lua_upvalueindex(2));
	if (zip_entry_openbyindex(z->zip, entry) == 0) {
		const char *str = zip_entry_name(z->zip);	
		size_t len = strlen(str);
		lua_pushlstring(L, str, str[len-1]=='/' ? len-1 : len);
		lua_pushboolean(L, zip_entry_isdir(z->zip));
		lua_pushinteger(L, ++entry);
		lua_replace(L, lua_upvalueindex(2));
		zip_entry_close(z->zip);
		return 2;
	}
	return 0;
}

LUA_METHOD(Zip,__iterate) {
	lua_pushvalue(L, 1);
	lua_pushinteger(L, 0);
	lua_pushcclosure(L, Zip_iter, 2);
	return 1;
}

const luaL_Reg Zip_metafields[] = {
	{"__gc",		Zip___gc},
	{"__iterate",	Zip___iterate},
	{NULL, NULL}
};

const luaL_Reg Zip_methods[] = {
	METHOD(Zip, close)
	METHOD(Zip, write)
	METHOD(Zip, read)
	METHOD(Zip, reopen)
	METHOD(Zip, extract)
	METHOD(Zip, extractall)
	METHOD(Zip, isdirectory)
	METHOD(Zip, remove)
	READONLY_PROPERTY(Zip, count)
	READONLY_PROPERTY(Zip, size)
	READONLY_PROPERTY(Zip, file)
	READONLY_PROPERTY(Zip, iszip64)
	READONLY_PROPERTY(Zip, error)
	READONLY_PROPERTY(Zip, async)
	{NULL, NULL}
};

