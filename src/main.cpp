// ============================================================================
// main.cpp
// Entry point for the Luau Executor.
//
// In production, this would be injected into your game process (DLL/SO).
// For development/testing, this standalone binary creates its own Luau VM,
// registers all UNC functions, and runs scripts from stdin or a file.
// ============================================================================

#include "../include/LuauExecutor.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

// ============================================================================
// Forward decls for Luau API (same externs as the rest of the project).
// Replace with real includes when linking against Luau.
// ============================================================================
extern "C" {
    lua_State* lua_newstate(void* allocator, void* ud);
    void       lua_close(lua_State* L);
    void       luaL_openlibs(lua_State* L);
    void       luaL_sandboxthread(lua_State* L);
}


static void PrintUsage(const char* argv0) {
    std::cout << "Usage:\n"
              << "  " << argv0 << "              — interactive REPL\n"
              << "  " << argv0 << " <file.luau>  — execute a script file\n"
              << "  " << argv0 << " --test        — run built-in self-test\n"
              << std::endl;
}


// ============================================================================
// Interactive REPL
// ============================================================================
static void RunREPL(LuauExecutor& executor) {
    std::cout << "[" << executor.GetConfig().name
              << " v" << executor.GetConfig().version << "] REPL\n"
              << "Type Luau code. Enter a blank line to execute. Ctrl+D to quit.\n"
              << std::endl;

    std::string buffer;

    while (true) {
        std::cout << ">> ";
        std::string line;
        if (!std::getline(std::cin, line)) break; // EOF

        if (line.empty() && !buffer.empty()) {
            // Execute accumulated buffer
            auto result = executor.Execute(buffer);
            if (result.success) {
                if (!result.output.empty())
                    std::cout << result.output << "\n";
                std::cout << "[OK " << result.executionTimeMs << "ms]\n";
            } else {
                std::cerr << "[ERROR] " << result.error << "\n";
            }
            buffer.clear();
        } else {
            if (!buffer.empty()) buffer += "\n";
            buffer += line;
        }
    }
}


// ============================================================================
// Execute a file
// ============================================================================
static int RunFile(LuauExecutor& executor, const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Error: cannot open " << path << "\n";
        return 1;
    }

    std::ostringstream ss;
    ss << f.rdbuf();
    std::string source = ss.str();

    auto result = executor.Execute(source, "@" + path);
    if (result.success) {
        if (!result.output.empty())
            std::cout << result.output << "\n";
        return 0;
    } else {
        std::cerr << "[ERROR] " << result.error << "\n";
        return 1;
    }
}


// ============================================================================
// Built-in self-test (verifies environment is set up correctly)
// ============================================================================
static int RunSelfTest(LuauExecutor& executor) {
    std::cout << "Running self-test...\n";

    struct TestCase {
        const char* name;
        const char* source;
        bool shouldSucceed;
    };

    TestCase tests[] = {
        { "identifyexecutor",
          "local name, ver = identifyexecutor() assert(name ~= nil) return name",
          true },
        { "getgenv",
          "local env = getgenv() assert(type(env) == 'table') return 'ok'",
          true },
        { "isfile",
          "local r = isfile('nonexistent_test_file.txt') assert(r == false) return 'ok'",
          true },
        { "getthreadidentity",
          "local id = getthreadidentity() assert(type(id) == 'number') return tostring(id)",
          true },
        { "checkcaller",
          "assert(checkcaller() == true) return 'ok'",
          true },
        { "crypt.base64encode",
          "local e = crypt.base64encode('hello') assert(e == 'aGVsbG8=') return e",
          true },
        { "crypt.base64decode",
          "local d = crypt.base64decode('aGVsbG8=') assert(d == 'hello') return d",
          true },
        { "loadstring",
          "local f = loadstring('return 1+1') assert(f() == 2) return 'ok'",
          true },
        { "table operations",
          "local t = {1,2,3} assert(#t == 3) return 'ok'",
          true },
        { "error handling",
          "error('test error')",
          false },
    };

    int passed = 0;
    int failed = 0;
    int total  = sizeof(tests) / sizeof(tests[0]);

    for (const auto& tc : tests) {
        auto result = executor.Execute(tc.source, "=test:" + std::string(tc.name));

        bool ok = (result.success == tc.shouldSucceed);
        if (ok) {
            std::cout << "  [PASS] " << tc.name;
            if (result.success && !result.output.empty())
                std::cout << " -> " << result.output;
            std::cout << "\n";
            passed++;
        } else {
            std::cout << "  [FAIL] " << tc.name << " — expected "
                      << (tc.shouldSucceed ? "success" : "failure")
                      << ", got " << (result.success ? "success" : "failure");
            if (!result.error.empty()) std::cout << " (" << result.error << ")";
            std::cout << "\n";
            failed++;
        }
    }

    std::cout << "\nResults: " << passed << "/" << total << " passed";
    if (failed > 0) std::cout << ", " << failed << " failed";
    std::cout << "\n";

    return (failed == 0) ? 0 : 1;
}


// ============================================================================
// main
// ============================================================================
int main(int argc, char* argv[]) {
    // Create workspace directory if it doesn't exist
    std::filesystem::create_directories("./workspace");

    // Configuration
    ExecutorConfig config;
    config.name     = "MyExecutor";
    config.version  = "1.0.0";
    config.identity = 8;

    // Create a standalone Luau VM for testing.
    // In production, you'd attach to the game's existing lua_State instead.
    lua_State* L = lua_newstate(nullptr, nullptr);
    if (!L) {
        std::cerr << "Failed to create Lua state\n";
        return 1;
    }
    luaL_openlibs(L);

    // Initialize executor
    LuauExecutor executor(config);
    executor.SetOutputCallback([](const std::string& msg) {
        std::cout << msg << "\n";
    });
    executor.SetErrorCallback([](const std::string& err) {
        std::cerr << "[Error] " << err << "\n";
    });

    if (!executor.Initialize(L)) {
        std::cerr << "Failed to initialize executor\n";
        lua_close(L);
        return 1;
    }

    int exitCode = 0;

    if (argc < 2) {
        RunREPL(executor);
    } else if (std::string(argv[1]) == "--test") {
        exitCode = RunSelfTest(executor);
    } else if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        PrintUsage(argv[0]);
    } else {
        exitCode = RunFile(executor, argv[1]);
    }

    executor.Shutdown();
    lua_close(L);
    return exitCode;
}
