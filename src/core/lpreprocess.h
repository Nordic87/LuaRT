#pragma once

#include <luart.h>

extern int original_searcher_ref;
int lua_preload_searcher(lua_State* L);
char* preprocess_lua(lua_State *L, const char* src);
char* preprocess_file(lua_State *L, const char* filename);