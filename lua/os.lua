-- Astra OS policy layer example. `astra` is provided by your small Lua C binding.
-- Bind only the documented functions in include/astra.h.
assert(astra.abi_version() == 1, "unsupported Astra kernel ABI")

local function log(message)
  astra.write("[lua] " .. message .. "\n")
end

local services = {}
function spawn(name, fn)
  assert(type(fn) == "function")
  services[name] = coroutine.create(fn)
end

function run_once()
  for name, service in pairs(services) do
    local ok, err = coroutine.resume(service)
    if not ok then log(name .. " failed: " .. tostring(err)); services[name] = nil end
  end
end

spawn("init", function()
  log("Astra Lua OS layer online; ticks=" .. astra.ticks())
  log(astra.fs_read("/etc/motd"))
end)
