#pragma once
// ============================================================================
// LuauEnvironment.h
// Complete UNC (Unified Naming Convention) compatible Luau environment.
// This registers every standard + custom function into the executor's
// global environment so scripts can call them.
//
// Reference: UNC Standard v2 (latest)
// https://github.com/unified-naming-convention/NamingStandard
// ============================================================================

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

// Forward declarations — replace with your actual Luau headers
struct lua_State;
typedef int (*lua_CFunction)(lua_State* L);

// ============================================================================
// Function registry entry
// ============================================================================
struct FunctionEntry {
    const char* library;      // nullptr = global, otherwise library name
    const char* name;         // function name
    lua_CFunction func;       // C implementation
    const char* description;  // human-readable note
};

// ============================================================================
// COMPLETE LUAU STANDARD LIBRARY — FUNCTION LIST
// Every function that ships with Luau, organized by library.
// ============================================================================

namespace LuauStdLib {

// ─────────────────────────────────────────────────────────────────────────────
// GLOBAL FUNCTIONS (no library prefix)
// ─────────────────────────────────────────────────────────────────────────────
// assert(v, message?)                — raise error if v is falsy
// collectgarbage(opt?, arg?)         — GC control
// error(message, level?)            — raise an error
// gcinfo()                          — return GC memory in KB
// getfenv(f?)                       — get function environment
// getmetatable(obj)                 — get metatable
// ipairs(t)                         — iterator for array part
// loadstring(code, chunkname?)      — compile+load Luau source
// newproxy(addmt?)                  — create blank userdata proxy
// next(t, index?)                   — raw table iterator
// pairs(t)                          — iterator for all keys
// pcall(f, args...)                 — protected call
// print(args...)                    — output to console
// rawequal(a, b)                    — equality without metamethods
// rawget(t, k)                      — index without metamethods
// rawlen(t)                         — length without metamethods
// rawset(t, k, v)                   — newindex without metamethods
// require(module)                   — load a module
// select(index, args...)            — pick from varargs
// setfenv(f, env)                   — set function environment
// setmetatable(t, mt)               — set metatable
// tonumber(v, base?)                — convert to number
// tostring(v)                       — convert to string
// type(v)                           — return type name
// typeof(v)                         — return detailed type (Luau extension)
// unpack(t, i?, j?)                 — unpack table to tuple
// xpcall(f, err, args...)           — protected call with error handler

// ─────────────────────────────────────────────────────────────────────────────
// COROUTINE library
// ─────────────────────────────────────────────────────────────────────────────
// coroutine.close(co)               — close a coroutine
// coroutine.create(f)               — create new coroutine
// coroutine.isyieldable()           — can the running coroutine yield?
// coroutine.resume(co, args...)     — resume coroutine
// coroutine.running()               — return running coroutine + is-main
// coroutine.status(co)              — "running"/"suspended"/"normal"/"dead"
// coroutine.wrap(f)                 — create coroutine wrapper function
// coroutine.yield(args...)          — yield the coroutine

// ─────────────────────────────────────────────────────────────────────────────
// STRING library
// ─────────────────────────────────────────────────────────────────────────────
// string.byte(s, i?, j?)            — char codes
// string.char(codes...)             — codes to string
// string.find(s, pattern, init?, plain?) — pattern search
// string.format(fmt, args...)       — sprintf-style formatting
// string.gmatch(s, pattern)         — global pattern match iterator
// string.gsub(s, pattern, repl, n?) — global substitution
// string.len(s)                     — string length
// string.lower(s)                   — lowercase
// string.match(s, pattern, init?)   — pattern match
// string.pack(fmt, args...)         — pack into binary string
// string.packsize(fmt)              — size of packed string
// string.rep(s, n, sep?)            — repeat string
// string.reverse(s)                 — reverse string
// string.split(s, sep?)             — split by separator (Luau extension)
// string.sub(s, i, j?)              — substring
// string.unpack(fmt, s, pos?)       — unpack binary string
// string.upper(s)                   — uppercase

// ─────────────────────────────────────────────────────────────────────────────
// TABLE library
// ─────────────────────────────────────────────────────────────────────────────
// table.clear(t)                    — remove all entries
// table.clone(t)                    — shallow clone
// table.concat(t, sep?, i?, j?)     — concatenate array elements
// table.create(n, value?)           — pre-allocate array
// table.find(t, value, init?)       — search array part
// table.foreach(t, f)               — iterate (deprecated, included for compat)
// table.foreachi(t, f)              — iterate array (deprecated, included for compat)
// table.freeze(t)                   — make table read-only
// table.getn(t)                     — array length (deprecated, included for compat)
// table.insert(t, pos?, value)      — insert into array
// table.isfrozen(t)                 — check if frozen
// table.maxn(t)                     — largest positive numeric key
// table.move(a1, f, e, t, a2?)      — move elements between tables
// table.pack(args...)               — pack varargs into table
// table.remove(t, pos?)             — remove from array
// table.sort(t, comp?)              — in-place sort
// table.unpack(t, i?, j?)           — unpack table

// ─────────────────────────────────────────────────────────────────────────────
// MATH library
// ─────────────────────────────────────────────────────────────────────────────
// math.abs(x)                       — absolute value
// math.acos(x)                      — arc cosine
// math.asin(x)                      — arc sine
// math.atan(y, x?)                  — arc tangent (atan2 when 2 args)
// math.atan2(y, x)                  — arc tangent of y/x
// math.ceil(x)                      — round up
// math.clamp(x, min, max)           — clamp value (Luau extension)
// math.cos(x)                       — cosine
// math.cosh(x)                      — hyperbolic cosine
// math.deg(x)                       — radians to degrees
// math.exp(x)                       — e^x
// math.floor(x)                     — round down
// math.fmod(x, y)                   — float modulo
// math.frexp(x)                     — mantissa + exponent
// math.huge                         — infinity constant
// math.ldexp(m, e)                  — m * 2^e
// math.log(x, base?)               — logarithm
// math.log10(x)                     — base-10 log (deprecated, compat)
// math.max(args...)                 — maximum
// math.min(args...)                 — minimum
// math.modf(x)                      — integer + fractional parts
// math.noise(x, y?, z?)             — Perlin noise (Luau extension)
// math.pi                           — pi constant
// math.pow(x, y)                    — x^y (deprecated, use x^y operator)
// math.rad(x)                       — degrees to radians
// math.random(m?, n?)               — random number
// math.randomseed(seed)             — seed RNG
// math.round(x)                     — round to nearest integer (Luau extension)
// math.sign(x)                      — sign of x: -1, 0, or 1 (Luau extension)
// math.sin(x)                       — sine
// math.sinh(x)                      — hyperbolic sine
// math.sqrt(x)                      — square root
// math.tan(x)                       — tangent
// math.tanh(x)                      — hyperbolic tangent

// ─────────────────────────────────────────────────────────────────────────────
// BIT32 library
// ─────────────────────────────────────────────────────────────────────────────
// bit32.arshift(x, disp)            — arithmetic right shift
// bit32.band(args...)               — bitwise AND
// bit32.bnot(x)                     — bitwise NOT
// bit32.bor(args...)                — bitwise OR
// bit32.btest(args...)              — test if AND is nonzero
// bit32.bxor(args...)               — bitwise XOR
// bit32.countlz(x)                  — count leading zeros (Luau extension)
// bit32.countrz(x)                  — count trailing zeros (Luau extension)
// bit32.extract(x, field, width?)   — extract bit field
// bit32.lrotate(x, disp)            — left rotate
// bit32.lshift(x, disp)             — left shift
// bit32.replace(x, v, field, width?)— replace bit field
// bit32.rrotate(x, disp)            — right rotate
// bit32.rshift(x, disp)             — right shift
// bit32.byteswap(x)                 — reverse byte order (Luau extension)

// ─────────────────────────────────────────────────────────────────────────────
// OS library (sandboxed subset)
// ─────────────────────────────────────────────────────────────────────────────
// os.clock()                        — CPU time
// os.date(format?, time?)           — formatted date
// os.difftime(t2, t1)               — time difference
// os.time(table?)                   — current time

// ─────────────────────────────────────────────────────────────────────────────
// DEBUG library
// ─────────────────────────────────────────────────────────────────────────────
// debug.info(level|f, what)         — debug info (Luau-specific API)
// debug.traceback(message?, level?) — stack traceback
// debug.profilebegin(label)         — start profiler zone
// debug.profileend()                — end profiler zone
// debug.setmemorycategory(tag)      — label memory allocs
// debug.resetmemorycategory()       — reset memory label

// ─────────────────────────────────────────────────────────────────────────────
// UTF8 library
// ─────────────────────────────────────────────────────────────────────────────
// utf8.char(codes...)               — code points to UTF-8 string
// utf8.charpattern                  — pattern that matches one UTF-8 char
// utf8.codepoint(s, i?, j?)         — code points from string
// utf8.codes(s)                     — iterator over code points
// utf8.graphemes(s, i?, j?)         — iterator over grapheme clusters (Luau ext)
// utf8.len(s, i?, j?)               — length in code points
// utf8.nfcnormalize(s)              — NFC normalize (Luau extension)
// utf8.nfdnormalize(s)              — NFD normalize (Luau extension)
// utf8.offset(s, n, i?)             — byte offset of nth code point

// ─────────────────────────────────────────────────────────────────────────────
// BUFFER library (Luau extension)
// ─────────────────────────────────────────────────────────────────────────────
// buffer.create(size)               — create new buffer
// buffer.fromstring(s)              — buffer from string
// buffer.tostring(b)                — buffer to string
// buffer.len(b)                     — buffer length
// buffer.copy(dst, doff, src, soff?, count?) — copy bytes between buffers
// buffer.fill(b, offset, value, count?)      — fill with byte value
// buffer.readi8(b, offset)          — read signed 8-bit int
// buffer.readu8(b, offset)          — read unsigned 8-bit int
// buffer.readi16(b, offset)         — read signed 16-bit int
// buffer.readu16(b, offset)         — read unsigned 16-bit int
// buffer.readi32(b, offset)         — read signed 32-bit int
// buffer.readu32(b, offset)         — read unsigned 32-bit int
// buffer.readf32(b, offset)         — read 32-bit float
// buffer.readf64(b, offset)         — read 64-bit float
// buffer.readstring(b, offset, count) — read string
// buffer.writei8(b, offset, value)  — write signed 8-bit int
// buffer.writeu8(b, offset, value)  — write unsigned 8-bit int
// buffer.writei16(b, offset, value) — write signed 16-bit int
// buffer.writeu16(b, offset, value) — write unsigned 16-bit int
// buffer.writei32(b, offset, value) — write signed 32-bit int
// buffer.writeu32(b, offset, value) — write unsigned 32-bit int
// buffer.writef32(b, offset, value) — write 32-bit float
// buffer.writef64(b, offset, value) — write 64-bit float
// buffer.writestring(b, offset, value, count?) — write string

} // namespace LuauStdLib


// ============================================================================
// UNC CUSTOM FUNCTIONS — FULL LIST (Unified Naming Convention v2)
// These are the executor-specific functions that scripts expect.
// ============================================================================

namespace UNCFunctions {

// Stub implementations — replace bodies with real logic tied to your game.
// Each function signature: int fname(lua_State* L);

// ─── CACHE ──────────────────────────────────────────────────────────────────
int cache_invalidate(lua_State* L);   // cache.invalidate(instance)
int cache_iscached(lua_State* L);     // cache.iscached(instance)
int cache_replace(lua_State* L);      // cache.replace(instance, newInstance)
int cloneref(lua_State* L);           // cloneref(instance)
int compareinstances(lua_State* L);   // compareinstances(a, b)

// ─── CLOSURES ───────────────────────────────────────────────────────────────
int checkcaller(lua_State* L);        // checkcaller()
int clonefunction(lua_State* L);      // clonefunction(func)
int getcallingscript(lua_State* L);   // getcallingscript()
int hookfunction(lua_State* L);       // hookfunction(old, new) -> old
int iscclosure(lua_State* L);         // iscclosure(f)
int islclosure(lua_State* L);         // islclosure(f)
int isexecutorclosure(lua_State* L);  // isexecutorclosure(f) / checkclosure(f)
int loadstring_custom(lua_State* L);  // loadstring(src, chunkname?)
int newcclosure(lua_State* L);        // newcclosure(f)
int newlclosure(lua_State* L);        // newlclosure(f)

// ─── CONSOLE ────────────────────────────────────────────────────────────────
int rconsoleclear(lua_State* L);      // rconsoleclear()
int rconsolecreate(lua_State* L);     // rconsolecreate()
int rconsoledestroy(lua_State* L);    // rconsoledestroy()
int rconsoleinput(lua_State* L);      // rconsoleinput() -> string
int rconsoleprint(lua_State* L);      // rconsoleprint(text)
int rconsolesettitle(lua_State* L);   // rconsolesettitle(title)

// ─── CRYPTO ─────────────────────────────────────────────────────────────────
int crypt_base64encode(lua_State* L); // crypt.base64encode(data) / crypt.base64.encode
int crypt_base64decode(lua_State* L); // crypt.base64decode(data) / crypt.base64.decode
int crypt_encrypt(lua_State* L);      // crypt.encrypt(data, key, iv?, mode?)
int crypt_decrypt(lua_State* L);      // crypt.decrypt(data, key, iv?, mode?)
int crypt_generatebytes(lua_State* L);// crypt.generatebytes(count)
int crypt_generatekey(lua_State* L);  // crypt.generatekey()
int crypt_hash(lua_State* L);        // crypt.hash(data, algorithm)
int crypt_hmac(lua_State* L);        // crypt.hmac(data, key, algorithm)

// ─── DEBUG ──────────────────────────────────────────────────────────────────
int debug_getconstant(lua_State* L);  // debug.getconstant(f, idx)
int debug_getconstants(lua_State* L); // debug.getconstants(f)
int debug_getinfo_custom(lua_State* L);// debug.getinfo(f) (extended)
int debug_getproto(lua_State* L);     // debug.getproto(f, idx, active?)
int debug_getprotos(lua_State* L);    // debug.getprotos(f)
int debug_getstack(lua_State* L);     // debug.getstack(level, idx?)
int debug_getupvalue(lua_State* L);   // debug.getupvalue(f, idx)
int debug_getupvalues(lua_State* L);  // debug.getupvalues(f)
int debug_setconstant(lua_State* L);  // debug.setconstant(f, idx, value)
int debug_setstack(lua_State* L);     // debug.setstack(level, idx, value)
int debug_setupvalue(lua_State* L);   // debug.setupvalue(f, idx, value)

// ─── DRAWING ────────────────────────────────────────────────────────────────
int Drawing_new(lua_State* L);        // Drawing.new(type) -> DrawingObject
int Drawing_clear(lua_State* L);      // cleardrawcache()
int getrenderproperty(lua_State* L);  // getrenderproperty(obj, prop)
int setrenderproperty(lua_State* L);  // setrenderproperty(obj, prop, val)
int isrenderobj(lua_State* L);        // isrenderobj(obj)

// ─── FILESYSTEM ─────────────────────────────────────────────────────────────
int readfile(lua_State* L);           // readfile(path)
int writefile(lua_State* L);          // writefile(path, data)
int appendfile(lua_State* L);         // appendfile(path, data)
int loadfile_custom(lua_State* L);    // loadfile(path)
int listfiles(lua_State* L);         // listfiles(dir?)
int isfile(lua_State* L);            // isfile(path)
int isfolder(lua_State* L);          // isfolder(path)
int makefolder(lua_State* L);        // makefolder(path)
int delfolder(lua_State* L);         // delfolder(path)
int delfile(lua_State* L);           // delfile(path)
int dofile_custom(lua_State* L);      // dofile(path)

// ─── INPUT ──────────────────────────────────────────────────────────────────
int isrbxactive(lua_State* L);        // isrbxactive() / isgameactive()
int mouse1click(lua_State* L);        // mouse1click()
int mouse1press(lua_State* L);        // mouse1press()
int mouse1release(lua_State* L);      // mouse1release()
int mouse2click(lua_State* L);        // mouse2click()
int mouse2press(lua_State* L);        // mouse2press()
int mouse2release(lua_State* L);      // mouse2release()
int mousemoveabs(lua_State* L);       // mousemoveabs(x, y)
int mousemoverel(lua_State* L);       // mousemoverel(dx, dy)
int mousescroll(lua_State* L);        // mousescroll(pixels)
int keypress(lua_State* L);           // keypress(keyCode)
int keyrelease(lua_State* L);         // keyrelease(keyCode)
int keyclick(lua_State* L);           // keyclick(keyCode)

// ─── INSTANCES ──────────────────────────────────────────────────────────────
int fireclickdetector(lua_State* L);  // fireclickdetector(cd, dist?, player?)
int fireproximityprompt(lua_State* L);// fireproximityprompt(prompt)
int firetouchinterest(lua_State* L);  // firetouchinterest(part, otherPart, toggle)
int firesignal(lua_State* L);        // firesignal(signal, args...)
int getconnections(lua_State* L);     // getconnections(signal)
int gethiddenproperty(lua_State* L);  // gethiddenproperty(inst, prop)
int sethiddenproperty(lua_State* L);  // sethiddenproperty(inst, prop, val)
int gethui(lua_State* L);            // gethui()
int getinstances(lua_State* L);       // getinstances()
int getnilinstances(lua_State* L);    // getnilinstances()

// ─── METATABLE ──────────────────────────────────────────────────────────────
int getrawmetatable(lua_State* L);    // getrawmetatable(obj)
int hookmetamethod(lua_State* L);     // hookmetamethod(obj, mm, hook)
int getnamecallmethod(lua_State* L);  // getnamecallmethod()
int setnamecallmethod(lua_State* L);  // setnamecallmethod(method)
int setrawmetatable(lua_State* L);    // setrawmetatable(obj, mt)
int setreadonly(lua_State* L);        // setreadonly(t, readonly)
int isreadonly(lua_State* L);         // isreadonly(t)

// ─── MISC ───────────────────────────────────────────────────────────────────
int identifyexecutor(lua_State* L);   // identifyexecutor() -> name, version
int getexecutorname(lua_State* L);    // getexecutorname()
int lz4compress(lua_State* L);        // lz4compress(data)
int lz4decompress(lua_State* L);      // lz4decompress(data, origSize)
int messagebox(lua_State* L);         // messagebox(text, caption, flags)
int request(lua_State* L);            // request({Url, Method, Headers?, Body?})
int setclipboard(lua_State* L);       // setclipboard(text) / toclipboard
int setfpscap(lua_State* L);          // setfpscap(fps)
int getfpscap(lua_State* L);          // getfpscap()

// ─── SCRIPTS ────────────────────────────────────────────────────────────────
int getgc(lua_State* L);             // getgc(includeTables?)
int getgenv(lua_State* L);           // getgenv()
int getreg(lua_State* L);            // getreg()
int getrenv(lua_State* L);           // getrenv()
int getscriptbytecode(lua_State* L);  // getscriptbytecode(script) / dumpstring
int getscripthash(lua_State* L);      // getscripthash(script)
int getscripts(lua_State* L);        // getscripts()
int getrunningscripts(lua_State* L);  // getrunningscripts()
int getloadedmodules(lua_State* L);   // getloadedmodules()
int getsenv(lua_State* L);           // getsenv(script)
int getscriptclosure(lua_State* L);   // getscriptclosure(script)
int getthreadidentity(lua_State* L);  // getthreadidentity() / getidentity
int setthreadidentity(lua_State* L);  // setthreadidentity(id) / setidentity

// ─── WEBSOCKET ──────────────────────────────────────────────────────────────
int WebSocket_connect(lua_State* L);  // WebSocket.connect(url)

// ─── TASK LIBRARY (task.*) ──────────────────────────────────────────────────
int task_cancel(lua_State* L);        // task.cancel(thread)
int task_defer(lua_State* L);         // task.defer(f, args...)
int task_delay(lua_State* L);         // task.delay(duration, f, args...)
int task_spawn(lua_State* L);         // task.spawn(f, args...)
int task_wait(lua_State* L);          // task.wait(duration?)

} // namespace UNCFunctions


// ============================================================================
// Registration helpers
// ============================================================================

// Master table of all custom (UNC) functions and which library they belong to.
// library == nullptr means register as a global.
inline const std::vector<FunctionEntry>& GetUNCRegistry() {
    static const std::vector<FunctionEntry> registry = {
        // ── Cache ───────────────────────────────────────
        { "cache",   "invalidate",       UNCFunctions::cache_invalidate,       "Invalidate cached instance ref"  },
        { "cache",   "iscached",         UNCFunctions::cache_iscached,         "Check if instance is cached"     },
        { "cache",   "replace",          UNCFunctions::cache_replace,          "Replace cached ref"              },
        { nullptr,   "cloneref",         UNCFunctions::cloneref,               "Clone instance reference"        },
        { nullptr,   "compareinstances", UNCFunctions::compareinstances,       "Compare two instance refs"       },

        // ── Closures ────────────────────────────────────
        { nullptr,   "checkcaller",        UNCFunctions::checkcaller,          "Check if caller is executor"     },
        { nullptr,   "clonefunction",      UNCFunctions::clonefunction,        "Clone a function"                },
        { nullptr,   "getcallingscript",   UNCFunctions::getcallingscript,     "Get the calling script"          },
        { nullptr,   "hookfunction",       UNCFunctions::hookfunction,         "Hook/detour a function"          },
        { nullptr,   "iscclosure",         UNCFunctions::iscclosure,           "Check if C closure"              },
        { nullptr,   "islclosure",         UNCFunctions::islclosure,           "Check if Luau closure"           },
        { nullptr,   "isexecutorclosure",  UNCFunctions::isexecutorclosure,    "Check if executor closure"       },
        { nullptr,   "checkclosure",       UNCFunctions::isexecutorclosure,    "Alias: isexecutorclosure"        },
        { nullptr,   "loadstring",         UNCFunctions::loadstring_custom,    "Compile+load Luau source"        },
        { nullptr,   "newcclosure",        UNCFunctions::newcclosure,          "Wrap as C closure"               },
        { nullptr,   "newlclosure",        UNCFunctions::newlclosure,          "Wrap as Luau closure"            },

        // ── Console ─────────────────────────────────────
        { nullptr,   "rconsoleclear",      UNCFunctions::rconsoleclear,        "Clear console"                   },
        { nullptr,   "rconsolecreate",     UNCFunctions::rconsolecreate,       "Create console window"           },
        { nullptr,   "rconsoledestroy",    UNCFunctions::rconsoledestroy,      "Destroy console window"          },
        { nullptr,   "rconsoleinput",      UNCFunctions::rconsoleinput,        "Read console input"              },
        { nullptr,   "rconsoleprint",      UNCFunctions::rconsoleprint,        "Print to console"                },
        { nullptr,   "rconsolesettitle",   UNCFunctions::rconsolesettitle,     "Set console title"               },

        // ── Crypto ──────────────────────────────────────
        { "crypt",   "base64encode",     UNCFunctions::crypt_base64encode,     "Base64 encode"                   },
        { "crypt",   "base64decode",     UNCFunctions::crypt_base64decode,     "Base64 decode"                   },
        { "crypt",   "encrypt",          UNCFunctions::crypt_encrypt,          "Encrypt data"                    },
        { "crypt",   "decrypt",          UNCFunctions::crypt_decrypt,          "Decrypt data"                    },
        { "crypt",   "generatebytes",    UNCFunctions::crypt_generatebytes,    "Generate random bytes"           },
        { "crypt",   "generatekey",      UNCFunctions::crypt_generatekey,      "Generate encryption key"         },
        { "crypt",   "hash",             UNCFunctions::crypt_hash,             "Hash data"                       },
        { "crypt",   "hmac",             UNCFunctions::crypt_hmac,             "HMAC sign data"                  },

        // ── Debug (custom extensions) ───────────────────
        { "debug",   "getconstant",      UNCFunctions::debug_getconstant,      "Get proto constant"              },
        { "debug",   "getconstants",     UNCFunctions::debug_getconstants,     "Get all proto constants"         },
        { "debug",   "getinfo",          UNCFunctions::debug_getinfo_custom,   "Extended debug info"             },
        { "debug",   "getproto",         UNCFunctions::debug_getproto,         "Get child proto"                 },
        { "debug",   "getprotos",        UNCFunctions::debug_getprotos,        "Get all child protos"            },
        { "debug",   "getstack",         UNCFunctions::debug_getstack,         "Get stack value"                 },
        { "debug",   "getupvalue",       UNCFunctions::debug_getupvalue,       "Get upvalue"                     },
        { "debug",   "getupvalues",      UNCFunctions::debug_getupvalues,      "Get all upvalues"                },
        { "debug",   "setconstant",      UNCFunctions::debug_setconstant,      "Set proto constant"              },
        { "debug",   "setstack",         UNCFunctions::debug_setstack,         "Set stack value"                 },
        { "debug",   "setupvalue",       UNCFunctions::debug_setupvalue,       "Set upvalue"                     },

        // ── Drawing ─────────────────────────────────────
        { "Drawing", "new",              UNCFunctions::Drawing_new,            "Create drawing object"           },
        { nullptr,   "cleardrawcache",   UNCFunctions::Drawing_clear,          "Clear draw cache"                },
        { nullptr,   "getrenderproperty",UNCFunctions::getrenderproperty,      "Get render property"             },
        { nullptr,   "setrenderproperty",UNCFunctions::setrenderproperty,      "Set render property"             },
        { nullptr,   "isrenderobj",      UNCFunctions::isrenderobj,            "Check if render object"          },

        // ── Filesystem ──────────────────────────────────
        { nullptr,   "readfile",         UNCFunctions::readfile,               "Read file contents"              },
        { nullptr,   "writefile",        UNCFunctions::writefile,              "Write data to file"              },
        { nullptr,   "appendfile",       UNCFunctions::appendfile,             "Append data to file"             },
        { nullptr,   "loadfile",         UNCFunctions::loadfile_custom,        "Load+compile file"               },
        { nullptr,   "listfiles",        UNCFunctions::listfiles,              "List directory contents"         },
        { nullptr,   "isfile",           UNCFunctions::isfile,                 "Check if path is file"           },
        { nullptr,   "isfolder",         UNCFunctions::isfolder,               "Check if path is folder"         },
        { nullptr,   "makefolder",       UNCFunctions::makefolder,             "Create directory"                },
        { nullptr,   "delfolder",        UNCFunctions::delfolder,              "Delete directory"                },
        { nullptr,   "delfile",          UNCFunctions::delfile,                "Delete file"                     },
        { nullptr,   "dofile",           UNCFunctions::dofile_custom,          "Execute file"                    },

        // ── Input ───────────────────────────────────────
        { nullptr,   "isrbxactive",      UNCFunctions::isrbxactive,            "Check if game is focused"        },
        { nullptr,   "isgameactive",     UNCFunctions::isrbxactive,            "Alias: isrbxactive"              },
        { nullptr,   "mouse1click",      UNCFunctions::mouse1click,            "Left mouse click"                },
        { nullptr,   "mouse1press",      UNCFunctions::mouse1press,            "Left mouse press"                },
        { nullptr,   "mouse1release",    UNCFunctions::mouse1release,          "Left mouse release"              },
        { nullptr,   "mouse2click",      UNCFunctions::mouse2click,            "Right mouse click"               },
        { nullptr,   "mouse2press",      UNCFunctions::mouse2press,            "Right mouse press"               },
        { nullptr,   "mouse2release",    UNCFunctions::mouse2release,          "Right mouse release"             },
        { nullptr,   "mousemoveabs",     UNCFunctions::mousemoveabs,           "Move mouse absolute"             },
        { nullptr,   "mousemoverel",     UNCFunctions::mousemoverel,           "Move mouse relative"             },
        { nullptr,   "mousescroll",      UNCFunctions::mousescroll,            "Scroll mouse"                    },
        { nullptr,   "keypress",         UNCFunctions::keypress,               "Press key"                       },
        { nullptr,   "keyrelease",       UNCFunctions::keyrelease,             "Release key"                     },
        { nullptr,   "keyclick",         UNCFunctions::keyclick,               "Click key"                       },

        // ── Instances ───────────────────────────────────
        { nullptr,   "fireclickdetector",   UNCFunctions::fireclickdetector,   "Fire click detector"             },
        { nullptr,   "fireproximityprompt", UNCFunctions::fireproximityprompt, "Fire proximity prompt"           },
        { nullptr,   "firetouchinterest",   UNCFunctions::firetouchinterest,   "Fire touch interest"             },
        { nullptr,   "firesignal",          UNCFunctions::firesignal,          "Fire signal"                     },
        { nullptr,   "getconnections",      UNCFunctions::getconnections,      "Get signal connections"          },
        { nullptr,   "gethiddenproperty",   UNCFunctions::gethiddenproperty,   "Get hidden property"             },
        { nullptr,   "sethiddenproperty",   UNCFunctions::sethiddenproperty,   "Set hidden property"             },
        { nullptr,   "gethui",              UNCFunctions::gethui,              "Get hidden UI container"         },
        { nullptr,   "getinstances",        UNCFunctions::getinstances,        "Get all instances"               },
        { nullptr,   "getnilinstances",     UNCFunctions::getnilinstances,     "Get nil-parented instances"      },

        // ── Metatable ───────────────────────────────────
        { nullptr,   "getrawmetatable",    UNCFunctions::getrawmetatable,      "Get raw metatable"               },
        { nullptr,   "hookmetamethod",     UNCFunctions::hookmetamethod,       "Hook metamethod"                 },
        { nullptr,   "getnamecallmethod",  UNCFunctions::getnamecallmethod,    "Get namecall method name"        },
        { nullptr,   "setnamecallmethod",  UNCFunctions::setnamecallmethod,    "Set namecall method name"        },
        { nullptr,   "setrawmetatable",    UNCFunctions::setrawmetatable,      "Set raw metatable"               },
        { nullptr,   "setreadonly",        UNCFunctions::setreadonly,           "Set table readonly"              },
        { nullptr,   "isreadonly",         UNCFunctions::isreadonly,            "Check if readonly"               },

        // ── Misc ────────────────────────────────────────
        { nullptr,   "identifyexecutor",   UNCFunctions::identifyexecutor,     "Return executor name+version"    },
        { nullptr,   "getexecutorname",    UNCFunctions::getexecutorname,      "Return executor name"            },
        { nullptr,   "lz4compress",        UNCFunctions::lz4compress,          "LZ4 compress"                    },
        { nullptr,   "lz4decompress",      UNCFunctions::lz4decompress,       "LZ4 decompress"                  },
        { nullptr,   "messagebox",         UNCFunctions::messagebox,           "Show message box"                },
        { nullptr,   "request",            UNCFunctions::request,              "HTTP request"                    },
        { nullptr,   "http_request",       UNCFunctions::request,              "Alias: request"                  },
        { nullptr,   "setclipboard",       UNCFunctions::setclipboard,         "Set clipboard text"              },
        { nullptr,   "toclipboard",        UNCFunctions::setclipboard,         "Alias: setclipboard"             },
        { nullptr,   "setfpscap",          UNCFunctions::setfpscap,            "Set FPS cap"                     },
        { nullptr,   "getfpscap",          UNCFunctions::getfpscap,            "Get FPS cap"                     },

        // ── Scripts ─────────────────────────────────────
        { nullptr,   "getgc",              UNCFunctions::getgc,               "Get GC objects"                   },
        { nullptr,   "getgenv",            UNCFunctions::getgenv,             "Get global environment"           },
        { nullptr,   "getreg",             UNCFunctions::getreg,              "Get Lua registry"                 },
        { nullptr,   "getrenv",            UNCFunctions::getrenv,             "Get game environment"             },
        { nullptr,   "getscriptbytecode",  UNCFunctions::getscriptbytecode,   "Get script bytecode"              },
        { nullptr,   "dumpstring",         UNCFunctions::getscriptbytecode,   "Alias: getscriptbytecode"         },
        { nullptr,   "getscripthash",      UNCFunctions::getscripthash,       "Get script hash"                  },
        { nullptr,   "getscripts",         UNCFunctions::getscripts,          "Get all scripts"                  },
        { nullptr,   "getrunningscripts",  UNCFunctions::getrunningscripts,   "Get running scripts"              },
        { nullptr,   "getloadedmodules",   UNCFunctions::getloadedmodules,    "Get loaded modules"               },
        { nullptr,   "getsenv",            UNCFunctions::getsenv,             "Get script environment"           },
        { nullptr,   "getscriptclosure",   UNCFunctions::getscriptclosure,    "Get script closure"               },
        { nullptr,   "getthreadidentity",  UNCFunctions::getthreadidentity,   "Get thread identity level"        },
        { nullptr,   "getidentity",        UNCFunctions::getthreadidentity,   "Alias: getthreadidentity"         },
        { nullptr,   "setthreadidentity",  UNCFunctions::setthreadidentity,   "Set thread identity level"        },
        { nullptr,   "setidentity",        UNCFunctions::setthreadidentity,   "Alias: setthreadidentity"         },

        // ── WebSocket ───────────────────────────────────
        { "WebSocket","connect",           UNCFunctions::WebSocket_connect,    "Connect WebSocket"                },

        // ── Task ────────────────────────────────────────
        { "task",    "cancel",             UNCFunctions::task_cancel,          "Cancel deferred thread"           },
        { "task",    "defer",              UNCFunctions::task_defer,           "Defer function to next resumption"},
        { "task",    "delay",              UNCFunctions::task_delay,           "Delay function execution"         },
        { "task",    "spawn",              UNCFunctions::task_spawn,           "Spawn function on new thread"     },
        { "task",    "wait",               UNCFunctions::task_wait,            "Yield for duration"               },
    };
    return registry;
}

#endif // included via pragma once
