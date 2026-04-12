/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | lembed.c | LuaRT embed module implementation
*/

#define LUA_LIB

#include "lrtapi.h"
#include <luart.h>
#include "lua\llimits.h"

#include <File.h>
#include <shlwapi.h>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

#define MINIZ_HEADER_FILE_ONLY
#include <compression\lib\miniz.h>
#include <compression\lib\zip.h>
#include "resources\resource.h"

#define CMEMLIBS "DLL binary modules"

struct zip_t;

typedef void (*voidf)(void);
extern int Zip_extract(lua_State *L); 

typedef struct {
	struct zip_t *zip;
	wchar_t       tmp[MAX_PATH];
	HANDLE        hMap;
	HANDLE        hFile;
	uint8_t       *map;
} ZipFs;
extern ZipFs fs;

BYTE *datafs  = NULL;
wchar_t path[MAX_PATH];
wchar_t uniquePath[512] =  {0};

#define WIDE2(x) L##x
#define WIDE(x)  WIDE2(x)

BOOL open_fs(void *ptr, size_t size) {
  wchar_t tempPath[MAX_PATH];
  ULONGLONG size64 = (ULONGLONG)size;

  GetTempPathW(MAX_PATH, tempPath);
  GetTempFileNameW(tempPath, L"ZIP", 0, fs.tmp);

  fs.hFile = CreateFileW(fs.tmp, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
  fs.hMap = CreateFileMappingW(fs.hFile, NULL, PAGE_READWRITE, (DWORD)(size64 >> 32), (DWORD)(size64 & 0xFFFFFFFF), NULL);
  fs.map = MapViewOfFile(fs.hMap, FILE_MAP_WRITE, 0, 0, size);

  if (!fs.map)
          return FALSE;
  uint8_t *enc = (uint8_t *)ptr;
  uint8_t *map = (uint8_t *)fs.map;
  uint8_t key = (uint8_t)size;

  const size_t CHUNK = 1 << 20;
  size_t offset = 0;

  while (offset < size) {
      size_t n = (size - offset > CHUNK) ? CHUNK : (size - offset);
      for (size_t i = 0; i < n; i++)
          map[offset + i] = enc[offset + i] ^ key;
      offset += n;
  }
  fs.zip = zip_mem_new(fs.map, size);
  return fs.zip != NULL;
}

static BOOL CreateUniqueTempDir() {
    if (GetTempFileNameW(temp_path, WIDE(LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE), 0, uniquePath) == 0)
        return FALSE;
    DeleteFileW(uniquePath);

    if (!CreateDirectoryW(uniquePath, NULL))
        return FALSE;

    return (wcslen(uniquePath) + 1 < 512);
}

static const char *getnextfilename (char **path, char *end) {
  char *sep;
  char *name = *path;
  if (name == end)
    return NULL;  /* no more names */
  else if (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  if (sep == NULL)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  return name;
}

static void pusherrornotfound (lua_State *L, const char *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no embedded file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno embedded file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}

static const char *searchpath (lua_State *L, const char *name,
                                             const char *path,
                                             const char *sep,
                                             const char *dirsep) {
  luaL_Buffer buff;
  char *pathname;  /* path with name inserted */
  char *endpathname;  /* its end */
  const char *filename;
  /* separator is non-empty and appears in 'name'? */
  if (*sep != '\0' && strchr(name, *sep) != NULL)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  while ((filename = getnextfilename(&pathname, endpathname)) != NULL) {
    filename = luaL_gsub(L, filename, "\\", "/");
    if (filename[0] == '.' && filename[1] == '/')
      filename += 2;
    lua_pop(L, 1); 
    CharLowerA((char *)filename);
    if ((zip_entry_open(fs.zip, filename) == 0))
      /* does file exist and is readable? */
      return lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));
  return NULL;  /* not found */
}

static const char *findfile (lua_State *L, const char *name,
                                           const char *pname,
                                           const char *dirsep) {
  const char *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  if (l_unlikely(path == NULL))
    luaL_error(L, "'package.%s' must be a string", pname);
  return searchpath(L, name, path, ".", dirsep);
}

static int searcher_embedded_Lua (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  const char *filename = findfile(L, name, "path", "/");
  if (filename) {
    char *buff = NULL; 
	  int idx = 0;
	  size_t size;

    zip_entry_read(fs.zip, (void**)&buff, &size);
    if (memcmp(buff, "\xEF\xBB\xBF", 3)==0)
      idx = 3;
    if (luaL_loadbuffer(L, (const char*)(buff+idx), size-idx, filename))
      luaL_error(L, "error loading module '%s' from embedded file '%s':\n\t%s", lua_tostring(L, 1), filename, lua_tostring(L, -1));
    zip_entry_close(fs.zip);
    lua_pushstring(L, filename); 
    return 2;   
  }
  return 1;
}

extern BOOL make_path(wchar_t *folder);

static char* get_directory(const char* filepath) {
  const char* last_sep = strrchr(filepath, '/');
  if (!last_sep)
    last_sep = strrchr(filepath, '\\');

  if (!last_sep)
    return NULL;

  size_t len = last_sep - filepath;
  char* dir = (char*)malloc(len + 1);
  if (dir) {
      strncpy(dir, filepath, len);
      dir[len] = '\0';
  }
  return dir;
}

static int searcher_embedded_C (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  const char *filename = findfile(L, name, "cpath", "/");
  if (filename) { 
found:
    lua_getfield(L, LUA_REGISTRYINDEX, CMEMLIBS);
    lua_pushwstring(L, uniquePath);
    lua_pushfstring(L, "/%s", filename);
    lua_concat(L, 2);
    if (zip_entry_open(fs.zip, filename) == 0) { 
      wchar_t *tmp = lua_towstring(L, -1);
      make_path(tmp);
      if (zip_entry_fread(fs.zip, lua_tostring(L, -1)) == 0) {
        HMODULE hm;
        char *modpath = get_directory(filename);
        
        if ( (hm = LoadLibraryExW(tmp, NULL, DONT_RESOLVE_DLL_REFERENCES)) ) {
          ULONG size;
          PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToDataEx(hm, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size, NULL);

          if (importDescriptor)
            while (importDescriptor->Characteristics && importDescriptor->Name) {
                PSTR importName = (PSTR)((PBYTE)hm + importDescriptor->Name);
                char modname[512];
                char fname[512];
                wchar_t dllpath[512];
                snprintf(modname, 512, "%s/%s", modpath, importName);
                snprintf(fname, 512, "%ls\\%s", uniquePath, importName);
                _snwprintf(dllpath, 512, L"%ls\\%hs", uniquePath, modpath);
                if (zip_entry_open(fs.zip, modname) == 0) {
                    make_path(dllpath); 
                    zip_entry_fread(fs.zip, fname);
                    zip_entry_close(fs.zip);
                }
                importDescriptor++;
            }
          FreeLibrary(hm);
          if ( (hm = LoadLibraryW(tmp)) ) {
            char funcname[MAX_PATH];
            _snprintf(funcname, MAX_PATH, "luaopen_%s", luaL_gsub(L, name, ".", "_"));
            lua_pop(L, 1);
            lua_CFunction f = (lua_CFunction)(voidf)GetProcAddress(hm, funcname);
            if (f) {
              lua_pushlightuserdata(L, (void*)hm);
              lua_rawset(L, -3);
              lua_pushcfunction(L, f);
            } else FreeLibrary(hm);
          } else {
            luaL_getlasterror(L, GetLastError());
            luaL_error(L, "error loading module '%s' from embedded file '%s' : %s", lua_tostring(L, 1), filename, lua_tostring(L, -1));
          }
        }
        free(modpath);
      }
      free(tmp);
      zip_entry_close(fs.zip);
      if (lua_isfunction(L, -1)) {
        lua_pushstring(L, filename); 
        return 2;   
      }
    }
  }
  const char *p = strchr(name, '.');
  if (p == NULL) return 0;  /* is root */
  lua_pushlstring(L, name, p - name);
  if ((filename = findfile(L, lua_tostring(L, -1), "cpath", "/")))
    goto found;
  lua_pushfstring(L,"no embedded file '%s'", filename);
  return 1;
}

//-------------------------------------------------[luaL_embedclose() luaRT C API]
LUA_API int luaL_embedclose(lua_State *L) {
  if (lua_getfield(L, LUA_REGISTRYINDEX, CMEMLIBS)) {
    wchar_t doubleNull[514];
    wchar_t *path;
    lua_pushnil(L);
    while (lua_next(L, -2)) {
      path = lua_towstring(L, -2);
      FreeLibrary(lua_touserdata(L, -1));
      DeleteFileW(path);
      free(path);
      lua_pop(L, 1);
    }

    SHFILEOPSTRUCTW op = {0};
    op.wFunc  = FO_DELETE;
    op.pFrom  = doubleNull;
    op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    SHFileOperationW(&op);

    if (fs.zip) {
      UnmapViewOfFile(fs.map);
      CloseHandle(fs.hMap);
      CloseHandle(fs.hFile);
      DeleteFileW(fs.tmp);
    }
  }
	return 0;
}

//-------------------------------------------------[luaL_embedopen() luaRT C API]
LUA_API BYTE *luaL_embedopen(lua_State *L) {
  if (!datafs) {
    HRSRC hres = FindResource(NULL, MAKEINTRESOURCE(EMBED), RT_RCDATA);
    HGLOBAL hdata = LoadResource(NULL, hres);
    datafs = LockResource(hdata);
    size_t size = SizeofResource(NULL, hres);
    if (datafs && (size > 2) && open_fs(datafs, size)) {
      lua_Integer len;
      lua_getglobal(L, "package");
      lua_getfield(L, -1, "searchers");
      len = luaL_len(L, -1);
      lua_pushvalue(L, -2);
      lua_pushcclosure(L, searcher_embedded_Lua, 1);
      lua_rawseti(L, -2, ++len);
      lua_pushvalue(L, -2);
      lua_pushcclosure(L, searcher_embedded_C, 1);
      lua_rawseti(L, -2, ++len);
      lua_pop(L, 2);
      SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
      AddDllDirectory(uniquePath);
    };
  }
  return (BYTE*)fs.zip;
}

LUA_METHOD(embed, File) {
    lua_getglobal(L, "embed");
    lua_pushcfunction(L, Zip_extract);
    lua_getfield(L, -2, "zip");
    lua_pushvalue(L, 1);
    lua_pushwstring(L, uniquePath);
    lua_pcall(L, 3, LUA_MULTRET, 0);
    return 1;
}

static embed_entry(lua_State *L, const char *type) {
  if (!lua_isstring(L, 1))
    luaL_error(L, "Bad argument #2 (string expected, found %s)", luaL_typename(L, 2));
  embed_File(L);
  if (!lua_toboolean(L, -1)) {
    lua_pushvalue(L, 1);
    lua_pushnewinstance(L, type, 1);
  }
  return 1;
}

LUA_METHOD(embed, sysFile) {
  return embed_entry(L, "File");
}

LUA_METHOD(embed, sysDirectory) {
  return embed_entry(L, "Directory");
}

static const luaL_Reg embedlib[] = {
	{"File",	    embed_File},
	{"Directory",	embed_File},
	{NULL, NULL}
};

static const luaL_Reg embed_properties[] = {
	{NULL, NULL}
};

//-------------------------------------------------[luaopen_embed() "embed" module]
LUAMOD_API int luaopen_embed(lua_State *L) {
  if (!CreateUniqueTempDir())
    return luaL_error(L, "failed to access system temporary folder");
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CMEMLIBS);
	lua_registermodule(L, "embed", embedlib, embed_properties, luaL_embedclose);
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaded");
  lua_getfield(L, -1, "compression");
  lua_pushstring(L, "zip");
  lua_getfield(L, -2, "Zip");
  lua_pushlightuserdata(L, fs.zip);
  lua_pcall(L, 1, 1, 0);
  lua_rawset(L, -6);
  lua_pop(L, 1);
  lua_getfield(L, -1, "sys");
  lua_pushcfunction(L, embed_sysFile);
  lua_setfield(L, -2, "File");
  lua_pushcfunction(L, embed_sysDirectory);
  lua_setfield(L, -2, "Directory");
  lua_pop(L, 3);
	return 1;
};
