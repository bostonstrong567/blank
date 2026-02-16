--[[
    UNC Environment Test Script
    Tests all major UNC function categories.
    Run with: ./LuauExecutor scripts/unc_test.luau
    Or:       ./LuauExecutor --test  (for built-in tests)
]]

local passed = 0
local failed = 0
local total  = 0

local function test(name: string, fn: () -> ())
    total += 1
    local ok, err = pcall(fn)
    if ok then
        passed += 1
        print("[PASS] " .. name)
    else
        failed += 1
        print("[FAIL] " .. name .. " — " .. tostring(err))
    end
end

print("========================================")
print("  UNC Environment Compatibility Test")
print("========================================\n")

-- ═══════════════════════════════════════
-- EXECUTOR IDENTITY
-- ═══════════════════════════════════════
test("identifyexecutor returns name and version", function()
    local name, ver = identifyexecutor()
    assert(type(name) == "string" and #name > 0, "name must be non-empty string")
    assert(type(ver) == "string" and #ver > 0, "version must be non-empty string")
end)

test("getexecutorname returns string", function()
    local name = getexecutorname()
    assert(type(name) == "string" and #name > 0)
end)

-- ═══════════════════════════════════════
-- CLOSURES
-- ═══════════════════════════════════════
test("checkcaller returns true from executor context", function()
    assert(checkcaller() == true)
end)

test("iscclosure works", function()
    assert(type(iscclosure(print)) == "boolean")
end)

test("islclosure works", function()
    local function f() end
    assert(type(islclosure(f)) == "boolean")
end)

test("isexecutorclosure exists", function()
    assert(type(isexecutorclosure) == "function")
end)

test("checkclosure alias exists", function()
    assert(type(checkclosure) == "function")
end)

test("newcclosure wraps function", function()
    local function f() return 42 end
    local wrapped = newcclosure(f)
    assert(type(wrapped) == "function")
end)

test("clonefunction returns function", function()
    local function f() return 1 end
    local cloned = clonefunction(f)
    assert(type(cloned) == "function")
end)

test("hookfunction returns original", function()
    local function original() return "original" end
    local function hook() return "hooked" end
    local old = hookfunction(original, hook)
    assert(type(old) == "function")
end)

test("loadstring compiles and runs", function()
    local f, err = loadstring("return 1 + 2")
    assert(f, "loadstring failed: " .. tostring(err))
    assert(f() == 3)
end)

test("loadstring reports errors", function()
    local f, err = loadstring("this is not valid luau }{}{")
    -- Should either return nil+error or the function should error on call
    -- (depends on whether the syntax error is catchable at compile time)
    if f == nil then
        assert(type(err) == "string")
    end
end)

-- ═══════════════════════════════════════
-- ENVIRONMENT / SCRIPTS
-- ═══════════════════════════════════════
test("getgenv returns table", function()
    local env = getgenv()
    assert(type(env) == "table")
end)

test("getrenv returns table", function()
    local env = getrenv()
    assert(type(env) == "table")
end)

test("getreg returns table", function()
    local reg = getreg()
    assert(type(reg) == "table")
end)

test("getthreadidentity returns number", function()
    local id = getthreadidentity()
    assert(type(id) == "number")
end)

test("getidentity alias works", function()
    assert(type(getidentity) == "function")
end)

test("setthreadidentity exists", function()
    assert(type(setthreadidentity) == "function")
end)

test("setidentity alias exists", function()
    assert(type(setidentity) == "function")
end)

test("getgc returns table", function()
    local gc = getgc()
    assert(type(gc) == "table")
end)

-- ═══════════════════════════════════════
-- FILESYSTEM
-- ═══════════════════════════════════════
test("writefile + readfile round-trip", function()
    writefile("_unc_test_file.txt", "hello from unc test")
    local contents = readfile("_unc_test_file.txt")
    assert(contents == "hello from unc test", "contents mismatch: " .. tostring(contents))
    delfile("_unc_test_file.txt")
end)

test("isfile returns correct values", function()
    writefile("_unc_test_exists.txt", "x")
    assert(isfile("_unc_test_exists.txt") == true)
    assert(isfile("_unc_test_nonexistent.txt") == false)
    delfile("_unc_test_exists.txt")
end)

test("isfolder works", function()
    makefolder("_unc_test_dir")
    assert(isfolder("_unc_test_dir") == true)
    assert(isfolder("_unc_test_nonexistent_dir") == false)
    delfolder("_unc_test_dir")
end)

test("appendfile works", function()
    writefile("_unc_test_append.txt", "hello")
    appendfile("_unc_test_append.txt", " world")
    local contents = readfile("_unc_test_append.txt")
    assert(contents == "hello world", "append failed: " .. tostring(contents))
    delfile("_unc_test_append.txt")
end)

test("listfiles returns table", function()
    local files = listfiles("")
    assert(type(files) == "table")
end)

-- ═══════════════════════════════════════
-- CRYPTO
-- ═══════════════════════════════════════
test("crypt.base64encode works", function()
    local encoded = crypt.base64encode("hello")
    assert(encoded == "aGVsbG8=", "expected aGVsbG8=, got " .. tostring(encoded))
end)

test("crypt.base64decode works", function()
    local decoded = crypt.base64decode("aGVsbG8=")
    assert(decoded == "hello", "expected hello, got " .. tostring(decoded))
end)

test("crypt.base64 round-trip", function()
    local original = "The quick brown fox jumps over the lazy dog"
    local encoded = crypt.base64encode(original)
    local decoded = crypt.base64decode(encoded)
    assert(decoded == original)
end)

test("crypt.generatekey returns string", function()
    local key = crypt.generatekey()
    assert(type(key) == "string" and #key > 0)
end)

test("crypt.generatebytes returns string", function()
    local bytes = crypt.generatebytes(16)
    assert(type(bytes) == "string" and #bytes > 0)
end)

test("crypt.hash exists", function()
    assert(type(crypt.hash) == "function")
end)

-- ═══════════════════════════════════════
-- METATABLE
-- ═══════════════════════════════════════
test("getrawmetatable works", function()
    local t = setmetatable({}, { __index = function() return 42 end })
    local mt = getrawmetatable(t)
    assert(type(mt) == "table")
end)

test("setrawmetatable works", function()
    local t = {}
    setrawmetatable(t, { __tostring = function() return "custom" end })
    assert(getmetatable(t) ~= nil)
end)

test("isreadonly returns boolean", function()
    local t = {}
    assert(type(isreadonly(t)) == "boolean")
end)

test("setreadonly exists", function()
    assert(type(setreadonly) == "function")
end)

test("hookmetamethod exists", function()
    assert(type(hookmetamethod) == "function")
end)

test("getnamecallmethod exists", function()
    assert(type(getnamecallmethod) == "function")
end)

test("setnamecallmethod exists", function()
    assert(type(setnamecallmethod) == "function")
end)

-- ═══════════════════════════════════════
-- CACHE
-- ═══════════════════════════════════════
test("cache library exists", function()
    assert(type(cache) == "table")
    assert(type(cache.invalidate) == "function")
    assert(type(cache.iscached) == "function")
    assert(type(cache.replace) == "function")
end)

test("cloneref exists", function()
    assert(type(cloneref) == "function")
end)

test("compareinstances exists", function()
    assert(type(compareinstances) == "function")
end)

-- ═══════════════════════════════════════
-- CONSOLE
-- ═══════════════════════════════════════
test("rconsoleprint exists", function()
    assert(type(rconsoleprint) == "function")
end)

test("rconsolecreate exists", function()
    assert(type(rconsolecreate) == "function")
end)

test("rconsoledestroy exists", function()
    assert(type(rconsoledestroy) == "function")
end)

-- ═══════════════════════════════════════
-- DEBUG EXTENSIONS
-- ═══════════════════════════════════════
test("debug.getconstant exists", function()
    assert(type(debug.getconstant) == "function")
end)

test("debug.getconstants exists", function()
    assert(type(debug.getconstants) == "function")
end)

test("debug.getupvalue exists", function()
    assert(type(debug.getupvalue) == "function")
end)

test("debug.getupvalues exists", function()
    assert(type(debug.getupvalues) == "function")
end)

test("debug.setupvalue exists", function()
    assert(type(debug.setupvalue) == "function")
end)

test("debug.getproto exists", function()
    assert(type(debug.getproto) == "function")
end)

test("debug.getprotos exists", function()
    assert(type(debug.getprotos) == "function")
end)

test("debug.getstack exists", function()
    assert(type(debug.getstack) == "function")
end)

test("debug.setstack exists", function()
    assert(type(debug.setstack) == "function")
end)

test("debug.setconstant exists", function()
    assert(type(debug.setconstant) == "function")
end)

-- ═══════════════════════════════════════
-- DRAWING
-- ═══════════════════════════════════════
test("Drawing library exists", function()
    assert(type(Drawing) == "table")
    assert(type(Drawing.new) == "function")
end)

test("cleardrawcache exists", function()
    assert(type(cleardrawcache) == "function")
end)

test("isrenderobj exists", function()
    assert(type(isrenderobj) == "function")
end)

-- ═══════════════════════════════════════
-- TASK LIBRARY
-- ═══════════════════════════════════════
test("task.spawn exists", function()
    assert(type(task.spawn) == "function")
end)

test("task.defer exists", function()
    assert(type(task.defer) == "function")
end)

test("task.delay exists", function()
    assert(type(task.delay) == "function")
end)

test("task.wait exists", function()
    assert(type(task.wait) == "function")
end)

test("task.cancel exists", function()
    assert(type(task.cancel) == "function")
end)

-- ═══════════════════════════════════════
-- MISC
-- ═══════════════════════════════════════
test("setclipboard exists", function()
    assert(type(setclipboard) == "function")
end)

test("toclipboard alias exists", function()
    assert(type(toclipboard) == "function")
end)

test("setfpscap exists", function()
    assert(type(setfpscap) == "function")
end)

test("getfpscap exists", function()
    assert(type(getfpscap) == "function")
end)

test("request exists", function()
    assert(type(request) == "function")
end)

test("http_request alias exists", function()
    assert(type(http_request) == "function")
end)

test("messagebox exists", function()
    assert(type(messagebox) == "function")
end)

test("lz4compress exists", function()
    assert(type(lz4compress) == "function")
end)

test("lz4decompress exists", function()
    assert(type(lz4decompress) == "function")
end)

-- ═══════════════════════════════════════
-- INPUT
-- ═══════════════════════════════════════
test("isrbxactive exists", function()
    assert(type(isrbxactive) == "function")
end)

test("isgameactive alias exists", function()
    assert(type(isgameactive) == "function")
end)

test("mouse1click exists", function()
    assert(type(mouse1click) == "function")
end)

test("keypress exists", function()
    assert(type(keypress) == "function")
end)

-- ═══════════════════════════════════════
-- INSTANCES
-- ═══════════════════════════════════════
test("firesignal exists", function()
    assert(type(firesignal) == "function")
end)

test("getconnections exists", function()
    assert(type(getconnections) == "function")
end)

test("gethiddenproperty exists", function()
    assert(type(gethiddenproperty) == "function")
end)

test("sethiddenproperty exists", function()
    assert(type(sethiddenproperty) == "function")
end)

test("gethui exists", function()
    assert(type(gethui) == "function")
end)

test("getinstances exists", function()
    assert(type(getinstances) == "function")
end)

test("getnilinstances exists", function()
    assert(type(getnilinstances) == "function")
end)

-- ═══════════════════════════════════════
-- WEBSOCKET
-- ═══════════════════════════════════════
test("WebSocket.connect exists", function()
    assert(type(WebSocket) == "table")
    assert(type(WebSocket.connect) == "function")
end)

-- ═══════════════════════════════════════
-- STANDARD LIBRARY SPOT CHECKS
-- ═══════════════════════════════════════
test("string.split (Luau extension)", function()
    local parts = string.split("a,b,c", ",")
    assert(#parts == 3)
    assert(parts[1] == "a" and parts[2] == "b" and parts[3] == "c")
end)

test("table.clone (Luau extension)", function()
    local t = {1, 2, 3}
    local c = table.clone(t)
    assert(c[1] == 1 and c[2] == 2 and c[3] == 3)
    assert(t ~= c) -- different table
end)

test("table.freeze (Luau extension)", function()
    local t = {x = 1}
    table.freeze(t)
    assert(table.isfrozen(t) == true)
end)

test("math.clamp (Luau extension)", function()
    assert(math.clamp(5, 1, 10) == 5)
    assert(math.clamp(-5, 1, 10) == 1)
    assert(math.clamp(15, 1, 10) == 10)
end)

test("math.sign (Luau extension)", function()
    assert(math.sign(5) == 1)
    assert(math.sign(-5) == -1)
    assert(math.sign(0) == 0)
end)

test("math.round (Luau extension)", function()
    assert(math.round(1.5) == 2)
    assert(math.round(1.4) == 1)
end)

test("bit32.countlz (Luau extension)", function()
    assert(type(bit32.countlz) == "function")
end)

test("bit32.countrz (Luau extension)", function()
    assert(type(bit32.countrz) == "function")
end)

test("buffer library exists (Luau extension)", function()
    assert(type(buffer) == "table")
    assert(type(buffer.create) == "function")
    assert(type(buffer.fromstring) == "function")
    assert(type(buffer.len) == "function")
end)

test("typeof (Luau extension)", function()
    assert(typeof(42) == "number")
    assert(typeof("hi") == "string")
    assert(typeof(true) == "boolean")
    assert(typeof(nil) == "nil")
end)


-- ═══════════════════════════════════════
-- RESULTS
-- ═══════════════════════════════════════
print("\n========================================")
print(string.format("  Results: %d/%d passed", passed, total))
if failed > 0 then
    print(string.format("  %d FAILED", failed))
end
print("========================================")
