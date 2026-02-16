#pragma once
// ============================================================================
// ExternalOffsets.h
// Offsets for external (out-of-process) access to game memory.
// Used when reading/writing game memory from a separate process.
//
// HOW TO UPDATE:
//   1. Use a memory scanner (e.g. ReClass, Cheat Engine) on the running game.
//   2. Find base addresses via module enumeration.
//   3. Record RVA (relative virtual address) from module base.
//   4. Replace placeholder 0x0 values below.
//   5. Rebuild the executor.
// ============================================================================

#include <cstdint>

namespace Offsets::External {

// ── Module base helpers ─────────────────────────────────────────────────────
// These are filled at runtime by enumerating loaded modules.
// Placeholders here are documentation only.
inline uintptr_t GameModuleBase   = 0x0;  // base address of the main game executable
inline uintptr_t LuauModuleBase   = 0x0;  // base if Luau is in a separate DLL/SO

// ── Global pointers (RVA from game module base) ─────────────────────────────
constexpr uintptr_t Ptr_GlobalState      = 0x0;  // -> global_State*
constexpr uintptr_t Ptr_MainLuaState     = 0x0;  // -> lua_State* (main thread)
constexpr uintptr_t Ptr_ScriptContext    = 0x0;  // -> ScriptContext* (if applicable)
constexpr uintptr_t Ptr_TaskScheduler    = 0x0;  // -> TaskScheduler* (if applicable)
constexpr uintptr_t Ptr_DataModel        = 0x0;  // -> DataModel / root game object

// ── ScriptContext offsets (from ScriptContext base) ─────────────────────────
constexpr uintptr_t ScriptContext_LuaState     = 0x0;  // lua_State* in ScriptContext
constexpr uintptr_t ScriptContext_ScriptStart  = 0x0;  // script identity / permissions
constexpr uintptr_t ScriptContext_ThreadStore  = 0x0;  // thread pool / store

// ── TaskScheduler offsets (from TaskScheduler base) ─────────────────────────
constexpr uintptr_t TaskScheduler_JobList       = 0x0;  // linked list of Jobs
constexpr uintptr_t TaskScheduler_FrameCount    = 0x0;  // frame/tick counter
constexpr uintptr_t TaskScheduler_FPS           = 0x0;  // current FPS value

// ── Job offsets (from Job base) ─────────────────────────────────────────────
constexpr uintptr_t Job_Name            = 0x0;   // std::string name
constexpr uintptr_t Job_State           = 0x0;   // enum JobState
constexpr uintptr_t Job_ScriptContext   = 0x0;   // ScriptContext* associated ctx
constexpr uintptr_t Job_Next            = 0x0;   // Job* next

// ── Instance / game object offsets ──────────────────────────────────────────
// For navigating your game's object tree externally.
constexpr uintptr_t Instance_Name       = 0x0;   // std::string / TString* name
constexpr uintptr_t Instance_ClassName  = 0x0;   // class descriptor / name
constexpr uintptr_t Instance_Parent     = 0x0;   // Instance* parent
constexpr uintptr_t Instance_Children   = 0x0;   // std::vector<Instance*> or linked list
constexpr uintptr_t Instance_Properties = 0x0;   // property table pointer

// ── Lua identity / security context ─────────────────────────────────────────
constexpr uintptr_t Identity_Level       = 0x0;  // current identity / permission level
constexpr uintptr_t Identity_ThreadRef   = 0x0;  // ref to associated lua_State

// ── Rendering / viewport (optional, for drawing overlays) ───────────────────
constexpr uintptr_t Render_ViewMatrix    = 0x0;  // float[16] view matrix
constexpr uintptr_t Render_ProjMatrix    = 0x0;  // float[16] projection matrix
constexpr uintptr_t Render_Viewport      = 0x0;  // viewport x,y,w,h
constexpr uintptr_t Render_Camera        = 0x0;  // Camera* / CFrame

// ── Memory read/write helper signatures ─────────────────────────────────────
// Byte patterns used to sig-scan for key functions in the game binary.
// Format: "AA BB CC ?? DD" where ?? is a wildcard byte.
namespace Signatures {
    constexpr const char* LuauLoad        = "";  // luau_load sig
    constexpr const char* LuaVExecute     = "";  // luaV_execute sig
    constexpr const char* LuaDCall        = "";  // luaD_call sig
    constexpr const char* LuaDPCall       = "";  // luaD_pcall sig
    constexpr const char* GetGlobalState  = "";  // global_State accessor
    constexpr const char* GetScriptContext= "";  // ScriptContext accessor
    constexpr const char* Deserialize     = "";  // bytecode deserializer
    constexpr const char* NewThread       = "";  // luaE_newthread
    constexpr const char* Compile         = "";  // Luau::compile
} // namespace Signatures

} // namespace Offsets::External
