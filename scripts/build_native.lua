-- Script to build native.lua

local arg = {...}
local natives = assert(io.open(assert(arg[1], "need input path"), "rb"))

local objectType = {
	"Any",
	"Hash",
	"Entity",
	"Player",
	"FireId",
	"Ped",
	"Vehicle",
	"Cam",
	"CarGenerator",
	"Group",
	"Train",
	"Pickup",
	"Object",
	"Weapon",
	"Interior",
	"Blip",
	"Texture",
	"TextureDict",
	"CoverPoint",
	"Camera",
	"TaskSequence",
	"ColourIndex",
	"Sphere",
	"ScrHandle"
}

io.write [=[
-- This file is automatically generated by build_native.lua
local ffi = require("ffi")
local ScriptHookV = ffi.load("ScriptHookV")

ffi.cdef[[
]=]

-- Generate object types
for _, v in ipairs(objectType) do
	io.write("typedef struct ", v, " {\n\tuint32_t id;\n} ", v, ";\n\n")
end

io.write [=[
typedef struct Vector3
{
	float x __attribute__((aligned(8)));
	float y __attribute__((aligned(8)));
	float z __attribute__((aligned(8)));
} Vector3;
]]

local union = ffi.new([[
union {
	uint64_t value;

	uint32_t i;
	float f;
	const char *string;
	Vector3 *vector3;

]=]

-- Generate object types
for _, v in ipairs(objectType) do
	io.write("\t", v, " ", v:lower(), ";\n")
end
io.write [=[}]])

]=]

local function parseFunction(namespace, line)
	local ret, name, params, hash = line:match("static (%S+) ([0-9A-Zx_]+)%(([^%)]*)%) {%s*[return]* invoke%b<>%((0x[0-9A-F]+)")
	assert(ret, line)

	local args = {}
	local funcArgs = {}
	local funcArgsSimple = {}
	local retVals = {}
	-- Split args
	for w in params:gmatch("[^,]+") do
		local dataType, argName = w:match("(%w+)%*? ([0-9A-Za-z_]+)")
		assert(dataType, line)
		local isPointer = dataType:sub(-1) == "*"

		local t = {
			type = dataType,
			name = argName,
			pointer = isPointer
		}
		args[#args + 1] = t

		if isPointer or name == "Vector3" or ret == "Vector3" then
			-- TODO
			retVals[#retVals + 1] = t
			io.stderr:write("Skipping ", namespace, ".", name, "\n")
			return
		else
			funcArgs[#funcArgs + 1] = t
			funcArgsSimple[#funcArgsSimple + 1] = argName
		end
	end

	-- Write function signature
	io.write("function ", namespace, ".", name, "(", table.concat(funcArgsSimple, ", "), ")\n")

	-- Function body
	for _, v in ipairs(args) do
	end

	io.write("end\n")
end

-- Read definitions
local currentNamespace = nil
for line in natives:lines() do
	line = line:gsub("\r\n", ""):gsub("\n", ""):gsub("\r", "")

	if line:find("namespace", 1, true) == 1 then
		currentNamespace = line:sub(11)
		io.write(currentNamespace, " = {}\n")
	elseif currentNamespace then
		if line == "}" then
			currentNamespace = nil
			io.write("\n")
		elseif line ~= "{" then
			parseFunction(currentNamespace, line)
		end
	end
end
