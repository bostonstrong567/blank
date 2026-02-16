// ============================================================================
// UNCFunctions.cpp
// Stub implementations for all UNC custom functions.
//
// Each function is a valid lua_CFunction (takes lua_State*, returns int).
// Replace the stub bodies with real logic tied to your game engine.
// ============================================================================

#include "../environment/LuauEnvironment.h"
#include "../offsets/InternalOffsets.h"
#include "../offsets/ExternalOffsets.h"

#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

// ============================================================================
// Stub Luau C API — same externs as LuauExecutor.cpp
// Replace with real #include <lua.h> when building against Luau.
// ============================================================================
extern "C" {
    lua_State* lua_newthread(lua_State* L);
    int        lua_gettop(lua_State* L);
    void       lua_settop(lua_State* L, int idx);
    void       lua_pushnil(lua_State* L);
    void       lua_pushnumber(lua_State* L, double n);
    void       lua_pushinteger(lua_State* L, int n);
    void       lua_pushstring(lua_State* L, const char* s);
    void       lua_pushlstring(lua_State* L, const char* s, size_t len);
    void       lua_pushboolean(lua_State* L, int b);
    void       lua_pushcclosure(lua_State* L, lua_CFunction fn, const char* debugname, int nup);
    void       lua_pushvalue(lua_State* L, int idx);
    double     lua_tonumber(lua_State* L, int idx);
    int        lua_tointeger(lua_State* L, int idx);
    const char* lua_tostring(lua_State* L, int idx);
    const char* lua_tolstring(lua_State* L, int idx, size_t* len);
    int        lua_toboolean(lua_State* L, int idx);
    void*      lua_touserdata(lua_State* L, int idx);
    int        lua_type(lua_State* L, int idx);
    int        lua_isstring(lua_State* L, int idx);
    int        lua_isnumber(lua_State* L, int idx);
    int        lua_isfunction(lua_State* L, int idx);
    int        lua_istable(lua_State* L, int idx);
    int        lua_isnil(lua_State* L, int idx);
    void       lua_createtable(lua_State* L, int narr, int nrec);
    void       lua_settable(lua_State* L, int idx);
    void       lua_gettable(lua_State* L, int idx);
    void       lua_setfield(lua_State* L, int idx, const char* k);
    void       lua_getfield(lua_State* L, int idx, const char* k);
    void       lua_rawset(lua_State* L, int idx);
    void       lua_rawseti(lua_State* L, int idx, int n);
    void       lua_rawgeti(lua_State* L, int idx, int n);
    int        lua_setmetatable(lua_State* L, int idx);
    int        lua_getmetatable(lua_State* L, int idx);
    void       lua_call(lua_State* L, int nargs, int nresults);
    int        lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);
    void       lua_setglobal(lua_State* L, const char* name);
    void       lua_getglobal(lua_State* L, const char* name);
    int        lua_error(lua_State* L);
    void       lua_pop(lua_State* L, int n);
    int        lua_ref(lua_State* L, int idx);
    int        luau_load(lua_State* L, const char* chunkname,
                         const char* data, size_t size, int env);
    char*      luau_compile(const char* source, size_t sourceLen,
                            void* options, size_t* outSize);
}

// Helper: executor metadata — change these to match your executor name
static const char* EXECUTOR_NAME    = "MyExecutor";
static const char* EXECUTOR_VERSION = "1.0.0";

// Helper: workspace root for sandboxed filesystem access
static const std::string WORKSPACE_ROOT = "./workspace/";

// Helper: validate that a path stays within the workspace sandbox
static bool IsPathSafe(const std::string& path) {
    namespace fs = std::filesystem;
    auto canonical = fs::weakly_canonical(WORKSPACE_ROOT + path);
    auto root = fs::weakly_canonical(WORKSPACE_ROOT);
    // Ensure the resolved path starts with the workspace root
    auto rootStr = root.string();
    auto pathStr = canonical.string();
    return pathStr.substr(0, rootStr.size()) == rootStr;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  CACHE functions                                                         ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::cache_invalidate(lua_State* L) {
    // TODO: Invalidate internal instance cache for the given instance.
    // Arg 1: instance userdata
    return 0;
}

int UNCFunctions::cache_iscached(lua_State* L) {
    // TODO: Check if instance is in the ref cache.
    lua_pushboolean(L, 0);
    return 1;
}

int UNCFunctions::cache_replace(lua_State* L) {
    // TODO: Replace cached reference of arg1 with arg2.
    return 0;
}

int UNCFunctions::cloneref(lua_State* L) {
    // TODO: Clone the instance reference so compareinstances returns false
    // but the underlying object is the same.
    lua_pushvalue(L, 1); // placeholder: just push same ref
    return 1;
}

int UNCFunctions::compareinstances(lua_State* L) {
    // TODO: Compare underlying instance pointers.
    lua_pushboolean(L, 0);
    return 1;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  CLOSURE functions                                                       ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::checkcaller(lua_State* L) {
    // Returns true if the calling function was created by the executor.
    // TODO: Compare calling closure's environment with executor environment.
    lua_pushboolean(L, 1);
    return 1;
}

int UNCFunctions::clonefunction(lua_State* L) {
    // TODO: Deep-clone the function at arg 1.
    lua_pushvalue(L, 1); // placeholder
    return 1;
}

int UNCFunctions::getcallingscript(lua_State* L) {
    // TODO: Walk the call stack and return the Script instance.
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::hookfunction(lua_State* L) {
    // hookfunction(original, hook) -> original
    // TODO: Replace original's internal function pointer with hook.
    // Return a callable reference to the original.
    lua_pushvalue(L, 1); // placeholder: return original
    return 1;
}

int UNCFunctions::iscclosure(lua_State* L) {
    // TODO: Check Closure_IsC offset on the closure at arg 1.
    lua_pushboolean(L, 0);
    return 1;
}

int UNCFunctions::islclosure(lua_State* L) {
    lua_pushboolean(L, 1);
    return 1;
}

int UNCFunctions::isexecutorclosure(lua_State* L) {
    // TODO: Check if the closure's env matches our executor env.
    lua_pushboolean(L, 0);
    return 1;
}

int UNCFunctions::loadstring_custom(lua_State* L) {
    // loadstring(source, chunkname?) -> function | (nil, error)
    size_t sourceLen = 0;
    const char* source = lua_tolstring(L, 1, &sourceLen);
    const char* chunkname = lua_tostring(L, 2);
    if (!chunkname) chunkname = "=loadstring";

    if (!source) {
        lua_pushnil(L);
        lua_pushstring(L, "string expected");
        return 2;
    }

    // Compile
    size_t bcSize = 0;
    char* bytecode = luau_compile(source, sourceLen, nullptr, &bcSize);
    if (!bytecode || bcSize == 0) {
        lua_pushnil(L);
        lua_pushstring(L, "compilation failed");
        return 2;
    }

    // Check for compile error (Luau signals error with leading 0 byte)
    if (bytecode[0] == 0) {
        std::string err(bytecode + 1, bcSize - 1);
        free(bytecode);
        lua_pushnil(L);
        lua_pushstring(L, err.c_str());
        return 2;
    }

    int status = luau_load(L, chunkname, bytecode, bcSize, 0);
    free(bytecode);

    if (status != 0) {
        // Error message is on top of stack
        lua_pushnil(L);
        lua_pushvalue(L, -2); // push the error
        return 2;
    }

    return 1; // compiled function on stack
}

int UNCFunctions::newcclosure(lua_State* L) {
    // Wraps a Luau closure as a C closure so iscclosure returns true.
    // TODO: Create wrapper C closure that calls the original.
    lua_pushvalue(L, 1); // placeholder
    return 1;
}

int UNCFunctions::newlclosure(lua_State* L) {
    // Wraps a C closure as a Luau closure so islclosure returns true.
    lua_pushvalue(L, 1); // placeholder
    return 1;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  CONSOLE functions                                                       ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::rconsoleclear(lua_State* L) {
    // TODO: Clear the executor console window.
    std::cout << "\033[2J\033[H"; // ANSI clear
    return 0;
}

int UNCFunctions::rconsolecreate(lua_State* L) {
    // TODO: Allocate/show console window if not already open.
    return 0;
}

int UNCFunctions::rconsoledestroy(lua_State* L) {
    // TODO: Hide/free the console window.
    return 0;
}

int UNCFunctions::rconsoleinput(lua_State* L) {
    // Blocking read from console. Returns the line.
    std::string line;
    std::getline(std::cin, line);
    lua_pushstring(L, line.c_str());
    return 1;
}

int UNCFunctions::rconsoleprint(lua_State* L) {
    const char* text = lua_tostring(L, 1);
    if (text) std::cout << text;
    return 0;
}

int UNCFunctions::rconsolesettitle(lua_State* L) {
    const char* title = lua_tostring(L, 1);
    if (title) {
        // ANSI escape to set terminal title
        std::cout << "\033]0;" << title << "\007";
    }
    return 0;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  CRYPTO functions                                                        ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

// Base64 encoding table
static const char B64_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string Base64Encode(const std::string& input) {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(B64_TABLE[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(B64_TABLE[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

static std::string Base64Decode(const std::string& input) {
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[B64_TABLE[i]] = i;
    std::string out;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

int UNCFunctions::crypt_base64encode(lua_State* L) {
    size_t len = 0;
    const char* data = lua_tolstring(L, 1, &len);
    if (!data) { lua_pushstring(L, ""); return 1; }
    std::string result = Base64Encode(std::string(data, len));
    lua_pushlstring(L, result.c_str(), result.size());
    return 1;
}

int UNCFunctions::crypt_base64decode(lua_State* L) {
    const char* data = lua_tostring(L, 1);
    if (!data) { lua_pushstring(L, ""); return 1; }
    std::string result = Base64Decode(data);
    lua_pushlstring(L, result.c_str(), result.size());
    return 1;
}

int UNCFunctions::crypt_encrypt(lua_State* L) {
    // TODO: Implement AES-CBC / AES-GCM encryption using a crypto library.
    lua_pushstring(L, "");
    return 1;
}

int UNCFunctions::crypt_decrypt(lua_State* L) {
    // TODO: Implement AES decryption.
    lua_pushstring(L, "");
    return 1;
}

int UNCFunctions::crypt_generatebytes(lua_State* L) {
    int count = lua_tointeger(L, 1);
    if (count <= 0) count = 32;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 255);
    std::string bytes(count, '\0');
    for (int i = 0; i < count; i++) bytes[i] = static_cast<char>(dist(gen));
    std::string encoded = Base64Encode(bytes);
    lua_pushstring(L, encoded.c_str());
    return 1;
}

int UNCFunctions::crypt_generatekey(lua_State* L) {
    // Generate a 32-byte (256-bit) random key, base64 encoded.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 255);
    std::string key(32, '\0');
    for (int i = 0; i < 32; i++) key[i] = static_cast<char>(dist(gen));
    lua_pushstring(L, Base64Encode(key).c_str());
    return 1;
}

int UNCFunctions::crypt_hash(lua_State* L) {
    // TODO: Implement SHA-256, SHA-384, SHA-512, MD5 hashing.
    // Arg1: data, Arg2: algorithm name
    lua_pushstring(L, "TODO:hash");
    return 1;
}

int UNCFunctions::crypt_hmac(lua_State* L) {
    // TODO: Implement HMAC using a crypto library.
    lua_pushstring(L, "TODO:hmac");
    return 1;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  DEBUG extension functions                                               ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::debug_getconstant(lua_State* L) {
    // debug.getconstant(func, index) -> value
    // TODO: Read Proto->k[index] using internal offsets.
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::debug_getconstants(lua_State* L) {
    // debug.getconstants(func) -> {constants...}
    // TODO: Read all Proto->k values.
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::debug_getinfo_custom(lua_State* L) {
    // Extended debug.getinfo — returns table with source, name, etc.
    // TODO: Read Proto fields via offsets.
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::debug_getproto(lua_State* L) {
    // debug.getproto(func, index, activated?) -> proto_func | {instances}
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::debug_getprotos(lua_State* L) {
    // debug.getprotos(func) -> {protos...}
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::debug_getstack(lua_State* L) {
    // debug.getstack(level, index?) -> value | {values...}
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::debug_getupvalue(lua_State* L) {
    // debug.getupvalue(func, index) -> value
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::debug_getupvalues(lua_State* L) {
    // debug.getupvalues(func) -> {upvalues...}
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::debug_setconstant(lua_State* L) {
    // debug.setconstant(func, index, value)
    // TODO: Write to Proto->k[index].
    return 0;
}

int UNCFunctions::debug_setstack(lua_State* L) {
    // debug.setstack(level, index, value)
    return 0;
}

int UNCFunctions::debug_setupvalue(lua_State* L) {
    // debug.setupvalue(func, index, value)
    return 0;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  DRAWING functions                                                       ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::Drawing_new(lua_State* L) {
    // Drawing.new(type) -> DrawingObject userdata
    // Types: "Line", "Text", "Image", "Circle", "Square", "Quad", "Triangle"
    // TODO: Allocate drawing object and push as userdata with metatable.
    lua_pushnil(L); // placeholder
    return 1;
}

int UNCFunctions::Drawing_clear(lua_State* L) {
    // cleardrawcache() — remove all drawing objects
    // TODO: Iterate and destroy all drawing objects.
    return 0;
}

int UNCFunctions::getrenderproperty(lua_State* L) {
    // getrenderproperty(drawingObj, property) -> value
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::setrenderproperty(lua_State* L) {
    // setrenderproperty(drawingObj, property, value)
    return 0;
}

int UNCFunctions::isrenderobj(lua_State* L) {
    // isrenderobj(value) -> bool
    lua_pushboolean(L, 0);
    return 1;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  FILESYSTEM functions                                                    ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::readfile(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    if (!path) { lua_pushstring(L, "invalid path"); return lua_error(L); }
    if (!IsPathSafe(path)) { lua_pushstring(L, "path escapes sandbox"); return lua_error(L); }

    std::ifstream f(WORKSPACE_ROOT + path, std::ios::binary);
    if (!f.is_open()) { lua_pushstring(L, "file not found"); return lua_error(L); }

    std::ostringstream ss;
    ss << f.rdbuf();
    std::string contents = ss.str();
    lua_pushlstring(L, contents.c_str(), contents.size());
    return 1;
}

int UNCFunctions::writefile(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    size_t dataLen = 0;
    const char* data = lua_tolstring(L, 2, &dataLen);
    if (!path || !data) { lua_pushstring(L, "invalid arguments"); return lua_error(L); }
    if (!IsPathSafe(path)) { lua_pushstring(L, "path escapes sandbox"); return lua_error(L); }

    std::ofstream f(WORKSPACE_ROOT + path, std::ios::binary);
    if (!f.is_open()) { lua_pushstring(L, "cannot open file for writing"); return lua_error(L); }
    f.write(data, dataLen);
    return 0;
}

int UNCFunctions::appendfile(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    size_t dataLen = 0;
    const char* data = lua_tolstring(L, 2, &dataLen);
    if (!path || !data) { lua_pushstring(L, "invalid arguments"); return lua_error(L); }
    if (!IsPathSafe(path)) { lua_pushstring(L, "path escapes sandbox"); return lua_error(L); }

    std::ofstream f(WORKSPACE_ROOT + path, std::ios::binary | std::ios::app);
    if (!f.is_open()) { lua_pushstring(L, "cannot open file for appending"); return lua_error(L); }
    f.write(data, dataLen);
    return 0;
}

int UNCFunctions::loadfile_custom(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    if (!path) { lua_pushnil(L); lua_pushstring(L, "invalid path"); return 2; }
    if (!IsPathSafe(path)) { lua_pushnil(L); lua_pushstring(L, "path escapes sandbox"); return 2; }

    std::ifstream f(WORKSPACE_ROOT + path);
    if (!f.is_open()) { lua_pushnil(L); lua_pushstring(L, "file not found"); return 2; }

    std::ostringstream ss;
    ss << f.rdbuf();
    std::string source = ss.str();

    // Push source and call loadstring
    lua_pushstring(L, source.c_str());
    lua_pushstring(L, ("@" + std::string(path)).c_str());
    return UNCFunctions::loadstring_custom(L);
}

int UNCFunctions::listfiles(lua_State* L) {
    const char* dir = lua_tostring(L, 1);
    if (!dir) dir = "";
    std::string fullPath = WORKSPACE_ROOT + dir;

    lua_createtable(L, 0, 0);
    int idx = 1;

    namespace fs = std::filesystem;
    if (fs::exists(fullPath) && fs::is_directory(fullPath)) {
        for (const auto& entry : fs::directory_iterator(fullPath)) {
            std::string name = entry.path().filename().string();
            lua_pushstring(L, name.c_str());
            lua_rawseti(L, -2, idx++);
        }
    }
    return 1;
}

int UNCFunctions::isfile(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    if (!path) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, std::filesystem::is_regular_file(WORKSPACE_ROOT + path) ? 1 : 0);
    return 1;
}

int UNCFunctions::isfolder(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    if (!path) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, std::filesystem::is_directory(WORKSPACE_ROOT + path) ? 1 : 0);
    return 1;
}

int UNCFunctions::makefolder(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    if (!path) { lua_pushstring(L, "invalid path"); return lua_error(L); }
    if (!IsPathSafe(path)) { lua_pushstring(L, "path escapes sandbox"); return lua_error(L); }
    std::filesystem::create_directories(WORKSPACE_ROOT + path);
    return 0;
}

int UNCFunctions::delfolder(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    if (!path) { lua_pushstring(L, "invalid path"); return lua_error(L); }
    if (!IsPathSafe(path)) { lua_pushstring(L, "path escapes sandbox"); return lua_error(L); }
    std::filesystem::remove_all(WORKSPACE_ROOT + path);
    return 0;
}

int UNCFunctions::delfile(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    if (!path) { lua_pushstring(L, "invalid path"); return lua_error(L); }
    if (!IsPathSafe(path)) { lua_pushstring(L, "path escapes sandbox"); return lua_error(L); }
    std::filesystem::remove(WORKSPACE_ROOT + path);
    return 0;
}

int UNCFunctions::dofile_custom(lua_State* L) {
    // Load and execute the file.
    // First load it, then if successful, call it.
    int loadResult = loadfile_custom(L);
    if (lua_isnil(L, -loadResult)) {
        // loadfile returned (nil, error)
        return loadResult;
    }
    // Call the loaded function
    lua_call(L, 0, 0);
    return 0;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  INPUT functions                                                         ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::isrbxactive(lua_State* L) {
    // TODO: Check if the game window is focused (platform-specific).
    lua_pushboolean(L, 1);
    return 1;
}

int UNCFunctions::mouse1click(lua_State* L) {
    // TODO: Simulate left mouse click via OS API.
    return 0;
}
int UNCFunctions::mouse1press(lua_State* L) { return 0; }
int UNCFunctions::mouse1release(lua_State* L) { return 0; }
int UNCFunctions::mouse2click(lua_State* L) { return 0; }
int UNCFunctions::mouse2press(lua_State* L) { return 0; }
int UNCFunctions::mouse2release(lua_State* L) { return 0; }

int UNCFunctions::mousemoveabs(lua_State* L) {
    // TODO: Move mouse to absolute screen position.
    // int x = lua_tointeger(L, 1);
    // int y = lua_tointeger(L, 2);
    return 0;
}

int UNCFunctions::mousemoverel(lua_State* L) {
    // TODO: Move mouse by relative offset.
    return 0;
}

int UNCFunctions::mousescroll(lua_State* L) {
    // TODO: Scroll mouse wheel.
    return 0;
}

int UNCFunctions::keypress(lua_State* L) {
    // TODO: Simulate key press via OS API.
    return 0;
}

int UNCFunctions::keyrelease(lua_State* L) { return 0; }
int UNCFunctions::keyclick(lua_State* L) { return 0; }


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  INSTANCE functions                                                      ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::fireclickdetector(lua_State* L) {
    // TODO: Fire ClickDetector on the given instance.
    return 0;
}

int UNCFunctions::fireproximityprompt(lua_State* L) { return 0; }

int UNCFunctions::firetouchinterest(lua_State* L) {
    // firetouchinterest(part, otherPart, toggle)
    // toggle: 0 = end touch, 1 = begin touch
    return 0;
}

int UNCFunctions::firesignal(lua_State* L) {
    // firesignal(signal, args...)
    // TODO: Fire all connections of the given signal.
    return 0;
}

int UNCFunctions::getconnections(lua_State* L) {
    // getconnections(signal) -> {Connection...}
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::gethiddenproperty(lua_State* L) {
    // gethiddenproperty(instance, propName) -> value, bool
    lua_pushnil(L);
    lua_pushboolean(L, 0);
    return 2;
}

int UNCFunctions::sethiddenproperty(lua_State* L) {
    // sethiddenproperty(instance, propName, value) -> bool
    lua_pushboolean(L, 0);
    return 1;
}

int UNCFunctions::gethui(lua_State* L) {
    // Returns the hidden UI container (CoreGui equivalent).
    lua_pushnil(L); // TODO: return actual container
    return 1;
}

int UNCFunctions::getinstances(lua_State* L) {
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::getnilinstances(lua_State* L) {
    lua_createtable(L, 0, 0);
    return 1;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  METATABLE functions                                                     ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::getrawmetatable(lua_State* L) {
    // Bypass __metatable to get the real metatable.
    if (lua_getmetatable(L, 1)) {
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::hookmetamethod(lua_State* L) {
    // hookmetamethod(object, metamethod, hook) -> original
    // TODO: Replace the metamethod in the raw metatable.
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::getnamecallmethod(lua_State* L) {
    // Returns the method name used in the current __namecall invocation.
    // TODO: Read from lua_State namecall TString.
    lua_pushstring(L, "");
    return 1;
}

int UNCFunctions::setnamecallmethod(lua_State* L) {
    // Sets the namecall method name for the current invocation.
    // TODO: Write to lua_State namecall field.
    return 0;
}

int UNCFunctions::setrawmetatable(lua_State* L) {
    // setrawmetatable(object, metatable)
    lua_setmetatable(L, 1);
    return 0;
}

int UNCFunctions::setreadonly(lua_State* L) {
    // setreadonly(table, bool)
    // TODO: Write to Table->readonly using internal offset.
    return 0;
}

int UNCFunctions::isreadonly(lua_State* L) {
    // TODO: Read Table->readonly using internal offset.
    lua_pushboolean(L, 0);
    return 1;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  MISC functions                                                          ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::identifyexecutor(lua_State* L) {
    lua_pushstring(L, EXECUTOR_NAME);
    lua_pushstring(L, EXECUTOR_VERSION);
    return 2;
}

int UNCFunctions::getexecutorname(lua_State* L) {
    lua_pushstring(L, EXECUTOR_NAME);
    return 1;
}

int UNCFunctions::lz4compress(lua_State* L) {
    // TODO: Implement LZ4 compression.
    lua_pushstring(L, "");
    return 1;
}

int UNCFunctions::lz4decompress(lua_State* L) {
    // TODO: Implement LZ4 decompression.
    lua_pushstring(L, "");
    return 1;
}

int UNCFunctions::messagebox(lua_State* L) {
    // messagebox(text, caption, flags) -> button clicked
    const char* text    = lua_tostring(L, 1);
    const char* caption = lua_tostring(L, 2);
    // int flags = lua_tointeger(L, 3);
    // TODO: Platform-specific message box.
    std::cout << "[MessageBox] " << (caption ? caption : "") << ": "
              << (text ? text : "") << std::endl;
    lua_pushinteger(L, 1); // OK button
    return 1;
}

int UNCFunctions::request(lua_State* L) {
    // request({Url=, Method=, Headers=, Body=}) -> {Success, StatusCode, Body, Headers}
    // TODO: Implement HTTP request using libcurl or platform HTTP.
    lua_createtable(L, 0, 4);
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "Success");
    lua_pushinteger(L, 0);
    lua_setfield(L, -2, "StatusCode");
    lua_pushstring(L, "");
    lua_setfield(L, -2, "Body");
    lua_createtable(L, 0, 0);
    lua_setfield(L, -2, "Headers");
    return 1;
}

int UNCFunctions::setclipboard(lua_State* L) {
    // TODO: Platform-specific clipboard write.
    const char* text = lua_tostring(L, 1);
    (void)text;
    return 0;
}

int UNCFunctions::setfpscap(lua_State* L) {
    // TODO: Set the game's FPS cap.
    // int fps = lua_tointeger(L, 1);
    return 0;
}

int UNCFunctions::getfpscap(lua_State* L) {
    lua_pushinteger(L, 60);
    return 1;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  SCRIPT functions                                                        ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::getgc(lua_State* L) {
    // getgc(includeTables?) -> {objects...}
    // TODO: Walk the GC list and collect references.
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::getgenv(lua_State* L) {
    // Return the executor's global environment table.
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    return 1;
}

int UNCFunctions::getreg(lua_State* L) {
    // Return the Lua registry.
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    return 1;
}

int UNCFunctions::getrenv(lua_State* L) {
    // Return the game's original environment.
    // TODO: Return the game state's global table.
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::getscriptbytecode(lua_State* L) {
    // getscriptbytecode(script) -> bytecode string
    // TODO: Read Script.Source or compiled bytecode from the instance.
    lua_pushstring(L, "");
    return 1;
}

int UNCFunctions::getscripthash(lua_State* L) {
    // getscripthash(script) -> hash string
    lua_pushstring(L, "");
    return 1;
}

int UNCFunctions::getscripts(lua_State* L) {
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::getrunningscripts(lua_State* L) {
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::getloadedmodules(lua_State* L) {
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::getsenv(lua_State* L) {
    // getsenv(script) -> environment table
    lua_createtable(L, 0, 0);
    return 1;
}

int UNCFunctions::getscriptclosure(lua_State* L) {
    // getscriptclosure(script) -> function
    lua_pushnil(L);
    return 1;
}

int UNCFunctions::getthreadidentity(lua_State* L) {
    // TODO: Read identity from lua_State userdata.
    lua_pushinteger(L, 8);
    return 1;
}

int UNCFunctions::setthreadidentity(lua_State* L) {
    // TODO: Write identity to lua_State userdata.
    // int level = lua_tointeger(L, 1);
    return 0;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  WEBSOCKET functions                                                     ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::WebSocket_connect(lua_State* L) {
    // WebSocket.connect(url) -> WebSocket object
    // TODO: Implement WebSocket client (e.g. using libwebsockets or Beast).
    lua_createtable(L, 0, 0);
    // The returned table should have: Send(msg), Close(), OnMessage, OnClose
    return 1;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  TASK library functions                                                  ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int UNCFunctions::task_cancel(lua_State* L) {
    // task.cancel(thread) — cancel a deferred/delayed thread
    // TODO: Find the thread in the scheduler and remove it.
    return 0;
}

int UNCFunctions::task_defer(lua_State* L) {
    // task.defer(func, args...) -> thread
    // Schedules func to run at the end of the current resumption cycle.
    // TODO: Push to deferred queue.
    lua_State* thread = lua_newthread(L);
    (void)thread;
    return 1;
}

int UNCFunctions::task_delay(lua_State* L) {
    // task.delay(seconds, func, args...) -> thread
    // TODO: Schedule with delay in the task scheduler.
    lua_State* thread = lua_newthread(L);
    (void)thread;
    return 1;
}

int UNCFunctions::task_spawn(lua_State* L) {
    // task.spawn(func, args...) -> thread
    // Immediately resumes func on a new thread.
    // TODO: Create thread, push func+args, resume immediately.
    lua_State* thread = lua_newthread(L);
    (void)thread;
    return 1;
}

int UNCFunctions::task_wait(lua_State* L) {
    // task.wait(duration?) -> elapsed
    // Yields the current thread for `duration` seconds.
    // TODO: Yield and schedule wake-up in task scheduler.
    double duration = lua_tonumber(L, 1);
    if (duration <= 0) duration = 0.03; // ~1 frame
    lua_pushnumber(L, duration);
    return 1;
}

static constexpr int LUA_GLOBALSINDEX_VAL = -10002;
static constexpr int LUA_REGISTRYINDEX_VAL = -10000;
