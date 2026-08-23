local Players = game:GetService("Players")
local Workspace = game:GetService("Workspace")
local RunService = game:GetService("RunService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Player = Players:FindFirstChild("bostoncheats") or Players.LocalPlayer
local LeafSim = require(Player.PlayerScripts:WaitForChild("LeafSim"))
local LeavesFolder = Workspace:WaitForChild("Leaves")

local Remotes = ReplicatedStorage:WaitForChild("Remotes")
local CollectLeaf = Remotes:WaitForChild("CollectLeaf")
local EmptyBackpack = Remotes:WaitForChild("EmptyBackpack")

local leafToId = debug.getupvalues(LeafSim.collectMany)[3]
assert(type(leafToId) == "table", "Failed to get leafToId/u30")

local genv = type(getgenv) == "function" and getgenv() or _G
local Farm = genv.LeafFarm or {}
genv.LeafFarm = Farm

for key, value in pairs({
    autoCollect = false,
    collectRadius = 15,
    collectInterval = 0.25,
    autoEmpty = false,
    emptyRadius = 21,
    emptyInterval = 0.2,
    dumpsterName = "Dumpster",
}) do
    if Farm[key] == nil then
        Farm[key] = value
    end
end

local Glue = Farm.Glue
if not Glue then
    Glue = {}
    Farm.Glue = Glue

    local registry = {}

    local function getConnector(signal)
        local kind = typeof(signal)

        if kind == "RBXScriptSignal" then
            return function(fn) return signal:Connect(fn) end
        elseif kind == "Instance" and signal:IsA("BindableEvent") then
            return function(fn) return signal.Event:Connect(fn) end
        elseif kind == "table" then
            local connect = signal.Connect or signal.connect
            if type(connect) == "function" then
                return function(fn) return connect(signal, fn) end
            end
        end

        return nil
    end

    local function disconnect(connection)
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
        elseif typeof(connection) == "table" and connection.Connected ~= nil then
            return connection.Connected == true
        end

        return true
    end

    function Glue.bind(key, signal, handler, options)
        assert(key ~= nil, "Glue.bind: key is required")
        assert(type(handler) == "function", "Glue.bind: handler must be a function")

        local existing = registry[key]
        if existing then
            if isLive(existing) then
                return existing.handle
            end
            Glue.unbind(key)
        end

        options = options or {}

        local connect = getConnector(signal)
        assert(connect, "Glue.bind: unsupported signal for key " .. tostring(key))

        local record = {
            key = key,
            handler = handler,
            enabled = options.enabled ~= false,
            once = options.once == true,
            alive = true,
        }

        record.connection = connect(function(...)
            if not record.alive or not record.enabled then
                return
            end

            if record.once then
                Glue.unbind(key)
            end

            local ok, err = pcall(record.handler, ...)
            if not ok then
                warn(("[Glue] handler '%s' errored: %s"):format(tostring(key), tostring(err)))
            end
        end)

        local handle = { key = key }
        function handle.Disconnect() return Glue.unbind(key) end
        handle.disconnect = handle.Disconnect
        function handle.SetEnabled(state) return Glue.setEnabled(key, state) end
        function handle.Toggle() return Glue.toggle(key) end
        function handle.IsConnected() return Glue.isBound(key) end
        record.handle = handle

        registry[key] = record

        local function wireControl(suffix, signals, action)
            if signals == nil then
                return
            end

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

        wireControl("::enableOn", options.enableOn, function() Glue.setEnabled(key, true) end)
        wireControl("::disableOn", options.disableOn, function() Glue.setEnabled(key, false) end)
        wireControl("::toggleOn", options.toggleOn, function() Glue.toggle(key) end)
        wireControl("::disconnectOn", options.disconnectOn, function() Glue.unbind(key) end)

        return handle
    end

    function Glue.bindOnce(key, signal, handler, options)
        options = options or {}
        options.once = true
        return Glue.bind(key, signal, handler, options)
    end

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

    function Glue.setEnabled(key, state)
        local record = registry[key]
        if not record then
            return false
        end
        record.enabled = state ~= false
        return true
    end

    function Glue.enable(key) return Glue.setEnabled(key, true) end
    function Glue.disable(key) return Glue.setEnabled(key, false) end

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
                warn(("[Glue] watch '%s' errored: %s"):format(tostring(key), tostring(result)))
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
            enabled = options.enabled,
            enableOn = options.enableOn,
            disableOn = options.disableOn,
            toggleOn = options.toggleOn,
            disconnectOn = options.disconnectOn,
        })
    end
end

local function getPosition(object)
    local kind = typeof(object)

    if kind == "Vector3" then
        return object
    elseif kind == "CFrame" then
        return object.Position
    elseif kind == "Instance" then
        if object:IsA("BasePart") then
            return object.Position
        elseif object:IsA("Attachment") then
            return object.WorldPosition
        elseif object:IsA("Model") then
            return object:GetPivot().Position
        end
    end

    return nil
end

local function checkWithin(a, b, radius)
    local posA, posB = getPosition(a), getPosition(b)

    if not (posA and posB) then
        return false, math.huge
    end

    local distance = (posA - posB).Magnitude
    return distance <= radius, distance
end

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

local function getRoot()
    local character = Players.LocalPlayer.Character
    return character and character:FindFirstChild("HumanoidRootPart")
end

local function getLeafId(leaf)
    return leafToId[leaf]
end

local function getCurrentLeaves()
    local leaves = {}

    for _, leaf in ipairs(LeavesFolder:GetChildren()) do
        local id = leafToId[leaf]
        if id then
            leaves[#leaves + 1] = { id = id, leaf = leaf }
        end
    end

    return leaves
end

local function countCurrentLeaves()
    local count = 0

    for _, leaf in ipairs(LeavesFolder:GetChildren()) do
        if leafToId[leaf] then
            count = count + 1
        end
    end

    return count
end

local function getLeafIdsWithin(radius)
    local root = getRoot()
    if not root then
        return {}
    end

    local rootPos = root.Position
    local ids = {}

    for _, leaf in ipairs(LeavesFolder:GetChildren()) do
        local id = leafToId[leaf]
        if id then
            local pos = getPosition(leaf)
            if pos and (pos - rootPos).Magnitude <= radius then
                ids[#ids + 1] = id
            end
        end
    end

    return ids
end

local function collectWithin(radius)
    local ids = getLeafIdsWithin(radius or Farm.collectRadius)

    for _, id in ipairs(ids) do
        CollectLeaf:FireServer(id)
    end

    return #ids
end

local dumpster
local function getDumpster()
    if not (dumpster and dumpster.Parent and dumpster.Name == Farm.dumpsterName) then
        dumpster = Workspace:FindFirstChild(Farm.dumpsterName, true)
    end
    return dumpster
end

local function isNearDumpster(radius)
    local root = getRoot()
    local target = getDumpster()

    if not (root and target) then
        return false, math.huge
    end

    return checkWithin(root, target, radius or Farm.emptyRadius)
end

local collectClock = 0
Glue.bind("collectLoop", RunService.Heartbeat, function(dt)
    if not Farm.autoCollect then
        return
    end

    collectClock = collectClock + dt
    if collectClock < Farm.collectInterval then
        return
    end
    collectClock = 0

    collectWithin(Farm.collectRadius)
end)

local emptyClock = 0
local wasNearDumpster = false
Glue.bind("emptyLoop", RunService.Heartbeat, function(dt)
    if not Farm.autoEmpty then
        wasNearDumpster = false
        return
    end

    emptyClock = emptyClock + dt
    if emptyClock < Farm.emptyInterval then
        return
    end
    emptyClock = 0

    local near = isNearDumpster(Farm.emptyRadius)
    if near and not wasNearDumpster then
        EmptyBackpack:FireServer()
    end
    wasNearDumpster = near
end)

Farm.getLeafId = getLeafId
Farm.getLeaves = getCurrentLeaves
Farm.countLeaves = countCurrentLeaves
Farm.getLeafIdsWithin = getLeafIdsWithin
Farm.collectWithin = collectWithin
Farm.getDumpster = getDumpster
Farm.isNearDumpster = isNearDumpster
Farm.isWithin = isWithin
Farm.emptyNow = function() return EmptyBackpack:FireServer() end

print("LeafFarm loaded | leaves:", countCurrentLeaves())
