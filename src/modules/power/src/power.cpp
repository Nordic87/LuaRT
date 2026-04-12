/*
 | power for LuaRT
 | Luart.org, Copyright (c) Tine Samir 2026.
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | power.cpp | LuaRT power module
*/

#include <luart.h>
#include <windows.h>
#include <powrprof.h>
#include <mutex>

// Low Battery Warning GUID
const GUID GUID_LOW_BATTERY_WARNING = 
{ 0xbcded951, 0x187b, 0x4d05, { 0xbc, 0xcc, 0xf7, 0xe5, 0x19, 0x60, 0xc2, 0x58 } };

//--- Power handles
HANDLE  hPowerRequest;
HPOWERNOTIFY hNotifyBattery; 
HPOWERNOTIFY hNotifySuspend;
HPOWERNOTIFY hNotify;
HWND hWnd;            
HANDLE hThread;       

int     onPlugged = LUA_NOREF;
int     onSuspend = LUA_NOREF;
int     onResume = LUA_NOREF;
int     onBatteryLow = LUA_NOREF;

std::mutex mtx; 
MSG msg;


//---------------------------------------------| power.battery property
LUA_PROPERTY_GET(power, battery) {
  SYSTEM_POWER_STATUS status;
  lua_createtable(L, 0, 3);
  if (GetSystemPowerStatus(&status) && status.BatteryFlag < 128) {
    lua_pushinteger(L, status.BatteryLifePercent);
    lua_setfield(L, -2, "percent");
    lua_pushboolean(L, status.BatteryFlag & 8);
    lua_setfield(L, -2, "charging");
    if (status.BatteryLifeTime != MAXDWORD) {
        lua_pushinteger(L, status.BatteryLifeTime);
        lua_setfield(L, -2, "lifetime");
    }
    lua_pushboolean(L, status.SystemStatusFlag);
    lua_setfield(L, -2, "saver");
    return 1;
  }
  return 0;
}

//---------------------------------------------| power.plugged property
LUA_PROPERTY_GET(power, plugged) {
    SYSTEM_POWER_STATUS status = {0}; 
    GetSystemPowerStatus(&status);
    lua_pushboolean(L, status.ACLineStatus == 1);
    return 1;
}

//---------------------------------------------| power.hibernate() function
LUA_METHOD(power, hibernate) {
    lua_pushboolean(L, SetSuspendState(TRUE, FALSE, FALSE));
    return 1;
}

//---------------------------------------------| power.sleep() function
LUA_METHOD(power, sleep) {
    lua_pushboolean(L, SetSuspendState(FALSE, FALSE, FALSE));
    return 1;
}  

//---------------------------------------------| power.hold() function
LUA_METHOD(power, hold) {
  lua_pushboolean(L, PowerSetRequest(hPowerRequest, PowerRequestExecutionRequired));
  return 1;
}

//---------------------------------------------| power.unhold() function
LUA_METHOD(power, unhold) {
  lua_pushboolean(L, PowerClearRequest(hPowerRequest, PowerRequestExecutionRequired));
  return 1;
}

//---------------------------------------------| power.onPlugged() event
LUA_PROPERTY_SET(power, onPlugged) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushvalue(L, 1);
    if (onPlugged != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, onPlugged);
    onPlugged = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

LUA_PROPERTY_GET(power, onPlugged) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, onPlugged);
    return 1;
}

//---------------------------------------------| power.onSuspend() event
LUA_PROPERTY_SET(power, onSuspend) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushvalue(L, 1);
    if (onSuspend != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, onSuspend);
        onSuspend = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

LUA_PROPERTY_GET(power, onSuspend) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, onSuspend);
    return 1;
}

//---------------------------------------------| power.display()
LUA_METHOD(power, displayoff) {
    PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
    return 0;
}  

//---------------------------------------------| power.onResume() event
LUA_PROPERTY_SET(power, onResume) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushvalue(L, 1);
    if (onResume != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, onResume);
        onResume = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

LUA_PROPERTY_GET(power, onResume) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, onResume);
    return 1;
}

//---------------------------------------------| power.onBatteryLow() event
LUA_PROPERTY_SET(power, onBatteryLow) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushvalue(L, 1);
    if (onBatteryLow != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, onBatteryLow);
    onBatteryLow = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

LUA_PROPERTY_GET(power, onBatteryLow) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, onBatteryLow);
    return 1;
}

//---------------------------------------------| power finalize() function
LUA_METHOD(power, finalize) {
  SendMessage(hWnd, WM_QUIT, 0, 0);
  CloseHandle(hPowerRequest);
  UnregisterPowerSettingNotification(hNotify);
  PowerUnregisterSuspendResumeNotification(hNotifySuspend);
  PowerUnregisterSuspendResumeNotification(hNotifyBattery);
  DestroyWindow(hWnd);
  return 0;
}

MODULE_PROPERTIES(power)
    READONLY_PROPERTY(power, battery)
    READONLY_PROPERTY(power, plugged)
    READWRITE_PROPERTY(power, onPlugged)
    READWRITE_PROPERTY(power, onSuspend)
    READWRITE_PROPERTY(power, onResume)
    READWRITE_PROPERTY(power, onBatteryLow)
END

MODULE_FUNCTIONS(power)
    METHOD(power, hold)
    METHOD(power, unhold)
    METHOD(power, hibernate)
    METHOD(power, sleep)
    METHOD(power, displayoff)
END

// Window procedure for power notifications
LRESULT CALLBACK WndProc(HWND hWnd, UINT _msg, WPARAM wParam, LPARAM lParam) {
    if ((_msg == WM_POWERBROADCAST)) {
        mtx.lock();
        msg = {hWnd, _msg, wParam, lParam, 0, {0, 0}};
        mtx.unlock();
        return FALSE;
    }
    return DefWindowProc(hWnd, _msg, wParam, lParam);
}

static const char *source[] = { "ac", "battery", "ups", NULL };

static int PowerTaskContinue(lua_State *L, int status, lua_KContext ctx) {
    MSG _msg =  {};
    uint64_t timeout = GetTickCount64();

    PeekMessage(&_msg, NULL, 0, 0, PM_REMOVE);
    mtx.lock();
    if (msg.message == WM_POWERBROADCAST) {
        switch (msg.wParam) {
            case PBT_POWERSETTINGCHANGE: {
                static bool first = false;
                POWERBROADCAST_SETTING* pbs = reinterpret_cast<POWERBROADCAST_SETTING*>(msg.lParam);
                msg = {0};
                if (first && (onPlugged != LUA_NOREF) && IsEqualGUID(pbs->PowerSetting, GUID_ACDC_POWER_SOURCE)) {
                    lua_rawgeti(L, LUA_REGISTRYINDEX, onPlugged);
                    lua_pushstring(L, source[pbs->Data[0]]);
                    lua_call(L, 1, 0);
                } else if (IsEqualGUID(pbs->PowerSetting, GUID_LOW_BATTERY_WARNING) && (onBatteryLow != LUA_NOREF)) {
                    lua_rawgeti(L, LUA_REGISTRYINDEX, onBatteryLow);
                    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
                        lua_error(L);
                }
                first = true;
            } break;
            case PBT_APMSUSPEND:
                if (onSuspend != LUA_NOREF) {
                    lua_rawgeti(L, LUA_REGISTRYINDEX, onSuspend);
                    if (lua_pcall(L, 0, 0, 0) != LUA_OK);
                        lua_error(L);
                } break;
            case PBT_APMRESUMESUSPEND:
                if (onResume != LUA_NOREF) {
                    lua_rawgeti(L, LUA_REGISTRYINDEX, onResume);
                    if (lua_pcall(L, 0, 0, 0) != LUA_OK);
                        lua_error(L);
                } break;
        }
    }
    mtx.unlock();
    return lua_yieldk(L, 0, ctx, PowerTaskContinue);
}

static ULONG CALLBACK PowerCallback(PVOID Context, ULONG Type, PVOID Setting) {
    switch (Type) {
        case PBT_APMSUSPEND:
            SendMessage(hWnd, WM_POWERBROADCAST, PBT_APMSUSPEND, 0);
            break;
        case PBT_APMRESUMESUSPEND:
        case PBT_APMRESUMEAUTOMATIC:
        case PBT_APMRESUMESTANDBY:
            SendMessage(hWnd, WM_POWERBROADCAST, PBT_APMRESUMESUSPEND, 0);
            break;
    }
    return ERROR_SUCCESS;
}

//----- "power" module registration function
extern "C" {
    int __declspec(dllexport) luaopen_power(lua_State *L)
    {
        DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS params = { 0 };
        params.Callback = PowerCallback;
        params.Context = nullptr;
        DWORD result = PowerRegisterSuspendResumeNotification(DEVICE_NOTIFY_CALLBACK, &params, &hNotifySuspend);
        REASON_CONTEXT reason = {0};
        WNDCLASSA wc = {0};
        reason.Version = POWER_REQUEST_CONTEXT_VERSION;
        reason.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
        reason.Reason.SimpleReasonString = const_cast<LPWSTR>(L"LuaRT Power Request");
        
        if (!(hPowerRequest = PowerCreateRequest(&reason))) {
            luaL_getlasterror(L, GetLastError());
            lua_error(L);
        } 

        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "LuaRTPowerWindow";
        RegisterClassA(&wc);
        hWnd = CreateWindowA("LuaRTPowerWindow", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, wc.hInstance, NULL);

        hNotify = RegisterPowerSettingNotification(hWnd, &GUID_ACDC_POWER_SOURCE, DEVICE_NOTIFY_WINDOW_HANDLE);
        hNotifyBattery= RegisterPowerSettingNotification(hWnd, &GUID_LOW_BATTERY_WARNING, DEVICE_NOTIFY_WINDOW_HANDLE);
        if (!hNotify) {
            power_finalize(L);
            luaL_error(L, "Failed to register power notification: %lu", GetLastError());
        }

        lua_pushtask(L, PowerTaskContinue, NULL, NULL);
        lua_regmodulefinalize(L, power);
        return 1;
    }
}