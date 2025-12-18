--
-- This file contains most of the code generation logic for the ir instructions
--

-- Helper function
-- Find instruction data by struct name
local function findInstData(insts, struct_name)
	local function search(tbl)
		for _, i in ipairs(tbl) do
			local key, value = next(i)
			local inst_struct_name = value.struct_name or key
			if inst_struct_name == struct_name then
				return value
			end
			-- Recursively search nested instructions
			local result = search(value)
			if result then
				return result
			end
		end
		return nil
	end

	return search(insts)
end

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
	local result = args.noIsaImpl and ""
		or [[static bool isaImpl(IROp op)
    {
        return (kIROpMask_OpMask & op) == kIROp_]]
			.. name
			.. [[;
    }
    enum { kOp = kIROp_]]
			.. name
			.. [[ }; ]]

	-- Add getter methods if operands are specified
	if args.operands then
		for i, operand in ipairs(args.operands) do
			if not operand.variadic then
				local operandName = operand[1]
				local operandType = operand[2]
				local getterName = "get" .. operandName:sub(1, 1):upper() .. operandName:sub(2)
				local returnType = "IRInst"
				if operandType then
					returnType = operandType
					result = result
						.. "\n    "
						.. returnType
						.. "* "
						.. getterName
						.. "() { return ("
						.. returnType
						.. "*)getOperand("
						.. (i - 1)
						.. "); }"
				else
					result = result
						.. "\n    "
						.. returnType
						.. "* "
						.. getterName
						.. "() { return getOperand("
						.. (i - 1)
						.. "); }"
				end
			end
		end
	end

	return result
end

-- The definitions for abstract instruction classes
local baseInst = function(name, args)
	args = args or {}
	local result = args.noIsaImpl and ""
		or [[static bool isaImpl(IROp opIn)
    {
        const int op = (kIROpMask_OpMask & opIn);
        return op >= kIROp_First]]
			.. name
			.. [[ && op <= kIROp_Last]]
			.. name
			.. [[;
    }]]

	-- Add getter methods if operands are specified
	if args.operands then
		for i, operand in ipairs(args.operands) do
			if not operand.variadic then
				local operandName = operand[1]
				local operandType = operand[2]
				local getterName = "get" .. operandName:sub(1, 1):upper() .. operandName:sub(2)
				local returnType = "IRInst"
				if operandType then
					returnType = operandType
					result = result
						.. "\n    "
						.. returnType
						.. "* "
						.. getterName
						.. "() { return ("
						.. returnType
						.. "*)getOperand("
						.. (i - 1)
						.. "); }"
				else
					result = result
						.. "\n    "
						.. returnType
						.. "* "
						.. getterName
						.. "() { return getOperand("
						.. (i - 1)
						.. "); }"
				end
			end
		end
	end

	return result
end

-- Prepare instruction data for template iteration
local function getAllOtherInstStructsData()
	local insts = require("source/slang/slang-ir-insts.lua").insts
	local result = {}

	walk_instructions(insts, function(key, value, struct_name, parent_struct)
		-- If this type already has a definition, skip it
		if not Slang["IR" .. struct_name] then
			local inst_data = {
				struct_name = struct_name,
				parent_struct = parent_struct,
				is_leaf = value.is_leaf,
				operands = {},
			}

			-- Prepare operand data
			if value.operands then
				-- Handle different operand formats for getAllOtherInstStructsData
				if type(value.operands[1]) == "table" then
					-- Format: { { "name", "type" }, { "name2", "type2" } }
					for i, operand in ipairs(value.operands) do
						local operandName = operand[1]
						local operandType = operand[2]
						table.insert(inst_data.operands, {
							name = operandName,
							type = operandType,
							getter_name = "get" .. operandName:sub(1, 1):upper() .. operandName:sub(2),
							index = i - 1,
							has_type = operandType ~= nil,
						})
					end
				elseif type(value.operands[1]) == "string" then
					-- Format: { "name", "type", variadic = true } - single operand
					local operandName = value.operands[1]
					local operandType = value.operands[2]
					table.insert(inst_data.operands, {
						name = operandName,
						type = operandType,
						getter_name = "get" .. operandName:sub(1, 1):upper() .. operandName:sub(2),
						index = 0,
						has_type = operandType ~= nil,
						variadic = value.operands.variadic,
					})
				end
			end

			table.insert(result, inst_data)
		end
	end)

	return result
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
			-- Calculate operand count from operands array or use min_operands as fallback
			local operand_count = 0
			if value.operands then
				operand_count = #value.operands
			elseif value.min_operands then
				operand_count = value.min_operands
			end

			RAW(
				"{kIROp_"
					.. struct_name
					.. ', {"'
					.. value.mnemonic
					.. '", '
					.. tostring(operand_count)
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

-- Collect type instructions for template generation, do it here rather than
-- clutter the FIDDLE TEMPLATE code
local function getBasicTypesForBuilderMethods()
	local lua_data = require("source/slang/slang-ir-insts.lua")
	local type_insts = lua_data.type_insts

	-- Define the list of type instruction exceptions that should NOT have FIDDLE auto-generated methods
	-- These types have complex logic or identity semantics that require manual implementation
	local excluded_types = {
		-- special cases
		"BindExistentialsType", -- Complex logic that simplifies interface types immediately
		"BoundInterfaceType", -- Conditional logic that skips wrapping for __Dynamic types
		"BackwardDiffIntermediateContextType", -- Has null->void conversion logic
		"AttributedType", -- Uses dynamic operand list
		"RefParamType", -- Has high-level helper that takes AddressSpace parameters
		"BorrowInParamType", -- Has high-level helper that takes AddressSpace parameters
		"FuncType", -- the result and params have always been given in the "wrong" order, and the IRAttr oddness at the end
		-- Identity semantics - use createXxxType() instead of getXxxType()
		"StructType",
		"ClassType",
	}

	local excluded_set = {}
	for _, type_name in ipairs(excluded_types) do
		excluded_set[type_name] = true
	end

	local result = {}

	if type_insts then
		walk_instructions(type_insts, function(key, value, struct_name, parent_struct)
			-- Only process leaf instructions (not base classes) that are not excluded
			if struct_name and value.is_leaf and not excluded_set[struct_name] then
				-- Determine method name - always add "get" prefix
				local method_name = "get" .. struct_name

				-- Infer return type - specific type pointer for the struct
				local return_type = "IR" .. struct_name .. "*"

				-- Infer opcode
				local opcode = "kIROp_" .. struct_name

				-- Get operands info from the instruction data
				local operands = {}
				local is_variadic = false
				local has_optional = false
				local optional_started = false
				if value and value.operands then
					-- Handle different operand formats
					if type(value.operands[1]) == "table" then
						-- Format: { { "name", "type" }, { "name2", "type2" } }
						for i, operand in ipairs(value.operands) do
							local operand_info = {
								name = operand[1],
								type = operand[2] or "IRInst",
								index = i - 1,
							}
							-- Check if this operand is variadic
							if operand.variadic then
								operand_info.variadic = true
								is_variadic = true
								-- Variadic and optional operands together are not supported for now
								if has_optional then
									error(
										"Type "
											.. struct_name
											.. " has both variadic and optional operands, which is not supported by the automatic IRBuilder method Fiddle generator"
									)
								end
							end
							-- Check if this operand is optional
							if operand.optional then
								operand_info.optional = true
								has_optional = true
								optional_started = true
							else
								-- Non-optional operand found after optional operands started
								if optional_started then
									error(
										"Type "
											.. struct_name
											.. " has non-optional operand '"
											.. operand[1]
											.. "' after optional operands. Optional operands must be at the end."
									)
								end
							end
							table.insert(operands, operand_info)
						end
					else
						-- Format: { "name", "type", variadic = true }
						local operand_info = {
							name = value.operands[1],
							type = value.operands[2] or "IRInst",
							index = 0,
						}
						-- Check if this operand is variadic
						if value.operands.variadic then
							operand_info.variadic = true
							is_variadic = true
						end
						-- Check if this operand is optional
						if value.operands.optional then
							operand_info.optional = true
							has_optional = true
						end
						table.insert(operands, operand_info)
					end
				end

				table.insert(result, {
					struct_name = struct_name,
					method_name = method_name,
					return_type = return_type,
					opcode = opcode,
					operands = operands,
					is_variadic = is_variadic,
					has_optional = has_optional,
				})
			end
		end)
	end

	return result
end

return {
	leafInst = function(args)
		-- Get the current instruction definition from the Lua data
		local insts = require("source/slang/slang-ir-insts.lua").insts
		local current_name = tostring(fiddle.current_decl):gsub("^IR", "")
		local inst_data = findInstData(insts, current_name)

		-- Merge the args with the instruction data
		local merged_args = args or {}
		if inst_data and inst_data.operands then
			merged_args.operands = inst_data.operands
		end

		return leafInst(current_name, merged_args)
	end,
	baseInst = function(args)
		-- Get the current instruction definition from the Lua data
		local insts = require("source/slang/slang-ir-insts.lua").insts
		local current_name = tostring(fiddle.current_decl):gsub("^IR", "")
		local inst_data = findInstData(insts, current_name)

		-- Merge the args with the instruction data
		local merged_args = args or {}
		if inst_data and inst_data.operands then
			merged_args.operands = inst_data.operands
		end

		return baseInst(current_name, merged_args)
	end,
	getAllOtherInstStructsData = getAllOtherInstStructsData,
	instStructForwardDecls = instStructForwardDecls,
	instInfoEntries = instInfoEntries,
	instEnums = instEnums,
	getBasicTypesForBuilderMethods = getBasicTypesForBuilderMethods,
}
