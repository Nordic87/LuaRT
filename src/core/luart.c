/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | luart.c | LuaRT interpreter
*/

#include <windows.h>
#include <commctrl.h>
#include <conio.h>
#include <time.h>
#include <locale.h>
#include <malloc.h>
#include <stdint.h>
#include "lrtapi.h"
#include <Task.h>
#include <wchar.h>
#include <stdlib.h>
#include <shlwapi.h>
#include <shlobj_core.h>
#include <luart.h>
#include "lpreprocess.h"
#include "resources\resource.h"

#ifndef _MSC_VER
	#ifdef __cplusplus
	extern "C" {
	#endif
		extern int _CRT_glob;
		void __wgetmainargs(int*,wchar_t***,wchar_t***,int,int*);
	#ifdef __cplusplus
	}
	#endif
#endif

extern struct zip_t *fs;
extern BYTE *datafs;
static lua_State *L;
static WCHAR exename[MAX_PATH];


#ifdef RTC

#include <stdio.h>
#include <stdint.h>
#include <File.h>

extern void xor_buffer_lenbytes(uint8_t *data, size_t len);

static int link(lua_State *L) {
	wchar_t *fname = lua_towstring(L, 1);
	wchar_t *fexe = lua_towstring(L, 2);
	HANDLE hFile = CreateFileW(fname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	DWORD sizeHigh, sizeLow = GetFileSize(hFile, &sizeHigh);
	uint64_t size = ((uint64_t)sizeHigh << 32) | sizeLow;
	HANDLE hMap = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, sizeHigh, sizeLow, NULL);
	uint8_t *map = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	int result = FALSE;

	if (map) {
		BOOL result = FALSE;
		uint8_t key = (uint8_t)size;
		const size_t CHUNK = 1 << 20;

		for (uint64_t off = 0; off < size; off += CHUNK) {
			size_t n = (size - off > CHUNK) ? CHUNK : (size - off);
			uint8_t *p = map + off;

			for (size_t i = 0; i < n; i++)
				p[i] ^= key;
		}
		HANDLE hExe = BeginUpdateResourceW(fexe, FALSE);
		result = UpdateResource(hExe, RT_RCDATA, MAKEINTRESOURCE(EMBED), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), map, (DWORD)size);
		EndUpdateResource(hExe, FALSE);
		UnmapViewOfFile(map);
		result = TRUE;
	}
	CloseHandle(hMap);
	CloseHandle(hFile);
	free(fname);
	free(fexe);
	lua_pushboolean(L, result);
	return result;
}

#pragma pack(2)
struct resource_directory
{
	int8_t width;
	int8_t height;
	int8_t color_count;
	int8_t reserved;
	int16_t planes;
	int16_t bit_count;
	int32_t bytes_in_resource;
	int16_t id;
};

struct header
{
	int16_t reserved;
	int16_t type;
	int16_t count;
};

struct icon_header
{
	int8_t width;
	int8_t height;
	int8_t color_count;
	int8_t reserved;
	int16_t planes;
	int16_t bit_count;
	int32_t bytes_in_resource;
	int32_t image_offset;
};

struct icon_image
{
	BITMAPINFOHEADER header;
	RGBQUAD colors;
	int8_t xors[1];
	int8_t ands[1];
};

struct icon
{
	int count;
	struct header *header;
	struct resource_directory *items;
	struct icon_image **images;
};

static int update_exe_icon(lua_State *L) {
	
	wchar_t *exe_path = luaL_checkFilename(L, 1);
	wchar_t *ico_path = luaL_checkFilename(L, 2);
	FILE *file;
	BOOL result = FALSE;

	if ((file = _wfopen(ico_path, L"rb"))) {
		int i, id = 1;
		struct icon icon;
		HANDLE handle;
		struct header file_header;
   		if ((handle = BeginUpdateResourceW(exe_path, FALSE))) {
			fread(&file_header, sizeof(struct header), 1, file);
			icon.count = file_header.count;
			icon.header = malloc(sizeof(struct header) + icon.count * sizeof(struct resource_directory));
			icon.header->reserved = 0;
			icon.header->type = 1;
			icon.header->count = icon.count;
			icon.items = (struct resource_directory *)(icon.header + 1);
			struct icon_header *icon_headers = malloc(icon.count * sizeof(struct icon_header));
			fread(icon_headers, icon.count * sizeof(struct icon_header), 1, file);
			icon.images = malloc(icon.count * sizeof(struct icon_image *));

			for (i = 0; i < icon.count; i++) {
				struct icon_image** image = icon.images + i;
				struct icon_header* icon_header = icon_headers + i;
				struct resource_directory *item = icon.items + i;

				*image = malloc(icon_header->bytes_in_resource);
				fseek(file, icon_header->image_offset, SEEK_SET);
				fread(*image, icon_header->bytes_in_resource, 1, file);

				memcpy(item, icon_header, sizeof(struct resource_directory));
				item->id = (int16_t)(i + 1);
			}
			
			UpdateResource(handle, RT_GROUP_ICON, MAKEINTRESOURCE(101), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), icon.header, sizeof(struct header) + icon.count * sizeof(struct resource_directory));
			for (i = 0; i < icon.count; i++)
				UpdateResource(handle, RT_ICON, MAKEINTRESOURCE(id++), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), icon.images[i], icon.items[i].bytes_in_resource);
			result = EndUpdateResource(handle, FALSE);
			for (i = 0; i < icon.count; i++)
				free(icon.images[i]);
			free(icon.images);
			free(icon.header);
			SHChangeNotify(0x08000000, 0, NULL, NULL);
		}
		fclose(file);
	}
	free(ico_path);
	free(exe_path);
    return result;
}
#endif

#ifdef RTWIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main() {
#endif
	INITCOMMONCONTROLSEX icex;
	int i, result = EXIT_SUCCESS;
	BOOL is_embeded = FALSE;
	wchar_t **envp, **wargv;
	int argc, si = 0, script_index = 0;
	Task *t;
#ifdef _MSC_VER	
    static int (*__wgetmainargs)(int *, wchar_t ***, wchar_t ***, int, int *);
    HMODULE h = LoadLibraryA("msvcrt.dll");
    __wgetmainargs = (void *)GetProcAddress(h, "__wgetmainargs");
    (*__wgetmainargs)(&argc, &wargv, &envp, 1, &si);
    FreeLibrary(h);
#else	
	__wgetmainargs(&argc, &wargv, &envp, _CRT_glob, &si);
#endif
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icex);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	L = luaL_newstate();
	lua_openmodules(L);
	lua_warning(L, "@off", 0);
	GetModuleFileNameW(NULL, (WCHAR*)exename, sizeof(exename));
	if ((is_embeded = (luaL_embedopen(L) != NULL))) {
		luaL_requiref(L, "embed", luaopen_embed, 2);
#ifdef RTSTATIC
		char tmpdir[MAX_PATH];
		GetTempPathA(MAX_PATH, tmpdir);
		char cpath[MAX_PATH * 2];
		snprintf(cpath, sizeof(cpath), ";%s\\__modules\\?\\?-static.dll", tmpdir);
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "cpath");
		lua_pushstring(L, cpath);
		lua_concat(L, 2);
		lua_setfield(L, -2, "cpath");
		lua_pop(L, 2);
#else
		lua_pop(L, 1);
#endif
	}
#ifndef RTSTATIC	
	atexit(lua_stop);
#endif
#ifdef RTC
	lua_register(L, "seticon", update_exe_icon);
	lua_register(L, "link", link);
#endif
	// --- Argument Parsing and Handling ---

	if (argc == 1 && !is_embeded) {
help:
		puts(LUA_VERSION " " LUA_ARCH " - Windows programming framework for Lua.\nCopyright (c) 2026, Samir Tine.\nusage:\tluart.exe [options] [script] [args]\n\n\t-e stat\t\tExecutes the given Lua statement\n\t-l name\t\tRequire library 'name'\n\t-v\t\tShow version information\n\tscript\t\tRun a Lua script file\n\targs\t\tArguments for Lua interpreter");
	} else {
		// 1. Identify script index
		if (is_embeded) {
			script_index = 0; 
		} else {
			for (i = 1; i < argc; i++) {
				if (wargv[i][0] == L'-') {
					switch(wargv[i][1]) {
						case L'e':
						case L'p':
						case L'l':
						case L'v':
							break;
						default:
							lua_pushfstring(L, "error: unknown option '%s'", __argv[i]);
							goto error;
					}
				} else {
					script_index = i;
					break;
				}
			}
		}

		// 2. Build 'arg' table
		lua_createtable(L, argc, 0);
		for (i = 1; i < argc + is_embeded; i++) {
			lua_pushwstring(L, wargv[i-is_embeded]);
			lua_rawseti(L, -2, i - 1);
		}
		lua_pushwstring(L, exename);
		lua_rawseti(L, -2, -1);
		lua_setglobal(L, "arg");
		lua_gc(L, LUA_GCGEN, 0, 0);

		// 3. Process Options (Pass 2)
		if (!is_embeded) {
			for (i = 1; i < argc; i++) {
				if (script_index > 0 && i >= script_index) break; 

				if (wargv[i][0] == L'-') {
					switch(wargv[i][1]) {
						case L'p':
							char* output = preprocess_file(L, __argv[++i]);
							if (!output) goto error;
							fputs(output, stdout);
							free(output);
							return EXIT_SUCCESS;
						case L'v':
							fputs(LUA_VERSION " " LUA_ARCH " Copyright (c) 2026, Samir Tine.\n", stdout);
							if (argc == 2) return EXIT_SUCCESS; 
							break;
						case L'l': 
							{
								if (wcslen(wargv[i]) == 2) {
									lua_pushstring(L, "error: expecting module name after -l option");
									goto error;
								}
								lua_getglobal(L, "require");
								lua_pushwstring(L, wargv[i]+2);
								if (lua_pcall(L, 1, 1, 0) >= LUA_ERRRUN) goto error;
								lua_setglobal(L, __argv[i]+2);
							}
							break;
						case L'e':
							{
								lua_pushwstring(L, wargv[++i]);
								if (luaL_loadstring(L, lua_tostring(L, -1)) != LUA_OK) goto error;
								lua_remove(L, -2); 
								if (lua_pcall(L, 0, 0, 0) != LUA_OK) goto error;
								lua_stop();
								return 0;
							}
							break;
					}
				}
			}
		}

		// 4. Load and Run Script
		if (is_embeded) {	
			if ( luaL_loadbufferx(L, "require('__mainLuaRTStartup__')", sizeof("require('__mainLuaRTStartup__')")-1, __argv[0], "t") != LUA_OK )
				goto error;
		} else {
			if (script_index > 0) {
				if (luaL_loadfilep(L, __argv[script_index], NULL) != LUA_OK) 
					goto error;
			} else
				goto help;
		}

		// Execute the loaded chunk (embedded or file)
		t = (Task*)lua_pushinstance(L, Task, 1);
		if (lua_pcall(L, 0, 0, 0)) 
			goto error;

		do {
			if (!lua_schedule(L)) {
error:			
#ifdef RTWIN
				size_t len;
				const char *err = lua_tolstring(L, -1, &len);	
				if (len)	{	
					if (is_embeded) {
						if (strstr(err, "[string """) == err) {
							const char *tmp = luaL_gsub(L, err, "[string \"", "");
							err = luaL_gsub(L, tmp, "\"]:", ":");
							len = strlen(err);
						}
					}
					lua_pushstring(L, err);							
					wchar_t *msg = lua_towstring(L, -1);		
					MessageBoxW(NULL, msg, L"Runtime error", MB_ICONERROR | MB_OK);
					free(msg);					
				}
#else
				const char *err = lua_tostring(L, -1);
				if (is_embeded) {
					if (strstr(err, "[string """) == err) {
						const char *tmp = luaL_gsub(L, err, "[string \"", "");
						err = luaL_gsub(L, tmp, "\"]:", ":");
					} 
				}
				fputs(err, stderr);
				fputs("\n", stderr);
#endif
				result = EXIT_FAILURE;
				break;
			}  
		} while (t->status != TTerminated);
	}
	lua_stop();
	return result;
}