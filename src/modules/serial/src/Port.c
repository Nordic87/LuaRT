/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Port.c | LuaRT Port object implementation
*/

#include <luart.h>
#include <windows.h>

#define LUA_LIB

#include "Port.h"
#include <Buffer.h>
#include <Task.h>

luart_type TPort;

static const char *parity_modes[] = { "none", "odd", "even", "mark", "space", NULL };
static const char *stopbits_modes[] = { "one", "one5", "two", NULL };
static const char *rts_modes[] = { "off", "on", "handshake", "toggle", NULL };
static const char *dtr_modes[] = { "disabled", "enabled", "handshake", NULL };

static LUA_METHOD(PortTask, gc) {
    PortTask *task = (PortTask*)(lua_self(L, 1, Task)->userdata);
    if (task->ov.hEvent) CloseHandle(task->ov.hEvent);
    if (task->line && !task->iswrite) free(task->line);
    free(task);
    return 0;
}

//-------------------------------------[ Port Constructor ]
LUA_CONSTRUCTOR(Port) {
    Port *c = (Port*)calloc(1, sizeof(Port));
    
    c->port = luaL_checkstring(L, 2);
    lua_newinstance(L, c, Port);
    return 1;
}

//-------------------------------------[ Port.open() ]
LUA_METHOD(Port, open) {
    Port *c = lua_self(L, 1, Port);
    BOOL result = TRUE;
    
    if (c->status || (c->hSerial = CreateFileA(c->port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
        result = FALSE;
    else {
        GetCommState(c->hSerial, &c->dcb);
        if (lua_istable(L, 2)) {
            lua_getfield(L, 2, "rts");
            c->dcb.fRtsControl = lua_optstring(L, -1, rts_modes, 0);
            lua_getfield(L, 2, "dtr");
            c->dcb.fDtrControl = lua_optstring(L, -1, dtr_modes, 0);
            lua_getfield(L, 2, "baudrate");
            c->dcb.BaudRate = luaL_optinteger(L, -1, CBR_9600);
            lua_getfield(L, 2, "bytesize");
            c->dcb.ByteSize = luaL_optinteger(L, -1, 8);
            lua_getfield(L, 2, "parity");
            c->dcb.Parity = lua_optstring(L, -1, parity_modes, NOPARITY);
            lua_getfield(L, 2, "stopbits");
            c->dcb.StopBits = lua_optstring(L, -1, stopbits_modes, ONESTOPBIT); 
            SetCommState(c->hSerial, &c->dcb);            
        } 
    }
    c->status = result;
    lua_pushboolean(L, result);
    return 1;
}

static int PortTaskContinue(lua_State *L, int status, lua_KContext ctx) {
    PortTask *task = (PortTask *)ctx;
    DWORD bytes;
    DWORD err;

    if (task->port->status != portOpen) {
        SetLastError(ERROR_ACCESS_DENIED);
        goto error;
    }
    if (task->failed)
        goto error;
    if (GetOverlappedResult(task->port->hSerial, &task->ov, &bytes, FALSE)) {
        if (GetLastError() == ERROR_IO_INCOMPLETE)
            goto error;
        ResetEvent(task->ov.hEvent);
        if (!bytes && task->bytesToRead) {
read:       if (!ReadFile(task->port->hSerial, task->line+task->line_len, task->bytesToRead, NULL, &task->ov)) {
                if (GetLastError() != ERROR_IO_PENDING)
                    task->failed = TRUE;
                goto yield;
            }
        }
        if (task->bytesToRead) {
            task->bytesToRead -= bytes;
            task->line_len += bytes;
            if (task->bytesToRead == 0) {
                lua_pushlstring(L, task->line, task->line_len);
                lua_pushinstance(L, Buffer, 1);
                if (!task->iswrite) {
                    free(task->line);
                    task->line = NULL;
                }
            } else goto read;
        } else lua_pushinteger(L, bytes);
    } else if ((err = GetLastError()) == ERROR_IO_PENDING || (err == ERROR_IO_INCOMPLETE)) {
yield:   return lua_yieldk(L, 0, ctx, PortTaskContinue);
    } else {
error:
        if (task->line && !task->iswrite) {
            free(task->line);
            task->line = NULL;
        }
        lua_pushboolean(L, FALSE);
    }
    return 1;
}

static int PortReadlineContinue(lua_State *L, int status, lua_KContext ctx) {
    PortTask *task = (PortTask *)ctx;
    DWORD bytes;
    DWORD err;
    const size_t MAX_CAPACITY = 16 * 1024 * 1024; 

    if (task->port->status != portOpen) {
        SetLastError(ERROR_ACCESS_DENIED);
        goto error;
    }
    if (task->failed)
        goto error;
    if (GetOverlappedResult(task->port->hSerial, &task->ov, &bytes, FALSE)) {
        if (GetLastError() == ERROR_IO_INCOMPLETE)
            goto error;
        ResetEvent(task->ov.hEvent);
        if (bytes == 0)
            goto read;
        if (task->line_len + bytes > task->line_capacity) {
            size_t new_capacity = task->line_capacity * 2;
            if (new_capacity > MAX_CAPACITY) {
                task->failed = TRUE;
                goto error;
            }
            char *new_line = (char *)realloc(task->line, new_capacity);
            if (!new_line) {
                task->failed = TRUE;
                goto error;
            }
            task->line = new_line;
            task->line_capacity = new_capacity;
        }
redo:
        task->line_len += bytes;
        if (task->line_len >= task->eol_len) {
            char *eol_pos = task->line;
            size_t remaining = task->line_len;
            while (remaining >= task->eol_len) {
                if (memcmp(eol_pos, task->eol, task->eol_len) == 0) {
                    lua_pushlstring(L, task->line, eol_pos - task->line);
                    lua_pushinstance(L, Buffer, 1);
                    free(task->line);
                    task->line = NULL;
                    goto done;
                }
                eol_pos++;
                remaining--;
            }
        }
read:   if (!ReadFile(task->port->hSerial, task->line+task->line_len, 1, &bytes, &task->ov)) {
            if (GetLastError() != ERROR_IO_PENDING)
                task->failed = TRUE;
        } else goto redo;
        goto loop;
    } else if ((err = GetLastError()) == ERROR_IO_PENDING || (err == ERROR_IO_INCOMPLETE))
loop:
        return lua_yieldk(L, 0, ctx, PortReadlineContinue);
    else {
error:
        if (task->line) {
            free(task->line);
            task->line = NULL;
        }
        lua_pushboolean(L, FALSE);
    }
done:
    return 1;
}

//-------------------------------------[ Port.read() ]
LUA_METHOD(Port, read) {
    Port *c = lua_self(L, 1, Port);
    DWORD bytesToRead = luaL_checkinteger(L, 2);
    PortTask *task = (PortTask *)calloc(1, sizeof(PortTask));

    task->port = c;
    task->bytesToRead = bytesToRead;
    task->line = (char *)calloc(1, bytesToRead);
    task->ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    lua_pushboolean(L, FALSE);
    task->failed = task->port->status != portOpen;
    
    if (task->failed || !ReadFile(c->hSerial, task->line, task->bytesToRead, NULL, &task->ov)) {
        if (GetLastError() != ERROR_IO_PENDING)
            task->failed = TRUE;
    }
    lua_pushtask(L, PortTaskContinue, task, PortTask_gc);
    return 1;
}

//-------------------------------------[ Port.readline() ]
LUA_METHOD(Port, readline) {
    Port *c = lua_self(L, 1, Port);
    PortTask *task = (PortTask *)calloc(1, sizeof(PortTask));
    if (!task) {
        lua_pushboolean(L, FALSE);
        return 1;
    }
    task->port = c;
    task->line_capacity = luaL_optinteger(L, 3, 512);
    if (task->line_capacity <= 0 || task->line_capacity > 16*1024*1024) {
        free(task);
        luaL_error(L, "capacity be between 1 and 16MB");
    }
    task->eol = luaL_optlstring(L, 2, "\n", &task->eol_len);
    task->line = (char *)calloc(1, task->line_capacity);
    if (!task->line) {
        free(task);
        lua_pushboolean(L, FALSE);
        return 1;
    }
    task->bytesToRead = task->line_capacity;
    task->ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!task->ov.hEvent) {
        free(task->line);
        free(task);
        lua_pushboolean(L, FALSE);
        return 1;
    }
    lua_pushboolean(L, FALSE);
    task->failed = task->port->status != portOpen;
    if (task->failed || !ReadFile(c->hSerial, task->line, 1, NULL, &task->ov)) {
        if (GetLastError() != ERROR_IO_PENDING)
            task->failed = TRUE;
    }
    lua_pushtask(L, PortReadlineContinue, task, PortTask_gc);
    return 1;
}

//-------------------------------------[ Port.write() ]
LUA_METHOD(Port, write) {
    Port *c = lua_self(L, 1, Port);
    size_t len;
    PortTask *task = (PortTask *)calloc(1, sizeof(PortTask));
    
    task->port = c;
    task->line = (char *)luaL_tolstring(L, 2, &len);
    task->ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    task->failed = task->port->status != portOpen;
    task->iswrite = TRUE;
    
    if (task->failed || !WriteFile(c->hSerial, task->line, len, NULL, &task->ov)) {
        if (GetLastError() != ERROR_IO_PENDING)
            task->failed = TRUE;
    }
    lua_pushtask(L, PortTaskContinue, task, PortTask_gc);
    return 1;
}

//-------------------------------------[ Port.flush() ]
LUA_METHOD(Port, flush) {
    FlushFileBuffers(lua_self(L, 1, Port)->hSerial);
    return 0;
}

//-------------------------------------[ Port.close() ]
LUA_METHOD(Port, close) {
    Port *c = lua_self(L, 1, Port);
    if (c->status == portOpen) {
        CloseHandle(c->hSerial);
        c->status = portClosed;
        c->hSerial = NULL;
    }
    return 0;
}

//-------------------------------------[ Port.isopen ]
LUA_PROPERTY_GET(Port, isopen) {
    lua_pushboolean(L, lua_self(L, 1, Port)->hSerial > 0);
    return 1;
}

//-------------------------------------[ Port.rts ]
LUA_PROPERTY_SET(Port, rts) {
    Port *c = lua_self(L, 1, Port);
    if (c->status && GetCommState(c->hSerial, &c->dcb)) {
        c->dcb.fRtsControl = luaL_checkoption(L, 2, "off", rts_modes); 
        SetCommState(c->hSerial, &c->dcb);
    }
    return 0;
}

LUA_PROPERTY_GET(Port, rts) {
    Port *c = lua_self(L, 1, Port);
    
    if (c->status && GetCommState(c->hSerial, &c->dcb))
        lua_pushstring(L, rts_modes[c->dcb.fRtsControl]);
    else lua_pushnil(L);
    return 1;
}

//-------------------------------------[ Port.dtr ]
LUA_PROPERTY_SET(Port, dtr) {
    Port *c = lua_self(L, 1, Port);
    if (c->status && GetCommState(c->hSerial, &c->dcb)) {
        c->dcb.fDtrControl = luaL_checkoption(L, 2, "disabled", dtr_modes); 
        SetCommState(c->hSerial, &c->dcb);
    } else lua_pushnil(L);
    return 0;
}

LUA_PROPERTY_GET(Port, dtr) {
    Port *c = lua_self(L, 1, Port);
    
    if (c->status && GetCommState(c->hSerial, &c->dcb))
        lua_pushstring(L, dtr_modes[c->dcb.fDtrControl]);
    else lua_pushnil(L);
    return 1;
}

//-------------------------------------[ Port.baudrate ]
LUA_PROPERTY_SET(Port, baudrate) {
    Port *c = lua_self(L, 1, Port);
    int baudRate = luaL_checkinteger(L, 2); 
    if (baudRate <= 0)
        luaL_error(L, "invalid baudrate value");
    if (c->status && GetCommState(c->hSerial, &c->dcb)) {
        c->dcb.BaudRate = baudRate;
        SetCommState(c->hSerial, &c->dcb);
    }
    return 0;
}

LUA_PROPERTY_GET(Port, baudrate) {
    Port *c = lua_self(L, 1, Port);
    
    if (c->status && GetCommState(c->hSerial, &c->dcb))
        lua_pushinteger(L, c->dcb.BaudRate);
    else lua_pushnil(L);
    return 1;
}

//-------------------------------------[ Port.bytesize ]
LUA_PROPERTY_SET(Port, bytesize) {
    Port *c = lua_self(L, 1, Port);
    int byteSize = luaL_checkinteger(L, 2); 
    if (byteSize < 5 || byteSize > 8)
        luaL_error(L, "bytesize must be between 5 and 8");
    if (c->status && GetCommState(c->hSerial, &c->dcb)) {
        c->dcb.ByteSize = byteSize;
        SetCommState(c->hSerial, &c->dcb);
    }
    return 0;
}

LUA_PROPERTY_GET(Port, bytesize) {
    Port *c = lua_self(L, 1, Port);
    
    if (c->status && GetCommState(c->hSerial, &c->dcb))
        lua_pushinteger(L, c->dcb.ByteSize);
    else lua_pushnil(L);
    return 1;
}

//-------------------------------------[ Port.parity ]
LUA_PROPERTY_SET(Port, parity) {
    Port *c = lua_self(L, 1, Port);
    if (c->status && GetCommState(c->hSerial, &c->dcb)) {
        c->dcb.Parity = luaL_checkoption(L, 2, "none", parity_modes); 
        SetCommState(c->hSerial, &c->dcb);
    }
    return 0;
}

LUA_PROPERTY_GET(Port, parity) {
    Port *c = lua_self(L, 1, Port);
    
    if (c->status && GetCommState(c->hSerial, &c->dcb))
        lua_pushstring(L, parity_modes[c->dcb.Parity]);
    else lua_pushnil(L);
    return 1;
}

//-------------------------------------[ Port.stopbits ]
LUA_PROPERTY_SET(Port, stopbits) {
    Port *c = lua_self(L, 1, Port);
    if (c->status && GetCommState(c->hSerial, &c->dcb)) {
        c->dcb.StopBits = luaL_checkoption(L, 2, "one", stopbits_modes); 
        SetCommState(c->hSerial, &c->dcb);
    }
    return 0;
}

LUA_PROPERTY_GET(Port, stopbits) {
    Port *c = lua_self(L, 1, Port);
    
    if (c->status && GetCommState(c->hSerial, &c->dcb))
        lua_pushstring(L, stopbits_modes[c->dcb.StopBits]);
    else lua_pushnil(L);
    return 1;
}

//-------------------------------------[ Port.__gc() ]
LUA_METHOD(Port, __gc) {
    Port_close(L);
    free(lua_self(L, 1, Port));
    return 0;
}

OBJECT_MEMBERS(Port)
    METHOD(Port, open)
    METHOD(Port, read)
    METHOD(Port, readline)
    METHOD(Port, write)
    METHOD(Port, flush)
    METHOD(Port, close)
    READONLY_PROPERTY(Port, isopen)
    READWRITE_PROPERTY(Port, rts)
    READWRITE_PROPERTY(Port, dtr)
    READWRITE_PROPERTY(Port, baudrate)
    READWRITE_PROPERTY(Port, parity)
    READWRITE_PROPERTY(Port, bytesize)
    READWRITE_PROPERTY(Port, stopbits)
END

OBJECT_METAFIELDS(Port)
    METHOD(Port, __gc)
END