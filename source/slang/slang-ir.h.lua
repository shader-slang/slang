--
-- This file contains most of the code generation logic for the ir instructions
--

-- Helper function
-- Walk the instruction tree and call a callback for each instruction
local function walk_instructions(insts, callback, parent_struct)
	local function walk_insts(tbl, parent)
		for _, i in ipairs(tbl) do
			local key, value = next(i)
			-- Determine struct name
			local struct_name = value.struct_name and value.struct_name or key

			-- Call the callback
			callback(key, value, struct_name, parent)

			-- Recursively process nested instructions
			walk_insts(value, struct_name)
		end
	end

	-- Start walking from the top-level insts
	if insts then
		walk_insts(insts, "Inst")
	end
end

-- The definitions for leaf instructions
local leafInst = function(name, args)
	args = args or {}
	return args.noIsaImpl and ""
			or [[static bool isaImpl(IROp op)
    {
        return (kIROpMask_OpMask & op) == kIROp_]]
			.. name
			.. [[;
    }
    enum { kOp = kIROp_]]
			.. name
			.. [[ }; ]]
end

-- The definitions for abstract instruction classes
local baseInst = function(name, args)
	args = args or {}
	return args.noIsaImpl and ""
			or [[static bool isaImpl(IROp opIn)
    {
        const int op = (kIROpMask_OpMask & opIn);
        return op >= kIROp_First]]
			.. name
			.. [[ && op <= kIROp_Last]]
			.. name
			.. [[;
    }]]
end

-- Generate struct definitions for instructions not defined by the user
local function allOtherInstStructs()
	local insts = require("source/slang/slang-ir-insts.lua").insts
	local output = {}

	walk_instructions(insts, function(key, value, struct_name, parent_struct)
		-- If this type already has a definition, skip it
		if not Slang["IR" .. struct_name] then
			-- Generate struct definition
			table.insert(output, string.format("struct IR%s : IR%s", struct_name, parent_struct))
			table.insert(output, "{")
			table.insert(
				output,
				string.format("    %s", value.is_leaf and leafInst(struct_name) or baseInst(struct_name))
			)
			table.insert(output, "};")
			table.insert(output, "")
		end
	end)

	return table.concat(output, "\n")
end

-- Forward declarations for everything
local function instStructForwardDecls()
	local insts = require("source/slang/slang-ir-insts.lua").insts
	local output = {}

	walk_instructions(insts, function(key, value, struct_name, parent_struct)
		-- Generate struct definition
		table.insert(output, string.format("struct IR%s;", struct_name))
	end)

	return table.concat(output, "\n")
end

-- The info table
local function instInfoEntries()
	local insts = require("source/slang/slang-ir-insts.lua").insts
	local output = {}

	local function constructFlags(value)
		local flags = {}

		-- Map of field names to their IROpFlags names
		local flagMap = {
			hoistable = "kIROpFlag_Hoistable",
			parent = "kIROpFlag_Parent",
			global = "kIROpFlag_Global",
			use_other = "kIROpFlag_UseOther",
		}

		-- Check each flag and add to the list if true
		for field, flagName in pairs(flagMap) do
			if value[field] then
				table.insert(flags, flagName)
			end
		end

		-- Join all flags with " | "
		if #flags > 0 then
			return table.concat(flags, " | ")
		else
			return "kIROpFlags_None" -- or return "" if you prefer empty string
		end
	end

	walk_instructions(insts, function(key, value, struct_name, parent_struct)
		if value.is_leaf then
			RAW(
				"{kIROp_"
				.. struct_name
				.. ', {"'
				.. value.mnemonic
				.. '", '
				.. tostring(value.min_operands)
				.. ", "
				.. constructFlags(value)
				.. "}},"
			)
		end
	end)
end

-- The enum table
local function instEnums()
	local insts = require("source/slang/slang-ir-insts.lua").insts
	local output = {}

	-- Walk manually so we can output in postorder
	local function traverse(tbl)
		local first_child = nil
		local last_child = nil

		-- First, process all children
		for _, i in ipairs(tbl) do
			local key, value = next(i)
			if value.is_leaf then
				-- Leaf instruction
				RAW("    kIROp_" .. value.struct_name .. ",")

				-- Track first and last child
				if first_child == nil then
					first_child = "kIROp_" .. value.struct_name
				end
				last_child = "kIROp_" .. value.struct_name
			else
				-- Parent instruction - recurse first
				local child_first, child_last = traverse(value)

				-- Track first and last child across all children
				if first_child == nil then
					first_child = child_first
				end
				if child_last then
					last_child = child_last
				end

				-- Then add parent entries
				if child_first and child_last then
					RAW("    kIROp_First" .. value.struct_name .. " = " .. child_first .. ",")
					RAW("    kIROp_Last" .. value.struct_name .. " = " .. child_last .. ",")
				end
			end
			RAW("\n")
		end

		return first_child, last_child
	end

	traverse(insts)
	return table.concat(output, "\n")
end

return {
	leafInst = function(args)
		return leafInst(tostring(fiddle.current_decl):gsub("^IR", ""), args)
	end,
	baseInst = function(args)
		return baseInst(tostring(fiddle.current_decl):gsub("^IR", ""), args)
	end,
	allOtherInstStructs = allOtherInstStructs,
	instStructForwardDecls = instStructForwardDecls,
	instInfoEntries = instInfoEntries,
	instEnums = instEnums,
}
