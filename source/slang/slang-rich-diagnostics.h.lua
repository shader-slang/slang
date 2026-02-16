-- Lua helper functions for rich diagnostic data processing
-- This file provides helper functions for FIDDLE templates in slang-rich-diagnostics.h

-- Load shared helpers for edit distance functions
local helpers = dofile("source/slang/slang-diagnostics-helpers.lua")
local edit_distance = helpers.edit_distance
local find_similar = helpers.find_similar

-- Load the diagnostic definitions (this processes using helpers internally)
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
	qualtype = "QualType",
	int = "int",
	name = "Name*",
	decl = "Decl*",
	expr = "Expr*",
	stmt = "Stmt*",
	val = "Val*",
}
function M.getCppType(lua_type)
	local mapped = cpp_type_map[lua_type]
	if not mapped then
		local supported = {}
		for key in pairs(cpp_type_map) do
			supported[#supported + 1] = key
		end
		table.sort(supported)

		-- Find similar types using edit distance
		local similar = find_similar(lua_type, supported, 3)

		local msg = "Unknown type '" .. lua_type .. "' in diagnostic parameter."

		if #similar > 0 then
			msg = msg .. "\n  Did you mean: "
			local suggestions = {}
			for i, s in ipairs(similar) do
				if i <= 3 then  -- Limit to top 3 suggestions
					table.insert(suggestions, "'" .. s.name .. "'")
				end
			end
			msg = msg .. table.concat(suggestions, ", ") .. "?"
		end

		msg = msg .. "\n  Supported types: " .. table.concat(supported, ", ")

		error(msg)
	end
	return mapped
end

-- Helper function to get the location extraction expression for a typed location
function M.getLocationExpr(location_name, location_type)
	if not location_type then
		-- Plain SourceLoc
		return location_name
	end

	-- Map types to their location extraction methods
	local location_extractors = {
		decl = location_name .. "->getNameLoc()",
		expr = location_name .. "->loc",
		stmt = location_name .. "->loc",
		type = location_name .. "->loc",
		val = location_name .. "->loc",
		name = location_name .. "->loc",
	}

	local extractor = location_extractors[location_type]
	if not extractor then
		-- Build a list of valid types for suggestion
		local valid_types = {}
		for k in pairs(location_extractors) do
			table.insert(valid_types, k)
		end
		-- Add SourceLoc-like names for suggestion (use plain location name without type)
		local sourceloc_variants = { "SourceLoc", "sourceloc", "Sourceloc", "sourceLoc" }

		-- Find similar types
		local similar = find_similar(location_type, valid_types, 3)

		-- Check if user might have meant SourceLoc
		local sourceloc_dist = edit_distance(location_type:lower(), "sourceloc")
		local might_mean_sourceloc = sourceloc_dist <= 3

		-- Build error message with suggestions
		local msg = "Unknown location type '" .. location_type .. "' for location '" .. location_name .. "'."

		if might_mean_sourceloc then
			msg = msg .. "\n  Hint: For a plain SourceLoc, omit the type annotation: use '" .. location_name .. "' instead of '" .. location_name .. ":" .. location_type .. "'."
		end

		if #similar > 0 then
			msg = msg .. "\n  Did you mean: "
			local suggestions = {}
			for i, s in ipairs(similar) do
				if i <= 3 then  -- Limit to top 3 suggestions
					table.insert(suggestions, "'" .. s.name .. "'")
				end
			end
			msg = msg .. table.concat(suggestions, ", ") .. "?"
		end

		table.sort(valid_types)
		msg = msg .. "\n  Valid location types: " .. table.concat(valid_types, ", ")

		error(msg)
	end

	return extractor
end

-- Member access mapping: type + member -> {cpp_expr, result_type}
-- cpp_expr can be a function that takes the base expression and returns the C++ code
local member_access_map = {
	expr = {
		type = { expr = function(base) return base .. "->type" end, type = "type" },
		loc = { expr = function(base) return base .. "->loc" end, type = "sourceloc" },
	},
	decl = {
		name = { expr = function(base) return base .. "->getName()" end, type = "name" },
		loc = { expr = function(base) return base .. "->loc" end, type = "sourceloc" },
	},
	type = {
		loc = { expr = function(base) return base .. "->loc" end, type = "sourceloc" },
	},
	stmt = {
		loc = { expr = function(base) return base .. "->loc" end, type = "sourceloc" },
	},
	val = {
		loc = { expr = function(base) return base .. "->loc" end, type = "sourceloc" },
	},
	name = {
		text = { expr = function(base) return base .. "->text" end, type = "string" },
		loc = { expr = function(base) return base .. "->loc" end, type = "sourceloc" },
	},
}

-- Helper function to resolve member access (single level only, no chaining)
-- Returns {cpp_expr, result_type} or raises an error
function M.resolveMemberAccess(base_type, member_name, base_expr)
	local type_members = member_access_map[base_type]
	if not type_members then
		error("Type '" .. base_type .. "' has no known members (accessing ." .. member_name .. ")")
	end

	local member_info = type_members[member_name]
	if not member_info then
		local available = {}
		for m in pairs(type_members) do
			table.insert(available, m)
		end
		table.sort(available)
		error("Type '" .. base_type .. "' has no member '" .. member_name .. "'. Available members: " ..
		      table.concat(available, ", "))
	end

	return member_info.expr(base_expr), member_info.type
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

return M
