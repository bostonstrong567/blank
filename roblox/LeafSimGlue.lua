--[[
    LeafSim — Glue connection manager + proximity events
    ====================================================
    Glue is the single entry point for every connection/signal/event this
    script makes. What it gives you:

      * Keyed registry — every connection gets a key. Binding the same key
        twice is an O(1) hash lookup + one boolean check, then it skips.
        No duplicate connections, no wasted work.
      * Safe toggles — enable/disable a connection without disconnecting it
        (one boolean check per fire while disabled, nothing else runs).
      * Trigger-driven control — pass signals as `enableOn` / `disableOn` /
        `toggleOn` / `disconnectOn` and Glue wires them up (and tears them
        down) for you.
      * Error containment — handlers run inside pcall; one bad handler
        never kills the connection loop or the rest of the script.
      * `once` semantics that disconnect BEFORE the handler runs, so
        re-entrant fires can't double-trigger.

    `isWithin` is now dual-mode:
      * Old check mode still works:  local ok, dist = isWithin(a, b, radius)
      * Event mode: pass a handler and it becomes a Glue-managed watcher
        that fires the handler once the distance requirement is met.
]]

local Players = game:GetService("Players")
local RunService = game:GetService("RunService")

-- ------------------------------------------------------------
-- These must already exist, same as the original script:
--   LeafSim      -> the module/table with collectMany
--   LeavesFolder -> the folder holding the leaf instances
-- ------------------------------------------------------------
local upvalues = debug.getupvalues(LeafSim.collectMany)
local leafToId = upvalues[3]

assert(type(leafToId) == "table", "Failed to get leafToId/u30")

-- ============================================================
-- Glue: one manager for every connection/signal/event we make
-- ============================================================
local Glue = {}
do
    local registry = {} -- key -> record. One hash lookup answers "already applied?"

    -- Turn anything signal-ish into a `connect(fn) -> connection` closure.
    -- Supports RBXScriptSignals, BindableEvents, and custom signal tables
    -- with a Connect/connect method.
    local function getConnector(signal)
        local kind = typeof(signal)

        if kind == "RBXScriptSignal" then
            return function(fn)
                return signal:Connect(fn)
            end
        end

        if kind == "Instance" and signal:IsA("BindableEvent") then
            local event = signal.Event
            return function(fn)
                return event:Connect(fn)
            end
        end

        if kind == "table" then
            local connect = signal.Connect or signal.connect
            if type(connect) == "function" then
                return function(fn)
                    return connect(signal, fn)
                end
            end
        end

        return nil
    end

    local function disconnect(connection)
        if connection == nil then
            return
        end

        local kind = typeof(connection)

        if kind == "RBXScriptConnection" then
            connection:Disconnect()
        elseif kind == "table" then
            local method = connection.Disconnect or connection.disconnect
            if type(method) == "function" then
                pcall(method, connection)
            end
        end
    end

    local function isLive(record)
        if not record.alive then
            return false
        end

        local connection = record.connection

        if typeof(connection) == "RBXScriptConnection" then
            return connection.Connected
        end

        if typeof(connection) == "table" and connection.Connected ~= nil then
            return connection.Connected == true
        end

        return true
    end

    --[[
        Glue.bind(key, signal, handler, options?) -> handle

        The glue. Every connection goes through here.

        options:
          enabled      = true   -- start live; false = connected but dormant
          once         = false  -- auto-disconnect right before the first fire
          enableOn     = signal or {signals}  -- fires -> connection enabled
          disableOn    = signal or {signals}  -- fires -> connection disabled
          toggleOn     = signal or {signals}  -- fires -> flip enabled state
          disconnectOn = signal or {signals}  -- fires -> fully unbind

        If `key` is already bound and still live, the call is a fast skip
        and you get the existing handle back (new options are ignored —
        unbind first if you want to re-wire).
    ]]
    function Glue.bind(key, signal, handler, options)
        assert(key ~= nil, "Glue.bind: key is required")
        assert(type(handler) == "function", "Glue.bind: handler must be a function")

        local existing = registry[key]
        if existing then
            if isLive(existing) then
                return existing.handle -- already applied: skip, O(1)
            end
            Glue.unbind(key) -- stale (instance destroyed etc.) — clean out, rebind fresh
        end

        options = options or {}

        local connect = getConnector(signal)
        assert(connect ~= nil, "Glue.bind: unsupported signal for key " .. tostring(key))

        local record = {
            key = key,
            handler = handler,
            enabled = options.enabled ~= false,
            once = options.once == true,
            alive = true,
            connection = nil,
            controlKeys = nil,
            handle = nil,
        }

        local function invoke(...)
            if not record.alive or not record.enabled then
                return -- dormant: one boolean check and out
            end

            if record.once then
                Glue.unbind(key) -- disconnect first so re-entrant fires can't double-run
            end

            local ok, err = pcall(record.handler, ...)
            if not ok then
                warn(("[Glue] handler for '%s' errored: %s"):format(tostring(key), tostring(err)))
            end
        end

        record.connection = connect(invoke)

        local handle = { key = key }
        function handle.Disconnect()
            return Glue.unbind(key)
        end
        handle.disconnect = handle.Disconnect
        function handle.SetEnabled(state)
            return Glue.setEnabled(key, state)
        end
        function handle.Toggle()
            return Glue.toggle(key)
        end
        function handle.IsConnected()
            return Glue.isBound(key)
        end
        record.handle = handle

        registry[key] = record

        -- Wire trigger-driven controls. Each control connection is itself a
        -- Glue binding (dedup + cleanup for free) and dies with its parent.
        local function wireControl(suffix, signals, action)
            if signals == nil then
                return
            end

            -- A single signal (including signal-like tables) gets wrapped;
            -- a plain array of signals is iterated.
            if typeof(signals) ~= "table" or signals.Connect or signals.connect then
                signals = { signals }
            end

            record.controlKeys = record.controlKeys or {}

            for index, controlSignal in ipairs(signals) do
                local controlKey = tostring(key) .. suffix .. "#" .. index
                record.controlKeys[#record.controlKeys + 1] = controlKey
                Glue.bind(controlKey, controlSignal, action)
            end
        end

        wireControl("::enableOn", options.enableOn, function()
            Glue.setEnabled(key, true)
        end)
        wireControl("::disableOn", options.disableOn, function()
            Glue.setEnabled(key, false)
        end)
        wireControl("::toggleOn", options.toggleOn, function()
            Glue.toggle(key)
        end)
        wireControl("::disconnectOn", options.disconnectOn, function()
            Glue.unbind(key)
        end)

        return handle
    end

    -- Sugar: bind that fires once then disconnects itself.
    function Glue.bindOnce(key, signal, handler, options)
        options = options or {}
        options.once = true
        return Glue.bind(key, signal, handler, options)
    end

    -- Fully disconnect and forget a key (plus its control bindings).
    function Glue.unbind(key)
        local record = registry[key]
        if not record then
            return false
        end

        registry[key] = nil
        record.alive = false
        disconnect(record.connection)
        record.connection = nil

        if record.controlKeys then
            for _, controlKey in ipairs(record.controlKeys) do
                Glue.unbind(controlKey)
            end
        end

        return true
    end

    -- Toggle without disconnecting: the connection stays attached, its
    -- handler just short-circuits while disabled.
    function Glue.setEnabled(key, state)
        local record = registry[key]
        if not record then
            return false
        end
        record.enabled = state ~= false
        return true
    end

    function Glue.enable(key)
        return Glue.setEnabled(key, true)
    end

    function Glue.disable(key)
        return Glue.setEnabled(key, false)
    end

    function Glue.toggle(key)
        local record = registry[key]
        if not record then
            return nil
        end
        record.enabled = not record.enabled
        return record.enabled
    end

    function Glue.isBound(key)
        local record = registry[key]
        return record ~= nil and isLive(record)
    end

    function Glue.isEnabled(key)
        local record = registry[key]
        return record ~= nil and record.enabled
    end

    -- Unbind everything, or everything whose string key starts with prefix.
    function Glue.unbindAll(prefix)
        local keys = {}
        for key in pairs(registry) do
            if prefix == nil or (type(key) == "string" and key:sub(1, #prefix) == prefix) then
                keys[#keys + 1] = key
            end
        end

        local removed = 0
        for _, key in ipairs(keys) do
            if Glue.unbind(key) then
                removed = removed + 1
            end
        end

        return removed
    end

    --[[
        Glue.watch(key, predicate, handler, options?) -> handle

        Condition-as-a-signal: runs `predicate` on Heartbeat (optionally
        throttled) and fires `handler` once the requirement is met.
        `predicate` returns (met, detail); `detail` is passed to the handler.

        options (on top of the Glue.bind control options):
          once     = true  -- fire once then auto-unbind (default)
          interval = 0     -- seconds between checks; 0 = every Heartbeat
          onExit   = fn    -- when once=false: fires when the condition stops
                           -- being met (then it can fire again on re-entry)
    ]]
    function Glue.watch(key, predicate, handler, options)
        options = options or {}

        local interval = options.interval or 0
        local once = options.once ~= false
        local onExit = options.onExit
        local accumulated = 0
        local met = false

        return Glue.bind(key, RunService.Heartbeat, function(dt)
            if interval > 0 then
                accumulated = accumulated + (dt or 0)
                if accumulated < interval then
                    return
                end
                accumulated = 0
            end

            local ok, result, detail = pcall(predicate)
            if not ok then
                warn(("[Glue] watch predicate for '%s' errored: %s"):format(tostring(key), tostring(result)))
                Glue.unbind(key)
                return
            end

            if result and not met then
                met = true
                if once then
                    Glue.unbind(key)
                end
                handler(detail)
            elseif not result and met then
                met = false
                if onExit then
                    pcall(onExit, detail)
                end
            end
        end, {
            -- note: `once` is intentionally NOT forwarded — the Heartbeat
            -- connection must keep firing until the condition is met.
            enabled = options.enabled,
            enableOn = options.enableOn,
            disableOn = options.disableOn,
            toggleOn = options.toggleOn,
            disconnectOn = options.disconnectOn,
        })
    end
end

-- ============================================================
-- Leaf helpers (unchanged behaviour)
-- ============================================================

local function getLeafId(leaf)
    return leafToId[leaf]
end

local function getCurrentLeaves()
    local leaves = {}

    for _, leaf in ipairs(LeavesFolder:GetChildren()) do
        local id = leafToId[leaf]

        if id ~= nil then
            leaves[#leaves + 1] = {
                id = id,
                leaf = leaf,
            }
        end
    end

    return leaves
end

local function countCurrentLeaves()
    return #getCurrentLeaves()
end

local function getPosition(object)
    if typeof(object) == "Vector3" then
        return object
    elseif typeof(object) == "CFrame" then
        return object.Position
    elseif typeof(object) == "Instance" then
        if object:IsA("BasePart") then
            return object.Position
        elseif object:IsA("Model") then
            return object:GetPivot().Position
        elseif object:IsA("Attachment") then
            return object.WorldPosition
        end
    end

    return nil
end

-- ============================================================
-- isWithin: check OR connection, your call
-- ============================================================

-- The plain synchronous check (exactly the old behaviour).
local function checkWithin(a, b, radius)
    local posA = getPosition(a)
    local posB = getPosition(b)

    if not posA or not posB then
        return false, math.huge
    end

    local distance = (posA - posB).Magnitude

    return distance <= radius, distance
end

--[[
    Check mode (backwards compatible — same as the old script):

        local within, distance = isWithin(root, leaf, 20)

    Event mode — pass a handler and it becomes a Glue-managed connection
    that fires the handler(distance) once `a` is within `radius` of `b`:

        local handle = isWithin(root, chest, 12, function(distance)
            print("in range!", distance)
        end, {
            once = true,            -- default: fire once, then auto-disconnect
            interval = 0.1,         -- seconds between checks (0 = every Heartbeat)
            onExit = function() end,-- when once=false: fires on leaving range
            key = "nearChest",      -- dedup key (defaults to a|b|radius, so
                                    -- re-registering the same watch skips)
            toggleOn = someSignal,  -- pause/resume when this fires
            disconnectOn = someSignal,
        })

        handle.Disconnect()  -- or handle.Toggle() / handle.SetEnabled(false)
]]
local function isWithin(a, b, radius, handler, options)
    if handler == nil then
        return checkWithin(a, b, radius)
    end

    options = options or {}

    local key = options.key
        or ("isWithin::" .. tostring(a) .. "|" .. tostring(b) .. "|" .. tostring(radius))

    return Glue.watch(key, function()
        return checkWithin(a, b, radius)
    end, handler, options)
end

local function getNearbyLeafIds(radius)
    local character = Players.LocalPlayer.Character
    local root = character
        and character:FindFirstChild("HumanoidRootPart")

    if not root then
        return {}
    end

    local ids = {}

    for _, leaf in ipairs(LeavesFolder:GetChildren()) do
        local id = getLeafId(leaf)

        if id and isWithin(root, leaf, radius) then
            ids[#ids + 1] = id
        end
    end

    return ids
end

-- ============================================================
-- Usage examples (delete or adapt)
-- ============================================================
--[[

-- 1) Dedup: run this block twice, the second bind is an O(1) skip.
Glue.bind("leafAdded", LeavesFolder.ChildAdded, function(leaf)
    print("new leaf:", getLeafId(leaf))
end)

-- 2) A toggleable loop, killed automatically when the character dies:
local UserInputService = game:GetService("UserInputService")

Glue.bind("autoCollect", RunService.Heartbeat, function()
    local ids = getNearbyLeafIds(20)
    if #ids > 0 then
        LeafSim.collectMany(ids)
    end
end, {
    enabled = false, -- starts off
    disconnectOn = Players.LocalPlayer.CharacterRemoving,
})

Glue.bind("autoCollectKeybind", UserInputService.InputBegan, function(input, gameProcessed)
    if not gameProcessed and input.KeyCode == Enum.KeyCode.F then
        print("autoCollect:", Glue.toggle("autoCollect"))
    end
end)

-- 3) isWithin as an event: do something once the requirement is met.
local character = Players.LocalPlayer.Character or Players.LocalPlayer.CharacterAdded:Wait()
local root = character:WaitForChild("HumanoidRootPart")

isWithin(root, LeavesFolder, 25, function(distance)
    print(("reached the leaves (%.1f studs away)"):format(distance))
end, { interval = 0.1 })

-- 4) Cleanup everything this script wired up:
-- Glue.unbindAll()

]]
