// ============================================================================
// LuauExecutor.cpp
// Core executor implementation.
//
// NOTE: This file uses placeholder / stub calls for the actual Luau C API.
//       When you integrate with the real Luau headers (lua.h, lualib.h,
//       luacode.h, Luau/Compiler.h), replace the stubs with real calls.
// ============================================================================

#include "../include/LuauExecutor.h"
#include "../offsets/InternalOffsets.h"
#include "../environment/LuauEnvironment.h"

#include <chrono>
#include <cstring>
#include <iostream>

// ============================================================================
// Stub Luau C API declarations (replace with #include <lua.h> etc.)
// These match the real Luau API signatures so the code compiles against them.
// ============================================================================
extern "C" {

// --- State management ---
lua_State* lua_newstate(void* allocator, void* ud);
void       lua_close(lua_State* L);
lua_State* lua_newthread(lua_State* L);

// --- Stack operations ---
int        lua_gettop(lua_State* L);
void       lua_settop(lua_State* L, int idx);
void       lua_pushvalue(lua_State* L, int idx);
void       lua_pop(lua_State* L, int n);
void       lua_remove(lua_State* L, int idx);
void       lua_insert(lua_State* L, int idx);
void       lua_replace(lua_State* L, int idx);

// --- Type checks ---
int        lua_type(lua_State* L, int idx);
int        lua_isstring(lua_State* L, int idx);
int        lua_isnumber(lua_State* L, int idx);
int        lua_isfunction(lua_State* L, int idx);
int        lua_istable(lua_State* L, int idx);
int        lua_isnil(lua_State* L, int idx);
int        lua_isboolean(lua_State* L, int idx);
const char* lua_typename(lua_State* L, int tp);

// --- Push values ---
void       lua_pushnil(lua_State* L);
void       lua_pushnumber(lua_State* L, double n);
void       lua_pushinteger(lua_State* L, int n);
void       lua_pushstring(lua_State* L, const char* s);
void       lua_pushlstring(lua_State* L, const char* s, size_t len);
void       lua_pushboolean(lua_State* L, int b);
void       lua_pushcclosure(lua_State* L, lua_CFunction fn, const char* debugname, int nup);
void       lua_pushlightuserdata(lua_State* L, void* p);

// --- Get values ---
double     lua_tonumber(lua_State* L, int idx);
int        lua_tointeger(lua_State* L, int idx);
const char* lua_tostring(lua_State* L, int idx);
const char* lua_tolstring(lua_State* L, int idx, size_t* len);
int        lua_toboolean(lua_State* L, int idx);
void*      lua_touserdata(lua_State* L, int idx);
lua_State* lua_tothread(lua_State* L, int idx);

// --- Table operations ---
void       lua_createtable(lua_State* L, int narr, int nrec);
void       lua_settable(lua_State* L, int idx);
void       lua_gettable(lua_State* L, int idx);
void       lua_setfield(lua_State* L, int idx, const char* k);
void       lua_getfield(lua_State* L, int idx, const char* k);
void       lua_rawset(lua_State* L, int idx);
void       lua_rawget(lua_State* L, int idx);
void       lua_rawseti(lua_State* L, int idx, int n);
void       lua_rawgeti(lua_State* L, int idx, int n);
int        lua_setmetatable(lua_State* L, int idx);
int        lua_getmetatable(lua_State* L, int idx);

// --- Call / pcall ---
void       lua_call(lua_State* L, int nargs, int nresults);
int        lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);

// --- Global table ---
void       lua_setglobal(lua_State* L, const char* name);
void       lua_getglobal(lua_State* L, const char* name);

// --- Misc ---
int        lua_error(lua_State* L);
size_t     lua_objlen(lua_State* L, int idx);
void       lua_concat(lua_State* L, int n);
int        lua_ref(lua_State* L, int idx);
void       lua_unref(lua_State* L, int ref);

// --- Luau-specific ---
int        luau_load(lua_State* L, const char* chunkname,
                     const char* data, size_t size, int env);
void       luaL_sandboxthread(lua_State* L);

// --- Libraries ---
void       luaL_openlibs(lua_State* L);

// --- Compile (Luau compiler) ---
// In practice you'd use the C++ Luau::compile() from Luau/Compiler.h
char*      luau_compile(const char* source, size_t sourceLen,
                        void* options, size_t* outSize);

} // extern "C"


// ============================================================================
// Constants
// ============================================================================
static constexpr int LUA_OK       = 0;
static constexpr int LUA_GLOBALSINDEX = -10002;
static constexpr int LUA_REGISTRYINDEX = -10000;
static constexpr int LUA_MULTRET  = -1;


// ============================================================================
// Constructor / Destructor
// ============================================================================

LuauExecutor::LuauExecutor(const ExecutorConfig& config)
    : m_config(config)
{
}

LuauExecutor::~LuauExecutor() {
    Shutdown();
}


// ============================================================================
// Initialize — attach to the game's existing lua_State
// ============================================================================

bool LuauExecutor::Initialize(lua_State* gameState) {
    if (m_initialized) return true;
    if (!gameState) return false;

    m_gameState = gameState;

    // Create our own thread off the game state so we don't pollute the main thread.
    m_thread = CreateThread();
    if (!m_thread) return false;

    // Apply identity (permission level)
    InjectIdentity(m_config.identity);

    // Register all UNC + standard custom functions
    SetupGlobalEnvironment();

    // Sandbox the thread
    SetupSandbox();

    m_initialized = true;

    if (m_onOutput)
        m_onOutput("[" + m_config.name + " v" + m_config.version + "] Initialized.");

    return true;
}


// ============================================================================
// Shutdown
// ============================================================================

void LuauExecutor::Shutdown() {
    // The thread is owned by the game state's GC — we don't lua_close it.
    // Just null our references.
    m_thread      = nullptr;
    m_gameState   = nullptr;
    m_initialized = false;
}


// ============================================================================
// Execute — compile source and run it
// ============================================================================

ExecutionResult LuauExecutor::Execute(const std::string& source,
                                       const std::string& chunkName)
{
    ExecutionResult result;
    if (!m_initialized || !m_thread) {
        result.error = "Executor not initialized";
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // 1. Compile source to bytecode
    std::string bytecode = Compile(source);
    if (bytecode.empty()) {
        result.error = "Compilation failed";
        if (m_onError) m_onError(result.error);
        return result;
    }

    // 2. Load bytecode onto the stack
    if (!LoadBytecode(bytecode, chunkName)) {
        result.error = "Failed to load bytecode";
        if (m_onError) m_onError(result.error);
        return result;
    }

    // 3. Execute with pcall
    int status = lua_pcall(m_thread, 0, LUA_MULTRET, 0);

    auto end = std::chrono::high_resolution_clock::now();
    result.executionTimeMs =
        std::chrono::duration<double, std::milli>(end - start).count();

    if (status != LUA_OK) {
        // Error is on top of stack
        const char* err = lua_tostring(m_thread, -1);
        result.error = err ? err : "Unknown error";
        lua_pop(m_thread, 1);

        if (m_onError) m_onError(result.error);
    } else {
        result.success = true;

        // Collect any return values as output
        int top = lua_gettop(m_thread);
        if (top > 0) {
            const char* str = lua_tostring(m_thread, -1);
            if (str) result.output = str;
            lua_settop(m_thread, 0); // clear stack
        }

        if (m_onOutput && !result.output.empty())
            m_onOutput(result.output);
    }

    return result;
}


// ============================================================================
// Compile — source -> bytecode
// ============================================================================

std::string LuauExecutor::Compile(const std::string& source,
                                   int optimizationLevel)
{
    size_t outSize = 0;

    // luau_compile returns a malloc'd buffer; caller frees.
    char* bytecode = luau_compile(source.c_str(), source.size(),
                                  nullptr, // options (use defaults)
                                  &outSize);

    if (!bytecode || outSize == 0) {
        return {};
    }

    // Check for compilation error (bytecode starts with 0 on error in Luau)
    if (outSize > 0 && bytecode[0] == 0) {
        std::string errMsg(bytecode + 1, outSize - 1);
        if (m_onError) m_onError("Compile error: " + errMsg);
        free(bytecode);
        return {};
    }

    std::string result(bytecode, outSize);
    free(bytecode);
    return result;
}


// ============================================================================
// LoadBytecode — push compiled closure onto the stack
// ============================================================================

bool LuauExecutor::LoadBytecode(const std::string& bytecode,
                                 const std::string& chunkName)
{
    int status = luau_load(m_thread, chunkName.c_str(),
                           bytecode.data(), bytecode.size(), 0);
    return (status == LUA_OK);
}


// ============================================================================
// SetupGlobalEnvironment — register all UNC functions
// ============================================================================

void LuauExecutor::SetupGlobalEnvironment() {
    if (!m_thread) return;

    const auto& registry = GetUNCRegistry();

    for (const auto& entry : registry) {
        // Check config flags — skip disabled categories
        if (!m_config.enableFS) {
            // Skip filesystem functions
            if (std::strcmp(entry.name, "readfile") == 0 ||
                std::strcmp(entry.name, "writefile") == 0 ||
                std::strcmp(entry.name, "appendfile") == 0 ||
                std::strcmp(entry.name, "listfiles") == 0 ||
                std::strcmp(entry.name, "isfile") == 0 ||
                std::strcmp(entry.name, "isfolder") == 0 ||
                std::strcmp(entry.name, "makefolder") == 0 ||
                std::strcmp(entry.name, "delfolder") == 0 ||
                std::strcmp(entry.name, "delfile") == 0 ||
                std::strcmp(entry.name, "dofile") == 0 ||
                std::strcmp(entry.name, "loadfile") == 0)
                continue;
        }

        if (!m_config.enableHTTP) {
            if (std::strcmp(entry.name, "request") == 0 ||
                std::strcmp(entry.name, "http_request") == 0 ||
                std::strcmp(entry.name, "connect") == 0)
                continue;
        }

        if (!m_config.enableInput) {
            if (std::strstr(entry.name, "mouse") ||
                std::strstr(entry.name, "key"))
                continue;
        }

        RegisterFunction(entry.library, entry.name, entry.func);
    }
}


// ============================================================================
// RegisterFunction — push a single C function into the environment
// ============================================================================

void LuauExecutor::RegisterFunction(const char* library,
                                     const char* name,
                                     lua_CFunction func)
{
    if (!m_thread || !name || !func) return;

    if (library) {
        // Register as library.name
        lua_getglobal(m_thread, library);

        // If the library table doesn't exist yet, create it
        if (lua_isnil(m_thread, -1)) {
            lua_pop(m_thread, 1);
            lua_createtable(m_thread, 0, 8);
            lua_pushvalue(m_thread, -1);
            lua_setglobal(m_thread, library);
        }

        lua_pushcclosure(m_thread, func, name, 0);
        lua_setfield(m_thread, -2, name);
        lua_pop(m_thread, 1); // pop library table
    } else {
        // Register as global
        lua_pushcclosure(m_thread, func, name, 0);
        lua_setglobal(m_thread, name);
    }
}


// ============================================================================
// RegisterLibrary — register a batch of functions under a library table
// ============================================================================

void LuauExecutor::RegisterLibrary(
    const char* name,
    const std::vector<std::pair<const char*, lua_CFunction>>& funcs)
{
    if (!m_thread || !name) return;

    lua_createtable(m_thread, 0, static_cast<int>(funcs.size()));

    for (const auto& [fname, ffunc] : funcs) {
        lua_pushcclosure(m_thread, ffunc, fname, 0);
        lua_setfield(m_thread, -2, fname);
    }

    lua_setglobal(m_thread, name);
}


// ============================================================================
// Identity management
// ============================================================================

void LuauExecutor::SetThreadIdentity(int level) {
    m_config.identity = level;
    if (m_thread) InjectIdentity(level);
}

int LuauExecutor::GetThreadIdentity() const {
    return m_config.identity;
}


// ============================================================================
// Internal helpers
// ============================================================================

lua_State* LuauExecutor::CreateThread() {
    if (!m_gameState) return nullptr;
    return lua_newthread(m_gameState);
}

void LuauExecutor::SetupSandbox() {
    if (!m_thread) return;
    luaL_sandboxthread(m_thread);
}

void LuauExecutor::InjectIdentity(int level) {
    if (!m_thread) return;

    // Use internal offset to write identity level into the lua_State's
    // userdata field. Adjust this based on your game's identity system.
    uintptr_t stateAddr = reinterpret_cast<uintptr_t>(m_thread);
    uintptr_t identityOffset = Offsets::Internal::LuaState_UserData;

    if (identityOffset != 0x0) {
        // Write identity level into the userdata / extra space
        *reinterpret_cast<int*>(stateAddr + identityOffset) = level;
    }
}
