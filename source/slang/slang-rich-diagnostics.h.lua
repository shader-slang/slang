-- Lua helper functions for rich diagnostic data processing
-- This file provides helper functions for FIDDLE templates in slang-rich-diagnostics.h

-- Load the diagnostic definitions
local diagnostics_module = dofile("source/slang/slang-diagnostics.lua")

local M = {}

-- Helper function to convert space-separated name to PascalCase
function M.toPascalCase(name)
	local result = ""
	for word in name:gmatch("%S+") do
		result = result .. word:sub(1, 1):upper() .. word:sub(2):lower()
	end
	return result
end

-- Helper function to convert space-separated name to snake_case
function M.toSnakeCase(name)
	return name:gsub("%s+", "_"):lower()
end

-- Helper function to convert Lua type names to C++ types
local cpp_type_map = {
	string = "String",
	type = "Type*",
	int = "int",
	name = "Name*",
}
function M.getCppType(lua_type)
	local mapped = cpp_type_map[lua_type]
	if not mapped then
		local supported = {}
		for key in pairs(cpp_type_map) do
			supported[#supported + 1] = key
		end
		table.sort(supported)
		error(
			"Unknown type '"
				.. lua_type
				.. "' in diagnostic parameter. Supported types: "
				.. table.concat(supported, ", ")
		)
	end
	return mapped
end

-- Helper function to convert severity names to C++ Severity enum values
local severity_map = {
	["ignored"] = "Severity::Disable",
	["note"] = "Severity::Note",
	["warning"] = "Severity::Warning",
	["error"] = "Severity::Error",
	["fatal error"] = "Severity::Fatal",
	["internal error"] = "Severity::Internal",
}

function M.getSeverityEnum(severity_name)
	local mapped = severity_map[severity_name]
	if not mapped then
		local supported = {}
		for key in pairs(severity_map) do
			supported[#supported + 1] = key
		end
		table.sort(supported)
		error(
			"Unknown severity '"
				.. tostring(severity_name)
				.. "'. Supported severities: "
				.. table.concat(supported, ", ")
		)
	end
	return mapped
end

-- Get all processed diagnostic definitions
function M.getDiagnostics()
	return diagnostics_module
end

-- Get unique parameters from a diagnostic's message parts
function M.getUniqueParams(diagnostic)
	local seen_params = {}
	local params = {}

	for _, part in ipairs(diagnostic.message_parts) do
		if part.type == "interpolation" and not seen_params[part.param_name] then
			seen_params[part.param_name] = true
			table.insert(params, {
				name = part.param_name,
				type = part.param_type,
				cpp_type = M.getCppType(part.param_type),
			})
		end
	end

	return params
end

return M
