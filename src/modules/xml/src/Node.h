/*
 | xml for LuaRT
 | Luart.org, Copyright (c) Tine Samir 2026.
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Element.h | LuaRT XML Element Object header file
*/

#pragma once

#include <luart.h>
#include "XMLNode.h"

//---------------------------------------- Element object
typedef struct {
	luart_type	                type;
    std::shared_ptr<XmlNode>    xmlnode;            
} Node;


extern luart_type TNode;
void lua_pushNode(lua_State *L, XmlNode *node);
void lua_pushNode(lua_State *L, std::shared_ptr<XmlNode> node);

LUA_CONSTRUCTOR(Node);
extern const luaL_Reg Node_methods[];
extern const luaL_Reg Node_metafields[];
