#pragma once
// ============================================================================
// UNCLibrary.h — Complete UNC (Unified Naming Convention) Function Registry
//
// This file defines EVERY standard UNC function and library that a fully
// compliant Luau executor should expose.  Each function is declared as a
// lua_CFunction so it can be registered into the Luau environment.
//
// Reference: UNC specification (latest)
// https://github.com/unified-naming-convention/NamingStandard
// ============================================================================

#include <string>
#include <vector>
#include <functional>

// Forward-declare the Luau C types we need.
struct lua_State;
using lua_CFunction = int (*)(lua_State* L);

// ============================================================================
// Section 1 — GLOBAL FUNCTIONS
// ============================================================================

namespace UNC::Globals {

// ── Console / Output ────────────────────────────────────────────────────────
int rconsoleprint(lua_State* L);      // print to executor console
int rconsoleclear(lua_State* L);      // clear executor console
int rconsolecreate(lua_State* L);     // create/show console window
int rconsoledestroy(lua_State* L);    // destroy console window
int rconsoleinput(lua_State* L);      // read line from console
int rconsoleinfo(lua_State* L);       // print info-level message
int rconsolewarn(lua_State* L);       // print warning-level message
int rconsoleerr(lua_State* L);        // print error-level message
int rconsoletitle(lua_State* L);      // set console window title
int printidentity(lua_State* L);      // print current thread identity

// ── Script Execution ────────────────────────────────────────────────────────
int loadstring(lua_State* L);         // compile + load Luau source string
int identifyexecutor(lua_State* L);   // returns executor name + version
int getexecutorname(lua_State* L);    // returns executor name
int lz4compress(lua_State* L);        // LZ4 compress
int lz4decompress(lua_State* L);      // LZ4 decompress
int messagebox(lua_State* L);         // native message box
int setclipboard(lua_State* L);       // copy string to clipboard
int getclipboard(lua_State* L);       // read string from clipboard
int setfpscap(lua_State* L);          // set FPS cap
int getfpscap(lua_State* L);          // get FPS cap
int isexecutorclosure(lua_State* L);  // is closure from executor?
int checkcaller(lua_State* L);        // is calling thread executor-owned?
int islclosure(lua_State* L);         // is Luau closure?
int iscclosure(lua_State* L);         // is C closure?
int request(lua_State* L);            // HTTP request (table-based)
int httpget(lua_State* L);            // simple HTTP GET
int httppost(lua_State* L);           // simple HTTP POST

// ── Globals manipulation ────────────────────────────────────────────────────
int getgenv(lua_State* L);            // get executor global environment table
int getrenv(lua_State* L);            // get game (real) global environment
int getsenv(lua_State* L);            // get script environment for a given script
int getmenv(lua_State* L);            // get ModuleScript environment (alias)
int getreg(lua_State* L);             // get Lua registry table
int getgc(lua_State* L);              // snapshot of all GC objects
int filtergc(lua_State* L);           // filtered snapshot of GC objects
int getinstances(lua_State* L);       // get all Instance userdata
int getnilinstances(lua_State* L);    // get Instance userdata with nil parent
int getscripts(lua_State* L);         // get all running Script instances
int getrunningscripts(lua_State* L);  // alias for getscripts
int getloadedmodules(lua_State* L);   // get all loaded ModuleScripts
int getconnections(lua_State* L);     // get connections on a Signal
int firesignal(lua_State* L);         // fire all connections on a Signal
int fireclickdetector(lua_State* L);  // fire ClickDetector
int firetouchinterest(lua_State* L);  // fire TouchInterest
int fireproximityprompt(lua_State* L);// fire ProximityPrompt
int isnetworkowner(lua_State* L);     // check network ownership

// ── Thread identity ─────────────────────────────────────────────────────────
int getthreadidentity(lua_State* L);  // get current thread identity level
int setthreadidentity(lua_State* L);  // set current thread identity level

// ── Hooking ─────────────────────────────────────────────────────────────────
int hookfunction(lua_State* L);       // replace a function, return original
int hookmetamethod(lua_State* L);     // hook a metamethod
int restorefunction(lua_State* L);    // undo hookfunction
int newcclosure(lua_State* L);        // wrap Luau closure as C closure
int newlclosure(lua_State* L);        // wrap C closure as Luau closure
int clonefunction(lua_State* L);      // clone a closure
int cloneref(lua_State* L);           // clone an Instance reference
int compareinstances(lua_State* L);   // compare two Instance refs by pointer

// ── Misc ────────────────────────────────────────────────────────────────────
int getnamecallmethod(lua_State* L);  // get current __namecall method string
int setnamecallmethod(lua_State* L);  // override __namecall method string
int gethiddenproperty(lua_State* L);  // read a hidden property
int sethiddenproperty(lua_State* L);  // write a hidden property
int setsimulationradius(lua_State* L);// set physics simulation radius
int isreadonly(lua_State* L);         // is table readonly?
int setreadonly(lua_State* L);        // set table readonly flag
int setrawmetatable(lua_State* L);    // set metatable bypassing __metatable
int getrawmetatable(lua_State* L);    // get real metatable ignoring __metatable
int gethui(lua_State* L);            // get hidden UI container

} // namespace UNC::Globals

// ============================================================================
// Section 2 — CACHE LIBRARY (cache.*)
// ============================================================================

namespace UNC::Cache {

int invalidate(lua_State* L);   // cache.invalidate(instance)
int iscached(lua_State* L);     // cache.iscached(instance) -> bool
int replace(lua_State* L);      // cache.replace(instance, newRef)

} // namespace UNC::Cache

// ============================================================================
// Section 3 — CLOSURES LIBRARY (closures.* / debug.*)
// ============================================================================

namespace UNC::Closures {

int getinfo(lua_State* L);          // closure info table
int getproto(lua_State* L);         // get inner Proto closures
int getprotos(lua_State* L);        // get all proto closures
int getconstant(lua_State* L);      // read constant by index
int getconstants(lua_State* L);     // read all constants
int setconstant(lua_State* L);      // write constant by index
int getupvalue(lua_State* L);       // read upvalue by index
int getupvalues(lua_State* L);      // read all upvalues
int setupvalue(lua_State* L);       // write upvalue by index
int getstack(lua_State* L);         // read stack frame value
int setstack(lua_State* L);         // write stack frame value

} // namespace UNC::Closures

// ============================================================================
// Section 4 — CRYPT LIBRARY (crypt.*)
// ============================================================================

namespace UNC::Crypt {

int base64encode(lua_State* L);     // crypt.base64encode / crypt.base64.encode
int base64decode(lua_State* L);     // crypt.base64decode / crypt.base64.decode
int encrypt(lua_State* L);          // crypt.encrypt(data, key, iv?, algo?)
int decrypt(lua_State* L);          // crypt.decrypt(data, key, iv?, algo?)
int hash(lua_State* L);             // crypt.hash(data, algo)
int hmac(lua_State* L);             // crypt.hmac(data, key, algo)
int generatebytes(lua_State* L);    // crypt.generatebytes(count)
int generatekey(lua_State* L);      // crypt.generatekey()
int random(lua_State* L);           // crypt.random(min?, max?)

} // namespace UNC::Crypt

// ============================================================================
// Section 5 — DEBUG LIBRARY (debug.*)
// ============================================================================

namespace UNC::Debug {

int getconstant(lua_State* L);
int getconstants(lua_State* L);
int setconstant(lua_State* L);
int getupvalue(lua_State* L);
int getupvalues(lua_State* L);
int setupvalue(lua_State* L);
int getstack(lua_State* L);
int setstack(lua_State* L);
int getinfo(lua_State* L);
int getproto(lua_State* L);
int getprotos(lua_State* L);
int getregistry(lua_State* L);
int setmetatable(lua_State* L);
int getmetatable(lua_State* L);
int traceback(lua_State* L);
int profilebegin(lua_State* L);
int profileend(lua_State* L);

} // namespace UNC::Debug

// ============================================================================
// Section 6 — DRAWING LIBRARY (Drawing.new, cleardrawcache, etc.)
// ============================================================================

namespace UNC::Drawing {

// Drawing object types: Line, Text, Image, Square, Quad, Circle, Triangle
int newDrawing(lua_State* L);        // Drawing.new(type) -> DrawingObject
int clearDrawCache(lua_State* L);    // cleardrawcache()
int getRenderedFrames(lua_State* L); // Drawing.Fonts / rendered frame count
int isRenderingAvailable(lua_State* L); // isrenderobj(obj)

} // namespace UNC::Drawing

// ============================================================================
// Section 7 — FILESYSTEM LIBRARY (readfile, writefile, etc.)
// ============================================================================

namespace UNC::Filesystem {

int readfile(lua_State* L);          // readfile(path) -> string
int writefile(lua_State* L);         // writefile(path, content)
int appendfile(lua_State* L);        // appendfile(path, content)
int loadfile(lua_State* L);          // loadfile(path) -> function
int listfiles(lua_State* L);         // listfiles(dir?) -> {paths}
int isfile(lua_State* L);            // isfile(path) -> bool
int isfolder(lua_State* L);          // isfolder(path) -> bool
int makefolder(lua_State* L);        // makefolder(path)
int delfolder(lua_State* L);         // delfolder(path)
int delfile(lua_State* L);           // delfile(path)
int dofile(lua_State* L);            // dofile(path) — load + execute
int getcustomasset(lua_State* L);    // getcustomasset(path) -> rbxasset url

} // namespace UNC::Filesystem

// ============================================================================
// Section 8 — INPUT LIBRARY
// ============================================================================

namespace UNC::Input {

int isrbxactive(lua_State* L);       // isrbxactive() / isgameactive()
int mouse1click(lua_State* L);       // simulate left click
int mouse1press(lua_State* L);
int mouse1release(lua_State* L);
int mouse2click(lua_State* L);       // simulate right click
int mouse2press(lua_State* L);
int mouse2release(lua_State* L);
int mousescroll(lua_State* L);       // simulate scroll
int mousemoverel(lua_State* L);      // move mouse relative
int mousemoveabs(lua_State* L);      // move mouse absolute
int keypress(lua_State* L);          // simulate key press
int keyrelease(lua_State* L);        // simulate key release
int keyclick(lua_State* L);          // press + release

} // namespace UNC::Input

// ============================================================================
// Section 9 — METATABLE LIBRARY
// ============================================================================

namespace UNC::Metatable {

int getrawmetatable(lua_State* L);
int setrawmetatable(lua_State* L);
int setreadonly(lua_State* L);
int isreadonly(lua_State* L);
int hookmetamethod(lua_State* L);
int getnamecallmethod(lua_State* L);
int setnamecallmethod(lua_State* L);

} // namespace UNC::Metatable

// ============================================================================
// Section 10 — WEBSOCKET LIBRARY (WebSocket.connect)
// ============================================================================

namespace UNC::WebSocket {

int connect(lua_State* L);          // WebSocket.connect(url) -> ws object
// ws object methods: Send, Close; events: OnMessage, OnClose

} // namespace UNC::WebSocket

// ============================================================================
// Section 11 — ACTOR / PARALLEL LUAU
// ============================================================================

namespace UNC::Actor {

int getactors(lua_State* L);         // get all Actor instances
int run_on_actor(lua_State* L);      // execute script on specific Actor thread
int is_parallel(lua_State* L);       // check if current context is parallel

} // namespace UNC::Actor

// ============================================================================
// Registration helper — call this to push everything into a lua_State.
// ============================================================================

namespace UNC {

struct FunctionEntry {
    const char* name;
    lua_CFunction func;
};

struct LibraryEntry {
    const char* libraryName;              // nullptr for globals
    std::vector<FunctionEntry> functions;
};

// Returns the full registry of all UNC libraries + globals.
std::vector<LibraryEntry> GetFullRegistry();

// Registers everything into the given lua_State's global environment.
void RegisterAll(lua_State* L);

// UNC compliance test — returns list of missing function names.
std::vector<std::string> RunComplianceCheck(lua_State* L);

} // namespace UNC
