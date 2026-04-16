/*
 | xml for LuaRT
 | Luart.org, Copyright (c) Tine Samir 2026.
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Node.cpp | LuaRT XML Node Object
*/

#include <luart.h>
#include <windows.h>
#include <msxml6.h>
#include <comdef.h>
#include "Node.h"

//--- Node type
luart_type TNode;

#import "msxml6.dll" named_guids
using namespace MSXML2;

//----------------------------------- Node constructor
LUA_CONSTRUCTOR(Node)
{   
  int nargs = lua_gettop(L);
  wchar_t *name = NULL, *content = NULL;
  
  Node *n = new Node {0};
  if (nargs > 1) {
    switch(lua_type(L, 2)) {
      case LUA_TSTRING:         name = lua_towstring(L, 2);
                                break;
      case LUA_TLIGHTUSERDATA:  {
        auto* ptr = (std::shared_ptr<XmlNode>*)lua_touserdata(L, 2);
                                  n->xmlnode = *ptr; 
                                  delete ptr;
                                  break;
                                }
      default:                  luaL_checkstring(L, 2);
    }  
  }

  if (nargs > 2)
    content = lua_towstring(L, 3);
   
  if (!n->xmlnode)
    n->xmlnode = std::make_shared<XmlNode>(name);
    if (content)
    n->xmlnode->text = content;
    free(content);
    free(name);
    lua_newinstance(L, n, Node);
    return 1;
}

void lua_pushNode(lua_State *L, XmlNode *node) {
  auto* shared = new std::shared_ptr<XmlNode>(node); 
  lua_pushlightuserdata(L, shared);  
  lua_pushinstance(L, Node, 1);
}

void lua_pushNode(lua_State *L, std::shared_ptr<XmlNode> node) {
  auto* shared = new std::shared_ptr<XmlNode>(node); 
  lua_pushlightuserdata(L, shared);  
  lua_pushinstance(L, Node, 1);
  lua_remove(L, -2);
}

//----------------------------------- Node.add()
LUA_METHOD(Node, add) {
  Node *n = lua_self(L, 1, Node);
  wchar_t *name = lua_towstring(L, 2);
  wchar_t *content = lua_towstring(L, 3);
  std::shared_ptr<XmlNode> newchild = std::make_shared<XmlNode>(name);

  n->xmlnode->AddChild(newchild);
  newchild->text = content;
  free(name);
  free(content);
  lua_pushNode(L, newchild);
  return 1;
}

//----------------------------------- Node.text property
LUA_PROPERTY_GET(Node, text) {
  Node *n = lua_self(L, 1, Node);
  lua_pushwstring(L, n->xmlnode->text.c_str());
  return 1;
}

LUA_PROPERTY_SET(Node, text) {
  Node *n = lua_self(L, 1, Node);
  wchar_t *text = lua_towstring(L, 2);
  
  n->xmlnode->text = text;
  free(text);
  return 1;
}

//----------------------------------- Node.name property
LUA_PROPERTY_GET(Node, name) {
  Node *n = lua_self(L, 1, Node);
  lua_pushwstring(L, n->xmlnode->name.c_str());
  return 1;
}

LUA_PROPERTY_SET(Node, name) {
  Node *n = lua_self(L, 1, Node);
  wchar_t *name = lua_towstring(L, 2);
  n->xmlnode->name = name;
  free(name);
  return 0;
}

//----------------------------------- Node.parent property
LUA_PROPERTY_GET(Node, parent) {
  Node *n = lua_self(L, 1, Node);
  if (!n->xmlnode->parent.expired()) {
    std::shared_ptr<XmlNode> parent = n->xmlnode->parent.lock();
    lua_pushNode(L, parent); 
  } else
    lua_pushnil(L);
  return 1;
}

LUA_PROPERTY_SET(Node, parent) {
  Node *n = lua_self(L, 1, Node);
  n->xmlnode->SetParent(lua_isnil(L, 2) ? nullptr : luaL_checkcinstance(L, 2, Node)->xmlnode);
  return 0;
}

//----------------------------------- Node.isleaf property
LUA_PROPERTY_GET(Node, isleaf) {
  lua_pushboolean(L, lua_self(L, 1, Node)->xmlnode->children.empty());
  return 1;
}

//----------------------------------- Node.attributes property
LUA_METHOD(attributes, index) {
  Node *n;
  wchar_t *attrib = lua_towstring(L, 2);
  const wchar_t *value;
  
  luaL_getmetafield(L, 1, "node");
  n = lua_self(L, -1, Node);
  if ((value = n->xmlnode->GetAttribute(attrib)))
    lua_pushwstring(L, value);
  else lua_pushnil(L);
  free(attrib);
  return 1;
}

LUA_METHOD(attributes, newindex) {
  Node *n;
  wchar_t *attrib = lua_towstring(L, 2);
  wchar_t *value = lua_isnil(L, 3) ? NULL : lua_towstring(L, 3);
  
  luaL_getmetafield(L, 1, "node");
  n = lua_self(L, -1, Node);
  n->xmlnode->SetAttribute(attrib, value);
  free(value);
  free(attrib);
  return 1;
}

LUA_PROPERTY_GET(Node, attributes) {
  lua_createtable(L, 0, 0);
  lua_createtable(L, 0, 3);
  lua_pushcfunction(L, attributes_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, attributes_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushvalue(L, 1);
  lua_setfield(L, -2, "node");
  lua_setmetatable(L, -2);
	return 1;
}

//----------------------------------- Node:clone() method
LUA_METHOD(Node, clone) {
  lua_pushNode(L, lua_self(L, 1, Node)->xmlnode->Clone());
  return 1;
}

//----------------------------------- Node:remove() method
LUA_METHOD(Node, remove) {
  Node *n = lua_self(L, 1, Node);
  auto parentPtr = n->xmlnode->parent.lock();
  
  if (!parentPtr) {
    lua_pushboolean(L, false); 
    return 1;
  }
  
  auto &siblings = parentPtr->children;
  auto it = std::find_if(siblings.begin(), siblings.end(),
                         [&](const std::shared_ptr<XmlNode>& child) {
                           return child.get() == n->xmlnode.get();
                          });
                          if (it != siblings.end()) {
                            siblings.erase(it);
    n->xmlnode->SetParent(nullptr); 
    lua_pushboolean(L, true);
  } else
    lua_pushboolean(L, false);
  return 1;
}

//----------------------------------- Node:query() method
LUA_METHOD(Node, query) {
  Node *n = lua_self(L, 1, Node);
  wchar_t *xpath = lua_towstring(L, 2);

  auto results = n->xmlnode->QueryXPath(xpath);
  if (results.empty())
    lua_pushnil(L);
  else {
    lua_createtable(L, static_cast<int>(results.size()), 0);
    for (size_t i = 0; i < results.size(); ++i) {
        lua_pushNode(L, results[i]);
        lua_rawseti(L, -2, i+1);
    }
  }
  free(xpath);
  return 1;
}

//----------------------------------- Node destructor
LUA_METHOD(Node, __gc) {
  Node *n = lua_self(L, 1, Node);
  delete n;
  return 0;
}

//---------------------------------- Node indexing
LUA_METHOD(Node, __metaindex) {
  Node *n = lua_self(L, 1, Node);
  int isnum, idx;

  if ((idx = lua_tointegerx(L, 2, &isnum)) || isnum) {
    idx = lua_tointeger(L, 2);

    if (idx < 1 || idx > n->xmlnode->Count())
      luaL_error(L, "Out of bound Node index");
    lua_pushNode(L, n->xmlnode->children[idx - 1]);
    return 1;
  }
  return 0;
}

//----------------------------------- Node __tostring metafield
LUA_METHOD(Node, __tostring) {
  Node *n = lua_self(L, 1, Node);
  lua_pushwstring(L, n->xmlnode->AsString().c_str());
  return 1;
}

//----------------------------------- Node __len metafield
LUA_METHOD(Node, __len) {
  Node *n = lua_self(L, 1, Node);
  lua_pushinteger(L, n->xmlnode->Count());
  return 1;
}

//----------------------------------- Node iteration
static int Element_iter(lua_State *L) {
	Node *n = (Node *)lua_touserdata(L, lua_upvalueindex(1));
	XmlNode::Iterator *it = (XmlNode::Iterator *)lua_touserdata(L, lua_upvalueindex(2));
	
  if (*it != n->xmlnode->end()) {
      lua_pushNode(L, (**it));
      ++(*it);
      return 1;
 	}
  delete it;
	return 0;	
}

LUA_METHOD(Node,__iterate) {
  Node *n = lua_self(L, 1, Node);
	lua_pushlightuserdata(L, n);
	lua_pushlightuserdata(L, new XmlNode::Iterator(n->xmlnode->begin()));
	lua_pushcclosure(L, Element_iter, 2);
	return 1;	
}

OBJECT_MEMBERS(Node)
  METHOD(Node, add)
  METHOD(Node, remove)
  METHOD(Node, clone)
  METHOD(Node, query)
  READWRITE_PROPERTY(Node, name)
  READWRITE_PROPERTY(Node, text)
  READWRITE_PROPERTY(Node, parent)
  READONLY_PROPERTY(Node, attributes)
  READONLY_PROPERTY(Node, isleaf)
END

OBJECT_METAFIELDS(Node)
  METHOD(Node, __gc)
  METHOD(Node, __len)
  METHOD(Node, __tostring)
  METHOD(Node, __iterate)
  METHOD(Node, __metaindex)
END