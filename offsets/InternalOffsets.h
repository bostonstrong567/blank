#pragma once
// ============================================================================
// InternalOffsets.h
// Offsets for internal (in-process) access to Luau VM structures.
// These are used when the executor is loaded inside the game process.
//
// HOW TO UPDATE:
//   1. Attach a debugger or use signature scanning on your game binary.
//   2. Locate the struct/field and record the byte offset from the base.
//   3. Replace the placeholder 0x0 values below with the real offsets.
//   4. Rebuild the executor.
// ============================================================================

namespace Offsets::Internal {

// ── lua_State offsets ───────────────────────────────────────────────────────
// The lua_State struct is the per-thread Luau VM state.
constexpr uintptr_t LuaState_Top           = 0x0;   // StkId top
constexpr uintptr_t LuaState_Base          = 0x0;   // StkId base
constexpr uintptr_t LuaState_Global        = 0x0;   // global_State*
constexpr uintptr_t LuaState_CallInfo      = 0x0;   // CallInfo* ci
constexpr uintptr_t LuaState_Stack         = 0x0;   // StkId stack
constexpr uintptr_t LuaState_StackLast     = 0x0;   // StkId stack_last
constexpr uintptr_t LuaState_OpenUpVal     = 0x0;   // UpVal* openupval
constexpr uintptr_t LuaState_NumCCalls     = 0x0;   // unsigned short nCcalls
constexpr uintptr_t LuaState_Status        = 0x0;   // uint8_t status
constexpr uintptr_t LuaState_SingleStep    = 0x0;   // bool singlestep
constexpr uintptr_t LuaState_IsActive      = 0x0;   // bool isactive
constexpr uintptr_t LuaState_UserData      = 0x0;   // void* userdata

// ── global_State offsets ────────────────────────────────────────────────────
// The global_State is shared across all threads in a Luau VM.
constexpr uintptr_t GlobalState_MainThread    = 0x0; // lua_State* mainthread
constexpr uintptr_t GlobalState_Registry      = 0x0; // TValue registry
constexpr uintptr_t GlobalState_StringTable   = 0x0; // stringtable strt
constexpr uintptr_t GlobalState_TotalBytes    = 0x0; // size_t totalbytes
constexpr uintptr_t GlobalState_GCThreshold   = 0x0; // size_t GCthreshold
constexpr uintptr_t GlobalState_GCState       = 0x0; // int gcstate
constexpr uintptr_t GlobalState_TMName        = 0x0; // TString* tmname[]
constexpr uintptr_t GlobalState_Callbacks     = 0x0; // lua_Callbacks cb

// ── TValue / tagged value offsets ───────────────────────────────────────────
constexpr uintptr_t TValue_Value     = 0x0;  // Value union
constexpr uintptr_t TValue_Extra     = 0x0;  // int extra[]
constexpr uintptr_t TValue_Tag       = 0x0;  // int tt (type tag)
constexpr size_t    TValue_Size      = 0x10; // sizeof(TValue) — typically 16 bytes

// ── Closure offsets ─────────────────────────────────────────────────────────
constexpr uintptr_t Closure_IsC         = 0x0;  // uint8_t isC
constexpr uintptr_t Closure_NUpvalues   = 0x0;  // uint8_t nupvalues
constexpr uintptr_t Closure_Env         = 0x0;  // Table* env
constexpr uintptr_t Closure_StackSize   = 0x0;  // uint8_t stacksize
constexpr uintptr_t Closure_Preload     = 0x0;  // uint8_t preload
constexpr uintptr_t Closure_CFunction   = 0x0;  // lua_CFunction f (C closures)
constexpr uintptr_t Closure_Proto       = 0x0;  // Proto* p (Luau closures)
constexpr uintptr_t Closure_Upvals      = 0x0;  // TValue upvals[] start

// ── Proto (function prototype) offsets ──────────────────────────────────────
constexpr uintptr_t Proto_Code          = 0x0;  // Instruction* code
constexpr uintptr_t Proto_SizeCode      = 0x0;  // int sizecode
constexpr uintptr_t Proto_Constants     = 0x0;  // TValue* k
constexpr uintptr_t Proto_SizeK         = 0x0;  // int sizek
constexpr uintptr_t Proto_Protos        = 0x0;  // Proto** p (child protos)
constexpr uintptr_t Proto_SizeP         = 0x0;  // int sizep
constexpr uintptr_t Proto_MaxStackSize  = 0x0;  // uint8_t maxstacksize
constexpr uintptr_t Proto_NumParams     = 0x0;  // uint8_t numparams
constexpr uintptr_t Proto_IsVarArg      = 0x0;  // uint8_t is_vararg
constexpr uintptr_t Proto_LineDefined   = 0x0;  // int linedefined
constexpr uintptr_t Proto_DebugName     = 0x0;  // TString* debugname
constexpr uintptr_t Proto_Bytecode      = 0x0;  // uint8_t* bytecode (serialized)
constexpr uintptr_t Proto_BytecodeSize  = 0x0;  // size_t bytecodeSize

// ── Table offsets ───────────────────────────────────────────────────────────
constexpr uintptr_t Table_Flags         = 0x0;  // uint8_t tmcache
constexpr uintptr_t Table_NodeLog2      = 0x0;  // uint8_t lsizenode
constexpr uintptr_t Table_SizeArray     = 0x0;  // int sizearray
constexpr uintptr_t Table_Array         = 0x0;  // TValue* array
constexpr uintptr_t Table_Node          = 0x0;  // LuaNode* node
constexpr uintptr_t Table_LastFree      = 0x0;  // LuaNode* lastfree
constexpr uintptr_t Table_MetaTable     = 0x0;  // Table* metatable
constexpr uintptr_t Table_GCList        = 0x0;  // GCObject* gclist
constexpr uintptr_t Table_ReadOnly      = 0x0;  // bool readonly
constexpr uintptr_t Table_SafeEnv       = 0x0;  // bool safeenv

// ── Userdata offsets ────────────────────────────────────────────────────────
constexpr uintptr_t Udata_MetaTable     = 0x0;  // Table* metatable
constexpr uintptr_t Udata_Tag           = 0x0;  // uint8_t tag
constexpr uintptr_t Udata_Len           = 0x0;  // int len
constexpr uintptr_t Udata_Data          = 0x0;  // start of user data blob

// ── TString offsets ─────────────────────────────────────────────────────────
constexpr uintptr_t TString_Hash        = 0x0;  // unsigned int hash
constexpr uintptr_t TString_Len         = 0x0;  // size_t len
constexpr uintptr_t TString_Data        = 0x0;  // char data[] (right after header)

// ── CallInfo offsets ────────────────────────────────────────────────────────
constexpr uintptr_t CallInfo_Base       = 0x0;  // StkId base
constexpr uintptr_t CallInfo_Func       = 0x0;  // StkId func
constexpr uintptr_t CallInfo_Top        = 0x0;  // StkId top
constexpr uintptr_t CallInfo_SavedPC    = 0x0;  // const Instruction* savedpc
constexpr uintptr_t CallInfo_Flags      = 0x0;  // int flags
constexpr uintptr_t CallInfo_NResults   = 0x0;  // int nresults

// ── UpVal offsets ───────────────────────────────────────────────────────────
constexpr uintptr_t UpVal_V             = 0x0;  // TValue* v (points to stack or own storage)
constexpr uintptr_t UpVal_U_Value       = 0x0;  // TValue value (closed value)
constexpr uintptr_t UpVal_U_Open_Prev   = 0x0;  // UpVal* prev (when open)
constexpr uintptr_t UpVal_U_Open_Next   = 0x0;  // UpVal* next (when open)

// ── VM function pointers (resolved at runtime via sig-scan or symbol) ──────
constexpr uintptr_t Func_LuauLoad        = 0x0; // luau_load()
constexpr uintptr_t Func_LuaVExecute     = 0x0; // luaV_execute()
constexpr uintptr_t Func_LuaDCall        = 0x0; // luaD_call()
constexpr uintptr_t Func_LuaDPCall       = 0x0; // luaD_pcall()
constexpr uintptr_t Func_LuaENewThread   = 0x0; // luaE_newthread()
constexpr uintptr_t Func_LuaCCheckStack  = 0x0; // luaC_checkstack()
constexpr uintptr_t Func_LuaFNewLClosure = 0x0; // luaF_newLclosure()
constexpr uintptr_t Func_LuaFNewCClosure = 0x0; // luaF_newCclosure()
constexpr uintptr_t Func_DeserializeBytecode = 0x0; // internal bytecode deserialize
constexpr uintptr_t Func_Compile         = 0x0; // luau_compile / Luau::compile

} // namespace Offsets::Internal
