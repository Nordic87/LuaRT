/*
 | Camera for LuaRT
 | Luart.org, Copyright (c) Tine Samir 2026.
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Camera.h | LuaRT binary module 
*/

#pragma once

#include "Capture.h"


extern luart_type TCamera;
extern UIInterface *ui;

typedef struct {
    luart_type type;
    Capture *capture;
} Camera;

LUA_CONSTRUCTOR(Camera);
extern const luaL_Reg Camera_methods[];
extern const luaL_Reg Camera_widget_methods[];
extern const luaL_Reg Camera_metafields[];

void register_camera(lua_State *L);