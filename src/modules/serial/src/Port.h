/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Port.h | LuaRT Port object header
*/

#pragma once

#include <luart.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum portStatus {
    portClosed = 0,
    portOpen
} portStatus;

//---------------------------------------- Port object
typedef struct {
	luart_type	type;
    HANDLE      hSerial;             
    portStatus  status;
    DCB         dcb;
    const char *port;
} Port;

typedef struct {
    Port        *port;             
    OVERLAPPED  ov;          
    DWORD       bytesToRead;     
    BOOL        failed;
    char        *line;             
    size_t      line_len;        
    size_t      line_capacity;   
    const char  *eol;        
    size_t      eol_len; 
    BOOL        iswrite;
} PortTask;

extern luart_type TPort;

LUA_CONSTRUCTOR(Port);
extern const luaL_Reg Port_methods[];
extern const luaL_Reg Port_metafields[];

#ifdef __cplusplus
}
#endif