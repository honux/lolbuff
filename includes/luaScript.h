/********************************************************************//**
  @class utils::LuaScript
  Encapsulates a Lua script.
*************************************************************************/

#pragma once

#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

#include <string>

/************************************************************************
  Forward Declarations
*************************************************************************/
namespace utils
{
    class LuaScript;
}

/************************************************************************
  LuaScript Class Declaration
*************************************************************************/
class utils::LuaScript
{
    typedef void (*LuaPanicCallback)(char const*);

public:

    /// 
    /// Creates an empty LuaScript object.
    /// @param[in] panicCallback (Optional) Function to be called when Lua panics.
    /// It receives a string with the error message. Default is `nullptr`.
    /// @throws std::bad_alloc Lua state could not be created.
    /// @remarks Lua always calls abort() after executing the panic function.
    /// 
    LuaScript (LuaPanicCallback panicCallback = nullptr);

    /// 
    /// Destroys and unloads the script.
    ///
    ~LuaScript ();

    /// 
    /// Concatenates strings or number values on the top of the stack into a string.
    /// @param[in] count Number of values to be concatenated.
    /// @remarks The input values will be popped out from the stack and the resulting string will
    /// be left on the top.
    ///
    void ConcatenateValues (int count);
        
    /// 
    /// Dumps the stack contents to the log.
    ///
    void DumpStack () const;

    /// 
    /// Gets the Lua state of the script.
    /// @return The Lua state of the script.
    /// 
    lua_State* GetState () const;

    /// 
    /// Gets a boolean value from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] value Pointer to a boolean where the value is to be stored. Cannot be `nullptr`.
    /// @return true if the number was successfully retrieved, false otherwise.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalBoolean (char const* name, bool* value);

    /// 
    /// Gets a Lua integer from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] value Pointer to a Lua integer where the value is to be stored. Cannot be `nullptr`.
    /// @return true if the number was successfully retrieved, false otherwise.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalInteger (char const* name, lua_Integer* value);

    /// 
    /// Gets a Lua integer from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] value Pointer to a Lua integer where the value is to be stored. Cannot be `nullptr`.
    /// @param[in] defval Default value to be stored if the variable does not exist.
    /// @return true if the number was successfully retrieved, false if the number
    /// could not be read and the `defval` value was stored instead.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalInteger (char const* name, lua_Integer* value, lua_Integer defval);

    /// 
    /// Gets a Lua number from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] value Pointer to a Lua number where the value is to be stored. Cannot be `nullptr`.
    /// @return true if the number was successfully retrieved, false otherwise.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalNumber (char const* name, lua_Number* value);

    /// 
    /// Gets a Lua number from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] value Pointer to a Lua number where the value is to be stored. Cannot be `nullptr`.
    /// @param[in] defval Default value to be stored if the variable does not exist.
    /// @return true if the number was successfully retrieved, false if the number
    /// could not be read and the `defval` value was stored instead.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalNumber (char const* name, lua_Number* value, lua_Number defval);

    /// 
    /// Gets a string from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] string Pointer to a string object where the string is to be stored. Cannot be `nullptr`.
    /// @return true if the string was successfully retrieved, false otherwise.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalString (char const* name, std::string* string);

    /// 
    /// Gets a string from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] string Pointer to a string object where the string is to be stored. Cannot be `nullptr`.
    /// @param[in] defval Default value to be stored if the variable does not exist.
    /// @return true if the string was successfully retrieved, false if the string
    /// could not be read and the `defval` value was stored instead.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalString (char const* name, std::string* string, char const* defval);

    /// 
    /// Gets a Lua unsigned integer from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] value Pointer to a Lua unsigned integer where the value is to be stored. Cannot be `nullptr`.
    /// @return true if the number was successfully retrieved, false otherwise.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalUnsigned (char const* name, lua_Unsigned* value);

    /// 
    /// Gets a Lua unsigned integer from a global Lua variable.
    /// @param[in] name Name of the variable.
    /// @param[out] value Pointer to a Lua unsigned integer where the value is to be stored. Cannot be `nullptr`.
    /// @param[in] defval Default value to be stored if the variable does not exist.
    /// @return true if the number was successfully retrieved, false if the number
    /// could not be read and the `defval` value was stored instead.
    /// @remarks The stack state is kept the same after this call.
    ///
    bool GetGlobalUnsigned (char const* name, lua_Unsigned* value, lua_Unsigned defval);

    /// 
    /// Gets a boolean value from the stack.
    /// @param[out] value Pointer to a boolean variable where the value is to be stored. Cannot be `nullptr`.
    /// @param[in] index (Optional) Index on the stack. Default is -1, which corresponds to the top element.
    /// @returns true if the value was read successfully, false otherwise.
    ///
    bool GetBoolean (bool* value, int index = -1);

    /// 
    /// Gets a Lua integer from the stack.
    /// @param[out] value Pointer to a Lua integer variable where the value is to be stored. Cannot be `nullptr`.
    /// @param[in] index (Optional) Index on the stack. Default is -1, which corresponds to the top element.
    /// @returns true if the value was read successfully, false otherwise.
    ///
    bool GetInteger (lua_Integer* value, int index = -1);

    /// 
    /// Gets a data pointer from the stack.
    /// @param[out] data Pointer to a void* variable where the data is to be stored. Cannot be `nullptr`.
    /// @param[in] index (Optional) Index on the stack. Default is -1, which corresponds to the top element.
    /// @returns true if the value was read successfully, false otherwise.
    ///
    bool GetLightUserData (void** data, int index = -1);

    /// 
    /// Gets a Lua number from the stack.
    /// @param[out] value Pointer to a Lua number variable where the value is to be stored. Cannot be `nullptr`.
    /// @param[in] index (Optional) Index on the stack. Default is -1, which corresponds to the top element.
    /// @returns true if the value was read successfully, false otherwise.
    ///
    bool GetNumber (lua_Number* value, int index = -1);

    /// 
    /// Gets a null-terminated string from the stack.
    /// @param[out] string Pointer to a variable where the address of the null-terminated string is to be stored.
    /// Cannot be `nullptr`.
    /// @param[in] index (Optional) Index on the stack. Default is -1, which corresponds to the top element.
    /// @returns true if the value was read successfully, false otherwise.
    /// @remarks The string contents are valid while the value is kept on the stack.
    ///
    bool GetString (char const** string, int index = -1);

    /// 
    /// Gets a null-terminated string from the stack.
    /// @param[out] string Pointer to a variable where the address of the null-terminated string is to be stored.
    /// Cannot be `nullptr`.
    /// @param[out] len Length of the string (including null-termination char). Cannot be `nullptr`.
    /// @param[in] index (Optional) Index on the stack. Default is -1, which corresponds to the top element.
    /// @returns true if the value was read successfully, false otherwise.
    /// @remarks The string contents are valid while the value is kept on the Lua stack. The application cannot
    /// modify the contents of the string.
    ///
    bool GetString (char const** string, size_t* len, int index = -1);

    /// 
    /// Gets a string object from the stack.
    /// @param[out] string Pointer to a string variable where the value is to be stored. Cannot be `nullptr`.
    /// @param[in] index (Optional) Index on the stack. Default is -1, which corresponds to the top element.
    /// @returns true if the value was read successfully, false otherwise.
    ///
    bool GetString (std::string* string, int index = -1);

    /// 
    /// Gets a Lua unsigned integer from the stack.
    /// @param[out] value Pointer to a Lua unsigned variable where the value is to be stored. Cannot be `nullptr`.
    /// @param[in] index (Optional) Index on the stack. Default is -1, which corresponds to the top element.
    /// @returns true if the value was read successfully, false otherwise.
    ///
    bool GetUnsigned (lua_Unsigned* value, int index = -1);

    /// 
    /// Loads Lua code from a file into a function placed on the top of the stack.
    /// @param[in] fileName Path to the Lua script file.
    /// @param[in] run (Optional) Defines whether the loaded code should be run automatically 
    /// with no arguments. If set to false, you must call Run() to execute it. Default is true.
    /// @return true if the Lua code was successfully loaded and executed, false otherwise.
    /// 
    bool LoadFromFile (char const* fileName, bool run = true);

    /// 
    /// Loads Lua code from a string into a function placed on the top of the stack.
    /// @param[in] code String containing Lua code.
    /// @param[in] run (Optional) Defines whether the loaded code should be run automatically 
    /// with no arguments. If set to false, you must call Run() to execute it. Default is true.
    /// @return true if the Lua code was successfully loaded and executed, false otherwise.
    /// 
    bool LoadFromString (char const* code, bool run = true);

    /// 
    /// Loads Lua code from a string into a function placed on the top of the stack.
    /// @param[in] code String containing Lua code.
    /// @param[in] run (Optional) Defines whether the loaded code should be run automatically 
    /// with no arguments. If set to false, you must call Run() to execute it. Default is true.
    /// @return true if the Lua code was successfully loaded and executed, false otherwise.
    /// 
    bool LoadFromString (std::string const& code, bool run = true);

    /// 
    /// Pops the top values from the stack.
    /// @param[in] count Number of values to be popped out. Default is one.
    ///
    void Pop (int count = 1);

    /// 
    /// Pushes a boolean value onto the stack.
    /// @param[in] value Boolean value.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushBoolean (bool value);

    /// 
    /// Pushes a C function onto the stack.
    /// @param[in] function C function with lua_CFunction signature.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushCFunction (lua_CFunction function);

    /// 
    /// Pushes the value of a global Lua variable or function onto the stack.
    /// @param[in] name Name of the variable or function.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks If the variable or function with the given name does not exist,
    /// nil is pushed.
    /// The application shall pop this value from the stack after using it.
    ///
    bool PushGlobal (char const* name);

    /// 
    /// Pushes a Lua integer onto the stack.
    /// @param[in] value Lua integer.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushInteger (lua_Integer value);

    /// 
    /// Pushes a data pointer onto the stack.
    /// @param[in] data Pointer to user data.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushLightUserData (void* data);

    /// 
    /// Pushes a nil value onto the stack.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushNil ();

    /// 
    /// Pushes a Lua number onto the stack.
    /// @param[in] value Lua number.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushNumber (lua_Number value);

    /// 
    /// Pushes a null-terminated string onto the stack.
    /// @param[in] string Null-terminated string.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushString (char const* string);

    /// 
    /// Pushes a string with the specified length onto the stack.
    /// @param[in] string Pointer to the string data.
    /// @param[in] len Length of the string (not including null-termination char).
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushString (char const* string, size_t len);

    /// 
    /// Pushes a string object onto the stack.
    /// @param[in] string BaseString object.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushString (std::string const& string);

    /// 
    /// Pushes a Lua table field whose key is at the top of the stack.
    /// @param[in] raw (Optional) Indicates whether the value shall be accessed directly, 
    /// bypassing metamethods. Default is false.
    /// @return true if the operation completed successfully, false if a table 
    /// was not found at the correct stack position or there was not enough space
    /// on the stack.
    /// @remarks The key is automatically popped out from the stack when this
    /// call completes successfully, but the application shall pop the table field 
    /// from the stack after using it.
    ///
    bool PushTableField (bool raw = false);

    /// 
    /// Pushes a Lua table field with the given key.
    /// @param[in] name Key of the table field.
    /// @return true if the operation completed successfully, false if a table 
    /// was not found at the correct stack position or there was not enough space
    /// on the stack.
    /// @remarks The application shall pop the table field from the stack after using it.
    ///
    bool PushTableField (char const* key);

    /// 
    /// Pushes a Lua table field with the given integer key. Useful to access Lua arrays.
    /// @param[in] name Key of the table field.
    /// @return true if the operation completed successfully, false if a table 
    /// was not found at the correct stack position or there was not enough space
    /// on the stack.
    /// @remarks The metamethods are ignored to get the value (raw access).
    /// The application shall pop the table field from the stack after using it.
    ///
    bool PushTableField (int key);

    /// 
    /// Pushes a Lua unsigned integer onto the stack.
    /// @param[in] value Lua unsigned integer.
    /// @return true if the operation completed successfully, false if there was 
    /// not enough space on the stack.
    /// @remarks The application shall pop this value from the stack after using it.
    ///
    bool PushUnsigned (lua_Unsigned value);

    /// 
    /// Reserves the specified number of slots on the stack.
    /// @param[in] len Number of slots to be reserved.
    /// @return true if the operation completed successfully, false otherwise.
    ///
    bool ReserveStackSpace (int len);

    ///
    /// Resets the Lua script object.
    /// @throws std::bad_alloc New Lua state could not be created.
    ///
    void Reset ();

    /// 
    /// Calls the function with respective arguments on the top of the stack.
    /// @param[in] nargs (Optional) Number of arguments to be passed. Default is zero.
    /// @param[in] nres (Optional) Number of values to be returned. Default is LUA_MULTRET.
    /// @return true if the execution was successful, false otherwise.
    /// @remarks Push the function first, followed by the arguments. 
    /// The arguments must be pushed onto the stack in direct order.
    /// The returned values will be pushed onto the stack in direct order.
    /// The function and its arguments are automatically popped out from the stack, but the 
    /// application shall pop the returned values after using them.
    /// 
    bool Run (int nargs = 0, int nres = LUA_MULTRET);

    /// 
    /// Sets the value of a global Lua variable to the value at the top of the stack.
    /// @param[in] name Name of the variable.
    ///
    void SetGlobal (char const* name);

    /// 
    /// Sets a Lua table field to the value at the top of the stack, using the table key
    /// at the second position and the table itself at the third position from the top of the stack.
    /// @return true if the operation completed successfully, false if a table was not found 
    /// at the correct stack position.
    /// @remarks The value and the key are automatically popped out from the stack after this call.
    ///
    bool SetTableField ();

    /// 
    /// Sets a Lua table field with the given key to the value at the top of the stack, using
    /// the table at the second position from the top of the stack.
    /// @param[in] name Key of the table field.
    /// @return true if the operation completed successfully, false if a table was not found 
    /// at the correct stack position.
    /// @remarks The value is automatically popped out from the stack after this call.
    ///
    bool SetTableField (char const* key);

protected:

    static int _LuaPanic (lua_State* L);

protected:

    lua_State* m_L;
    LuaPanicCallback m_panic;
        
private:

    LuaScript (LuaScript const& script);
    LuaScript& operator= (LuaScript const& script);
};

/************************************************************************
  LuaScript Class Inline Methods
*************************************************************************/

inline void utils::LuaScript::ConcatenateValues (int count)
{
    lua_concat(m_L, count);
}

inline lua_State* utils::LuaScript::GetState () const
{
    return m_L;
}

inline bool utils::LuaScript::GetBoolean (bool* value, int index /*= -1*/)
{
    if (lua_isboolean(m_L, index) != 0)
    {
        *value = (lua_toboolean(m_L, index) != 0);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::GetGlobalBoolean (char const* name, bool* value)
{
    PushGlobal(name);
    bool ret = GetBoolean(value);
    Pop();
    return ret;
}

inline bool utils::LuaScript::GetGlobalInteger (char const* name, lua_Integer* value)
{
    PushGlobal(name);
    bool ret = GetInteger(value);
    Pop();
    return ret;
}

inline bool utils::LuaScript::GetGlobalInteger (char const* name, lua_Integer* value, lua_Integer defval)
{
    PushGlobal(name);
    bool ret = GetInteger(value);
    Pop();
    if (!ret)
    {
        *value = defval;
    }
    return ret;
}

inline bool utils::LuaScript::GetGlobalNumber (char const* name, lua_Number* value)
{
    PushGlobal(name);
    bool ret = GetNumber(value);
    Pop();
    return ret;
}

inline bool utils::LuaScript::GetGlobalNumber (char const* name, lua_Number* value, lua_Number defval)
{
    PushGlobal(name);
    bool ret = GetNumber(value);
    Pop();
    if (!ret)
    {
        *value = defval;
    }
    return ret;
}

inline bool utils::LuaScript::GetGlobalUnsigned (char const* name, lua_Unsigned* value)
{
    PushGlobal(name);
    bool ret = GetUnsigned(value);
    Pop();
    return ret;
}

inline bool utils::LuaScript::GetGlobalUnsigned (char const* name, lua_Unsigned* value, lua_Unsigned defval)
{
    PushGlobal(name);
    bool ret = GetUnsigned(value);
    Pop();
    if (!ret)
    {
        *value = defval;
    }
    return ret;
}

inline bool utils::LuaScript::GetGlobalString (char const* name, std::string* string)
{
    PushGlobal(name);
    bool ret = GetString(string);
    Pop();
    return ret;
}

inline bool utils::LuaScript::GetGlobalString (char const* name, std::string* string, char const* defval)
{
    PushGlobal(name);
    bool ret = GetString(string);
    Pop();
    if (!ret)
    {
        string->assign(defval);
    }
    return ret;
}

inline bool utils::LuaScript::GetInteger (lua_Integer* value, int index /*= -1*/)
{
    int isNumber;
    lua_Integer ret = lua_tointegerx(m_L, index, &isNumber);
    if (isNumber)
    {
        *value = ret;
        return true;
    }
    return false;
}

inline bool utils::LuaScript::GetLightUserData (void** data, int index /*= -1*/)
{
    if (lua_isuserdata(m_L, index) != 0)
    {
        *data = lua_touserdata(m_L, index);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::GetNumber (lua_Number* value, int index /*= -1*/)
{
    int isNumber;
    lua_Number ret = lua_tonumberx(m_L, index, &isNumber);
    if (isNumber)
    {
        *value = ret;
        return true;
    }
    return false;
}

inline bool utils::LuaScript::GetString (char const** string, int index /*= -1*/)
{
    if (char const* ret = lua_tostring(m_L, index))
    {
        *string = ret;
        return true;
    }
    return false;
}

inline bool utils::LuaScript::GetString (char const** string, size_t* len, int index /*= -1*/)
{
    if (char const* ret = lua_tolstring(m_L, index, len))
    {
        *string = ret;
        return true;
    }
    return false;
}

inline bool utils::LuaScript::GetString (std::string* string, int index /*= -1*/)
{
    size_t len;
    if (char const* ret = lua_tolstring(m_L, index, &len))
    {
        string->assign(ret, len);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::GetUnsigned (lua_Unsigned* value, int index /*= -1*/)
{
    int isNumber;
    lua_Unsigned ret = lua_tounsignedx(m_L, index, &isNumber);
    if (isNumber)
    {
        *value = ret;
        return true;
    }
    return false;
}

inline void utils::LuaScript::Pop (int count /* = 1*/)
{
    lua_pop(m_L, count);
}

inline bool utils::LuaScript::PushNil ()
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushnil(m_L);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushBoolean (bool value)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushboolean(m_L, (int)value);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushCFunction (lua_CFunction function)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushcfunction(m_L, function);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushGlobal (char const* name)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_getglobal(m_L, name);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushLightUserData (void* data)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushlightuserdata(m_L, data);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushNumber (lua_Number value)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushnumber(m_L, value);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushInteger (lua_Integer value)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushinteger(m_L, value);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushString (char const* string)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushstring(m_L, string);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushString (char const* string, size_t len)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushlstring(m_L, string, len);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushString (std::string const& string)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushlstring(m_L, string.c_str(), string.length());
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushTableField (bool raw /*= false*/)
{
    if (lua_istable(m_L, -2) && lua_checkstack(m_L, 1) != 0)
    {
        if (raw)
        {
            lua_rawget(m_L, -2);
        }
        else
        {
            lua_gettable(m_L, -2);
        }
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushTableField (char const* key)
{
    if (lua_istable(m_L, -1) && lua_checkstack(m_L, 1) != 0)
    {
        lua_getfield(m_L, -1, key);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushTableField (int key)
{
    if (lua_istable(m_L, -1) && lua_checkstack(m_L, 1) != 0)
    {
        lua_rawgeti(m_L, -1, key);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::PushUnsigned (lua_Unsigned value)
{
    if (lua_checkstack(m_L, 1) != 0)
    {
        lua_pushunsigned(m_L, value);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::ReserveStackSpace (int len)
{
    return lua_checkstack(m_L, len) != 0;
}

inline void utils::LuaScript::SetGlobal (char const* name)
{
    lua_setglobal(m_L, name);
}

inline bool utils::LuaScript::SetTableField ()
{
    if (lua_istable(m_L, -3))
    {
        lua_settable(m_L, -3);
        return true;
    }
    return false;
}

inline bool utils::LuaScript::SetTableField (char const* key)
{
    if (lua_istable(m_L, -2))
    {
        lua_setfield(m_L, -2, key);
        return true;
    }
    return false;
}
