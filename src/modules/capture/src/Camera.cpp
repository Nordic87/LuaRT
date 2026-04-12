/*
 | Camera for LuaRT
 | Luart.org, Copyright (c) Tine Samir 2026.
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Camera.cpp | LuaRT binary module
*/

#undef UNICODE
#include <luart.h>
#include "../../ui/src/ui.h"
#include "../../ui/src/Widget.h"
#define UNICODE
#include "Camera.h"
#include "Capture.h"

using namespace winrt::Windows::Media::MediaProperties;

//--- Settings helpers
const char *Aspect[] = { "stretch", "4x3", "16x9", NULL };
const char *VideoQuality[] = { "Auto", "VGA", "HD720p", "HD1080p", "UHD2k", "UHD4k", NULL };
const VideoEncodingQuality VideoQualityValues[] = { VideoEncodingQuality::Auto,
                                VideoEncodingQuality::Vga,
                                VideoEncodingQuality::HD720p,
                                VideoEncodingQuality::HD1080p,
                                VideoEncodingQuality::Uhd2160p,
                                VideoEncodingQuality::Uhd4320p};

const char *Bitrate[] = { "low", "medium", "high", "ultra", NULL };
const uint32_t VideoBitrateValues[] = { 1000000, 2000000, 5000000, 10000000 };
const uint32_t AudioBitrateValues[] = { 64000, 128000, 192000, 256000 };

//--- Camera type
luart_type TCamera;

//--- Camera procedure
LRESULT CALLBACK CameraProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
  Widget *w = (Widget *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  Capture *c = (Capture *)w->user;

  if (uMsg == WM_SIZE) {
    
      c->OnWindowResize(LOWORD(lParam), HIWORD(lParam));
      return 0;
  }
  return ui->lua_widgetproc(hwnd, uMsg, wParam, lParam, 0, 0);
}

//------------------------------------ Camera constructor

LUA_CONSTRUCTOR(Camera)
{   
  Camera *cam = (Camera *)calloc(1, sizeof(Camera));
  Capture * capture = NULL;
  wchar_t* audio = NULL, *video = NULL;
  int idx = 2 + (ui != NULL);

  if (lua_istable(L, idx)) {
    luaL_checktype(L, idx, LUA_TTABLE);
    if (lua_getfield(L, idx, "video"))
      video = lua_towstring(L, -1);
    if (lua_getfield(L, idx, "audio"))
      audio = lua_towstring(L, -1);
    lua_pop(L, 2);
    idx++;
  }

  if (!ui) {
    capture = new Capture(video, audio);
    cam->capture = capture;
    lua_newinstance(L, cam, Camera);
  } else {
    Widget *w, *wp;
    double dpi;
    BOOL isdark;
    HWND h, hParent = (HWND)ui->lua_widgetinitialize(L, &wp, &dpi, &isdark);
    int width = (int)luaL_optinteger(L, idx+2, 320)*dpi, height = (int)luaL_optinteger(L, idx+3, 240)*dpi;
    
    h = CreateWindowExW(0, L"Window", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS, (int)luaL_optinteger(L, idx, 0)*dpi, (int)luaL_optinteger(L, idx+1, 0)*dpi, width, height, hParent, 0, GetModuleHandle(NULL), NULL);    
    w = ui->lua_widgetconstructor(L, h, TCamera, wp, (SUBCLASSPROC)CameraProc);
    try {
      capture = new Capture(h, video, audio);;
      w->user = capture;
    } catch (const std::exception &e) {
      luaL_error(L, "Failed to create Camera widget: %s", e.what());
    }
  }
  while (!capture->IsReady()) {
    lua_sleep(L, 10);
  }
  free(video); 
  free(audio);
  return 1;
}

LUA_METHOD(Camera, __gc) {
 if (ui) {
    Widget *w = ui->lua_widgetdestructor(L);
    if (w->user) {
        delete (Capture*)w->user;
        w->user = NULL;
    }
    free(w);
  } else {
    Camera *c = (Camera*)lua_self(L, 1, Camera);    
    if (c->capture) {
        delete c->capture;
        c->capture = NULL;
    }
  }
  return 0;
}

LUA_METHOD(Camera, snapshot) {
  Capture *c = ui ? (Capture*)(lua_selfwidget(L, 1))->user : lua_self(L, 1, Camera)->capture;
  wchar_t *filePath = luaL_checkFilename(L, 2);
  bool result = true;
  
  try {
    c->SnapshotAsync(filePath);
  } catch (winrt::hresult_error const& ex) {
    result = false;
  }
  free(filePath); 
  lua_pushboolean(L, result);
  return 1;
}

LUA_METHOD(Camera, startrecording) {
  Capture *c = (Capture *)lua_touserdata(L, lua_upvalueindex(1));
  if (c->IsRecording()) 
    c->StopRecordingAsync();
  luaL_checktype(L, 1, LUA_TTABLE); 
  if (!lua_getfield(L, 1, "file"))
    luaL_error(L, "Missing 'file' field in recording options table");
  wchar_t *filePath = luaL_checkFilename(L, -1);
  lua_getfield(L, 1, "format");
  auto quality = VideoQualityValues[luaL_checkoption(L, -1, "HD720p", VideoQuality)];
  lua_getfield(L, 1, "video");
  uint32_t videoBitrate = VideoBitrateValues[luaL_checkoption(L, -1, "high", Bitrate)];
  lua_getfield(L, 1, "audio");
  uint32_t audioBitrate = AudioBitrateValues[luaL_checkoption(L, -1, "medium", Bitrate)];

  c->StartRecordingAsync(filePath, quality, 2, 44100, audioBitrate, videoBitrate);
  free(filePath);
	return 0;
}

LUA_METHOD(Camera, stoprecording) {
  Capture *c = (Capture *)lua_touserdata(L, lua_upvalueindex(1));
  if (c->IsRecording()) 
    c->StopRecordingAsync();
	return 0;
}

LUA_PROPERTY_GET(Camera, recording) {
  Capture *c = ui ? (Capture*)(lua_selfwidget(L, 1))->user : lua_self(L, 1, Camera)->capture;
  lua_pushboolean(L, c->IsRecording());
  return 1;
}

LUA_PROPERTY_GET(Camera, aspect) {
  Capture *c = ui ? (Capture*)(lua_selfwidget(L, 1))->user : lua_self(L, 1, Camera)->capture;
  lua_pushstring(L, Aspect[(int)c->m_previewMode]);
  return 1;
}

LUA_PROPERTY_SET(Camera, aspect) {
  Capture *c = ui ? (Capture*)(lua_selfwidget(L, 1))->user : lua_self(L, 1, Camera)->capture;
  c->m_previewMode = (PreviewMode)luaL_checkoption(L, 2, "4:3", Aspect);
  return 0;
}

LUA_PROPERTY_GET(Camera, record) {
  Capture *c = ui ? (Capture*)(lua_selfwidget(L, 1))->user : lua_self(L, 1, Camera)->capture;
  lua_createtable(L, 0, 2);
  lua_pushlightuserdata(L, (void*)c);
  lua_pushcclosure(L, Camera_startrecording, 1);
  lua_setfield(L, -2, "start");
  lua_pushlightuserdata(L, (void*)c);
  lua_pushcclosure(L, Camera_stoprecording, 1);
  lua_setfield(L, -2, "stop");
  return 1;
}

LUA_PROPERTY_GET(Camera, border) {
	lua_pushboolean(L, (GetWindowLong((HWND)lua_selfwidget(L, 1)->handle, GWL_STYLE) & WS_BORDER));
	return 1;
}

LUA_PROPERTY_SET(Camera, border) {
	HWND h = (HWND)lua_selfwidget(L, 1)->handle;
	DWORD style = GetWindowLong(h, GWL_STYLE);

	style = lua_toboolean(L, 2) ? style | WS_BORDER : style & ~WS_BORDER;
	SetWindowLong(h, GWL_STYLE, style);
	SetWindowPos(h, 0, 0, 0, 0, 0, SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
	return 0;
}

OBJECT_MEMBERS(Camera)
	METHOD(Camera, snapshot)
  READWRITE_PROPERTY(Camera, aspect)
  READONLY_PROPERTY(Camera, recording)
  READONLY_PROPERTY(Camera, record)
END

OBJECT_METAFIELDS(Camera)
  METHOD(Camera, __gc)
END

OBJECT_MEMBERS(Camera_widget)
  READWRITE_PROPERTY(Camera, border)
END