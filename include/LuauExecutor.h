#pragma once
// ============================================================================
// LuauExecutor.h
// Core executor engine — compiles Luau source, sets up the environment,
// and runs scripts inside your game's Luau VM.
// ============================================================================

#include <string>
#include <functional>
#include <vector>
#include <memory>

// Forward decls (replace with your actual Luau includes when integrating)
struct lua_State;
typedef int (*lua_CFunction)(lua_State* L);

// ============================================================================
// Executor configuration
// ============================================================================
struct ExecutorConfig {
    std::string name        = "MyExecutor";
    std::string version     = "1.0.0";
    int         identity    = 8;           // thread identity / permission level
    bool        enableDebug = true;        // expose debug.get*/set* extensions
    bool        enableFS    = true;        // expose filesystem functions
    bool        enableHTTP  = true;        // expose request / WebSocket
    bool        enableInput = true;        // expose mouse/key simulation
    bool        enableDraw  = true;        // expose Drawing library
    std::string workspace   = "./workspace"; // filesystem sandbox root
};

// ============================================================================
// Execution result
// ============================================================================
struct ExecutionResult {
    bool        success = false;
    std::string output;               // print / return output
    std::string error;                // error message if failed
    double      executionTimeMs = 0;  // wall-clock time
};

// ============================================================================
// LuauExecutor — main class
// ============================================================================
class LuauExecutor {
public:
    explicit LuauExecutor(const ExecutorConfig& config = {});
    ~LuauExecutor();

    // -- Lifecycle -----------------------------------------------------------
    bool Initialize(lua_State* gameState);   // attach to game's lua_State
    void Shutdown();

    // -- Execution -----------------------------------------------------------
    ExecutionResult Execute(const std::string& source,
                            const std::string& chunkName = "=executor");

    // -- Compilation ---------------------------------------------------------
    // Compiles Luau source to bytecode. Returns empty string on error.
    std::string Compile(const std::string& source,
                        int optimizationLevel = 1);

    // Loads compiled bytecode onto the VM stack as a closure.
    bool LoadBytecode(const std::string& bytecode,
                      const std::string& chunkName);

    // -- Environment ---------------------------------------------------------
    void SetupGlobalEnvironment();           // register all UNC functions
    void RegisterFunction(const char* library,
                          const char* name,
                          lua_CFunction func);
    void RegisterLibrary(const char* name,
                         const std::vector<std::pair<const char*, lua_CFunction>>& funcs);

    // -- Getters -------------------------------------------------------------
    lua_State*          GetState() const { return m_thread; }
    lua_State*          GetGameState() const { return m_gameState; }
    const ExecutorConfig& GetConfig() const { return m_config; }
    bool                IsInitialized() const { return m_initialized; }

    // -- Identity ------------------------------------------------------------
    void SetThreadIdentity(int level);
    int  GetThreadIdentity() const;

    // -- Callbacks (hook these for UI integration) ----------------------------
    using OutputCallback = std::function<void(const std::string& message)>;
    using ErrorCallback  = std::function<void(const std::string& error)>;
    void SetOutputCallback(OutputCallback cb) { m_onOutput = std::move(cb); }
    void SetErrorCallback(ErrorCallback cb)   { m_onError  = std::move(cb); }

private:
    // -- Internal helpers ----------------------------------------------------
    lua_State* CreateThread();
    void       SetupSandbox();
    void       InjectIdentity(int level);

    // -- State ---------------------------------------------------------------
    ExecutorConfig m_config;
    lua_State*     m_gameState   = nullptr;  // game's main lua_State
    lua_State*     m_thread      = nullptr;  // our executor thread
    bool           m_initialized = false;

    OutputCallback m_onOutput;
    ErrorCallback  m_onError;
};
