#include <yaml-cpp/yaml.h>
#include <luart.h>
#include <File.h>
#include <math.h>
#include <regex>
#include <string>
#include <vector>

#include <regex>

std::string preprocess_placeholders(const std::string& yaml_src) {
    std::regex unquoted_placeholder(R"((^|\n)([ \t]*[\w\-]+:[ \t]*)(\{\{\s*[\w_]+\s*\}\}))");
    return std::regex_replace(yaml_src, unquoted_placeholder, "$1$2\"$3\"");
}

static std::string substitute_placeholders(const std::string &input, lua_State *L, int vars_index, const std::vector<std::pair<std::string, std::string>> &locals) {
    std::string result;
    std::regex placeholder_regex(R"(\{\{\s*([\w_]+)\s*\}\})");
    std::smatch match;
    std::string::const_iterator search_start(input.cbegin());
    while (std::regex_search(search_start, input.cend(), match, placeholder_regex)) {
        result.append(search_start, match.prefix().first);
        std::string var_name = match[1].str();
        bool found = false;

        // Case 1: use explicit table
        if (vars_index && lua_istable(L, vars_index)) {
            lua_getfield(L, vars_index, var_name.c_str());
            if (lua_isstring(L, -1)) {
                result.append(lua_tostring(L, -1));
                found = true;
            }
            lua_pop(L, 1);
        } else {
            // Case 2: try locals
            for (const auto &local : locals) {
                if (var_name == local.first) {
                    result.append(local.second);
                    found = true;
                    break;
                }
            }

            // Case 3: try function upvalue
            if (!found) {
                lua_Debug ar;
                if (lua_getstack(L, 1, &ar) && lua_getinfo(L, "f", &ar)) {
                    lua_getupvalue(L, -1, 1);
                    if (lua_istable(L, -1)) {
                        lua_getfield(L, -1, var_name.c_str());
                        if (lua_isstring(L, -1)) {
                            result.append(lua_tostring(L, -1));
                            found = true;
                        }
                        lua_pop(L, 1);
                    }
                    lua_pop(L, 2);
                }
            }

            // Case 4: fallback to global
            if (!found) {
                lua_getglobal(L, var_name.c_str());
                if (lua_isstring(L, -1)) {
                    result.append(lua_tostring(L, -1));
                    found = true;
                }
                lua_pop(L, 1);
            }
        }

        // Unresolved: fallback to "null"
        if (!found)
            result.append("null");

        search_start = match[0].second;
    }

    result.append(search_start, input.cend());
    return result;
}

static void process_node_placeholders(YAML::Node &node, lua_State *L, int vars_index, const std::vector<std::pair<std::string, std::string>> &locals) {
    switch (node.Type()) {
        case YAML::NodeType::Scalar:
            node = substitute_placeholders(node.as<std::string>(), L, vars_index, locals);
            break;
        case YAML::NodeType::Sequence:
            for (auto& item : node)
                process_node_placeholders(item, L, vars_index, locals);
            break;
        case YAML::NodeType::Map:
            for (auto it = node.begin(); it != node.end(); ++it) {
                process_node_placeholders(it->first, L, vars_index, locals);
                process_node_placeholders(it->second, L, vars_index, locals);
            }
            break;
        default:
            break;
    }
}

static void LuaToYaml(lua_State *L, YAML::Node &node, int index) {
    switch (lua_type(L, index)) {
        case LUA_TNIL:
            node = YAML::Null;
            break;
        case LUA_TNUMBER:
            node = lua_tonumber(L, index);
            break;
        case LUA_TBOOLEAN:
            node = static_cast<bool>(lua_toboolean(L, index));
            break;
        case LUA_TSTRING: {
            const char *str = lua_tostring(L, index);
            if (strcmp(str, "null") == 0) 
                node = YAML::Null;
            else
                node = str;
            break;
        }
        case LUA_TTABLE: {
            lua_len(L, index);
            lua_Integer len = lua_tointeger(L, -1);
            lua_pop(L, 1);
            if (len > 0) {
                node = YAML::Node(YAML::NodeType::Sequence);
                for (lua_Integer i = 1; i <= len; ++i) {
                    lua_rawgeti(L, index, i);
                    YAML::Node child;
                    LuaToYaml(L, child, -1);
                    node.push_back(child);
                    lua_pop(L, 1);
                }
            } else {
                node = YAML::Node(YAML::NodeType::Map);
                lua_pushvalue(L, index);
                int table_index = lua_gettop(L);
                lua_pushnil(L);
                while (lua_next(L, table_index)) {
                    if (lua_type(L, -2) == LUA_TSTRING) {
                        YAML::Node child;
                        LuaToYaml(L, child, -1);
                        node[lua_tostring(L, -2)] = child;
                    }
                    lua_pop(L, 1);
                }
                lua_pop(L, 1);
            }
            break;
        }
        default:
            luaL_error(L, "unsupported '%s' type for YAML encoding", luaL_typename(L,index));
    }
}

static void YamlToLua(lua_State *L, const YAML::Node &node) {
    switch (node.Type()) {
        case YAML::NodeType::Null:
            lua_pushstring(L, "null");
            break;
        case YAML::NodeType::Scalar: {
            std::string scalar;
            try {
                scalar = node.as<std::string>();
            } catch (...) {
                lua_pushstring(L, "");
                break;
            }
            if (scalar == "null") {
                lua_pushstring(L, "null");
            } else {
                try {
                    size_t pos;
                    double num = std::stod(scalar, &pos);
                    if (pos == scalar.length()) {
                        if (fmod(num, 1.0) == 0.0)
                            lua_pushinteger(L, static_cast<lua_Integer>(num));
                        else
                            lua_pushnumber(L, num);
                        break;
                    }
                } catch (...) {}
                if (scalar == "true")
                    lua_pushboolean(L, 1);
                else if (scalar == "false")
                    lua_pushboolean(L, 0);
                else
                    lua_pushstring(L, scalar.c_str());
            }
            break;
        }
        case YAML::NodeType::Sequence: {
            lua_createtable(L, node.size(), 0);
            for (size_t i = 0; i < node.size(); ++i) {
                YamlToLua(L, node[i]);
                lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
            }
            break;
        }
        case YAML::NodeType::Map: {
            lua_createtable(L, 0, node.size());
            for (const auto &pair : node) {
                try {
                    std::string key = pair.first.as<std::string>();
                    lua_pushstring(L, key.c_str());
                    YamlToLua(L, pair.second);
                    lua_rawset(L, -3);
                } catch (...) { continue; }
            }
            break;
        }
        default:
            lua_pushstring(L, "null");
    }
}


static int decode(lua_State *L, const char *src, int vars_index) {
    std::vector<std::pair<std::string, std::string>> locals;

    bool use_table = vars_index && lua_istable(L, vars_index);
    if (!use_table) {
        lua_Debug ar;
        if (lua_getstack(L, 1, &ar)) {
            for (int i = 1; const char *name = lua_getlocal(L, &ar, i); ++i) {
                luaL_tolstring(L, -1, NULL);
                locals.emplace_back(name, lua_tostring(L, -1));
                lua_pop(L, 2);
            }
        }
    }

    try {
        std::string preprocessed_src = preprocess_placeholders(src);
        std::vector<YAML::Node> docs = YAML::LoadAll(preprocessed_src);
        if (docs.size() == 1) {
            process_node_placeholders(docs[0], L, vars_index, locals);
            YamlToLua(L, docs[0]);
            lua_pushnil(L);
            return 2;
        } else {
            lua_createtable(L, docs.size(), 0);
            for (size_t i = 0; i < docs.size(); ++i) {
                process_node_placeholders(docs[i], L, vars_index, locals);
                YamlToLua(L, docs[i]);
                lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
            }
            lua_pushnil(L);
            return 2;
        }
    } catch (const YAML::Exception &e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
    }
}

LUA_METHOD(yaml, decode) {
    const char *src = luaL_checkstring(L, 1);
    int vars_index = lua_gettop(L) >= 2 ? 2 : 0;
    return decode(L, src, vars_index);
}

LUA_METHOD(yaml, load) {
    int nargs = lua_gettop(L);
    wchar_t *fname = luaL_checkFilename(L, 1);
    FILE *f = _wfopen(fname, L"rb");
    if (!f) {
        lua_pushnil(L);
        lua_pushstring(L, "failed to open file");
        free(fname);
        return 2;
    }

    fseek(f, 0, SEEK_END);
    long long fsize = _ftelli64(f);
    fseek(f, 0, SEEK_SET);
    char *src = (char *)calloc(1, fsize + 1);
    if (!src) {
        fclose(f);
        free(fname);
        lua_pushnil(L);
        lua_pushstring(L, "memory allocation failed");
        return 2;
    }

    fread(src, fsize, 1, f);
    fclose(f);
    free(fname);

    int nres = decode(L, src, nargs > 1 ? 2 : 0);
    free(src);
    return nres;
}

LUA_METHOD(yaml, encode) {
    luaL_checktype(L, 1, LUA_TTABLE);
    YAML::Emitter emitter;
    emitter.SetNullFormat(YAML::LowerNull);

    lua_len(L, 1);
    lua_Integer len = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if (len > 0) {
        for (lua_Integer i = 1; i <= len; ++i) {
            if (i > 1)
                emitter << YAML::Newline << YAML::BeginDoc;
            else
                emitter << YAML::BeginDoc;

            lua_rawgeti(L, 1, i);
            YAML::Node doc;
            LuaToYaml(L, doc, -1);
            lua_pop(L, 1);
            emitter << doc;
        }
    } else {
        YAML::Node node;
        LuaToYaml(L, node, 1);
        emitter << node;
    }

    lua_pushstring(L, emitter.c_str());
    return 1;
}

LUA_METHOD(yaml, save) {
    wchar_t *fname = luaL_checkFilename(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    FILE *f = _wfopen(fname, L"wb");
    free(fname);

    if (!f) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "failed to open file");
        return 2;
    }

    lua_pushcfunction(L, yaml_encode);
    lua_pushvalue(L, 2);

    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        fclose(f);
        lua_pushboolean(L, false);
        lua_pushstring(L, lua_tostring(L, -1));
        return 2;
    }

    const char *output = lua_tostring(L, -1);
    size_t written = fwrite(output, 1, strlen(output), f);
    fclose(f);

    lua_pushboolean(L, written == strlen(output));
    lua_pushnil(L);
    return 2;
}

LUA_PROPERTY_GET(yaml, null) {
    lua_pushstring(L, "null");
    return 1;
}

MODULE_PROPERTIES(yaml)
    READONLY_PROPERTY(yaml, null)
END

MODULE_FUNCTIONS(yaml)
    METHOD(yaml, decode)
    METHOD(yaml, encode)
    METHOD(yaml, load)
    METHOD(yaml, save)
END

extern "C" __declspec(dllexport) int luaopen_yaml(lua_State *L) {
    lua_regmodule(L, yaml);
    return 1;
}

