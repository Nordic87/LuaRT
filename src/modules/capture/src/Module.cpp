/*
 | Camera for LuaRT
 | Luart.org, Copyright (c) Tine Samir 2026.
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Camera.cpp | LuaRT binary module
*/

#undef UNICODE
#include <luart.h>
#include "../../ui/src/Widget.h"
#include "../../ui/src/ui.h"
#define UNICODE
#include "Camera.h"

static Capture *capture = NULL;

LUA_PROPERTY_GET(capture, devices) {
    lua_createtable(L, 0, 2);
    auto videoDevices = Capture::GetVideoDeviceNames();
    auto audioDevices = Capture::GetAudioDeviceNames();
    lua_createtable(L, videoDevices.size(), 0);
    for (size_t i = 0; i < videoDevices.size(); ++i) {
        lua_pushwstring(L, videoDevices[i].c_str());
        lua_rawseti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "video");
    lua_createtable(L, audioDevices.size(), 0);
    for (size_t i = 0; i < audioDevices.size(); ++i) {
        lua_pushwstring(L, audioDevices[i].c_str());
        lua_rawseti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "audio");    
    return 1;
}

MODULE_PROPERTIES(capture)
    READONLY_PROPERTY(capture, devices)
END

MODULE_FUNCTIONS(capture) 
END

UIInterface *ui = NULL;
luart_type TWidget;

extern "C" {
    int __declspec(dllexport) luaopen_capture(lua_State *L) {
        lua_getglobal(L, "package");
        if (lua_getfield(L, -1, "loaded") && lua_getfield(L, -1, "ui")) {
            luaL_getmetafield(L, -1, "__interface");
            ui = (UIInterface *)lua_touserdata(L, -1);
        }
        lua_regmodule(L, capture);
        if (ui) {
            TWidget = ui->TWidget;
            lua_pushvalue(L, -3);
            ui->lua_regwidgetmt(L, Camera, ui->WIDGET_METHODS, FALSE, FALSE, TRUE, FALSE, TRUE);
            luaL_setrawfuncs(L, Camera_methods);
            luaL_setrawfuncs(L, Camera_widget_methods);
            lua_pop(L, 2);
        } else lua_regobjectmt(L, Camera);
        return 1;
    }
}
