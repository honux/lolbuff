/************************************************************************
  LuaScript Class
  Encapsulates a Lua script.
*************************************************************************/

#include "luascript.h"

#include <cassert>
#include <exception>
#include <iostream>

/************************************************************************
  LuaScript Class Public Methods
*************************************************************************/

utils::LuaScript::LuaScript (LuaPanicCallback panicCallback /*= nullptr*/)
: m_panic(panicCallback)
{
    m_L = luaL_newstate();
    if (!m_L)
    {
        throw std::bad_alloc();
    }
    luaL_openlibs(m_L);

// Set panic handler
    lua_pushstring(m_L, "LuaScript_this");
    lua_pushlightuserdata(m_L, this);
    lua_settable(m_L, LUA_REGISTRYINDEX);
    lua_atpanic(m_L, &_LuaPanic);
}

utils::LuaScript::~LuaScript ()
{
    if (m_L)
    {
        lua_close(m_L);
    }
}

bool utils::LuaScript::LoadFromFile (char const* fileName, bool run /*= true*/)
{
    int ret = luaL_loadfile(m_L, fileName);
    switch (ret)
    {
    case LUA_OK:
        if (run)
        {
            return Run();
        }
        return true;

    case LUA_ERRSYNTAX:
        printf("[ERROR] utils::LuaScript::LoadFromFile - Syntax error: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;

    case LUA_ERRMEM:
    case LUA_ERRGCMM:
        printf("[ERROR] utils::LuaScript::LoadFromFile - Memory allocation failure: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;

    case LUA_ERRFILE:
        printf("[ERROR] utils::LuaScript::LoadFromFile - Unable to read from file: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;
    }

    printf("[ERROR] utils::LuaScript::LoadFromFile - Unknown error: %s\n", lua_tostring(m_L, -1));
    lua_pop(m_L, 1);
    return false;
}

bool utils::LuaScript::LoadFromString (char const* code, bool run /*= true*/)
{
    int ret = luaL_loadstring(m_L, code);
    switch (ret)
    {
    case LUA_OK:
        if (run)
        {
            return Run();
        }
        return true;

    case LUA_ERRSYNTAX:
        printf("[ERROR] utils::LuaScript::LoadFromString - Syntax error: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;

    case LUA_ERRMEM:
    case LUA_ERRGCMM:
        printf("[ERROR] utils::LuaScript::LoadFromString - Memory allocation failure: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;
    }

    printf("[ERROR] utils::LuaScript::LoadFromString - Unknown error: %s\n", lua_tostring(m_L, -1));
    lua_pop(m_L, 1);
    return false;
}

bool utils::LuaScript::LoadFromString (std::string const& code, bool run /*= true*/)
{
    int ret = luaL_loadbuffer(m_L, code.c_str(), code.length(), nullptr);
    switch (ret)
    {
    case LUA_OK:
        if (run)
        {
            return Run();
        }
        return true;

    case LUA_ERRSYNTAX:
        printf("[ERROR] utils::LuaScript::LoadFromString - Syntax error: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;

    case LUA_ERRMEM:
    case LUA_ERRGCMM:
        printf("[ERROR] utils::LuaScript::LoadFromString - Memory allocation failure: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;
    }

    printf("[ERROR] utils::LuaScript::LoadFromString - Unknown error: %s\n", lua_tostring(m_L, -1));
    lua_pop(m_L, 1);
    return false;
}

void utils::LuaScript::DumpStack () const
{
    int top = lua_gettop(m_L);
    for (int i = 1; i <= top; i++) /* repeat for each level */
    {
        int t = lua_type(m_L, i);
        switch (t)
        {
        case LUA_TSTRING: /* strings */
            printf("'%s'", lua_tostring(m_L, i));
            break;
        
        case LUA_TBOOLEAN: /* booleans */
            printf(lua_toboolean(m_L, i) ? "true" : "false");
            break;
        
        case LUA_TNUMBER: /* numbers */
            printf("%g", lua_tonumber(m_L, i));
            break;
        
        default: /* other values */
            printf("%s", lua_typename(m_L, t));
            break;
        }
        printf(" "); /* put a separator */
    }
    printf("\n"); /* end the listing */
}

void utils::LuaScript::Reset ()
{
    if (m_L != nullptr)
    {
        lua_close(m_L);
    }

    m_L = luaL_newstate();
    if (!m_L)
    {
        throw std::bad_alloc();
    }

    luaL_openlibs(m_L);
}

bool utils::LuaScript::Run (int nargs, int nres /* = LUA_MULTRET */)
{
    int ret = lua_pcall(m_L, nargs, nres, 0);
    switch (ret)
    {
    case LUA_OK:
        return true;

    case LUA_ERRRUN:
        printf("[ERROR] utils::LuaScript::Run - Runtime error: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;

    case LUA_ERRMEM:
    case LUA_ERRGCMM:
        printf("[ERROR] utils::LuaScript::Run - Memory allocation failure: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;

    case LUA_ERRERR:
        printf("[ERROR] utils::LuaScript::Run - Message handler error: %s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;
    }

    printf("[ERROR] utils::LuaScript::Run - Unknown error: %s\n", lua_tostring(m_L, -1));
    lua_pop(m_L, 1);
    return false;
}

/************************************************************************
  LuaScript Class Private Methods
*************************************************************************/

int utils::LuaScript::_LuaPanic (lua_State* L)
{
    if (lua_checkstack(L, 1) == 0)
    {
        return 0;
    }

    lua_pushstring(L, "LuaScript_this");
    lua_rawget(L, LUA_REGISTRYINDEX);
    LuaScript* script = (LuaScript*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    char const* errmsg = lua_tostring(L, -1);
    printf("[ERROR] utils::LuaScript - PANIC - %s\n", errmsg);
    if (script->m_panic)
    {
        script->m_panic(errmsg);
    }
    return 0;
}
