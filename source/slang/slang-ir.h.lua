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

-- Collect basic type instructions for template generation
local function getBasicTypesForBuilderMethods()
	local insts = require("source/slang/slang-ir-insts.lua").insts

	-- Define the list of basic types we want to generate getters for
	-- Each entry can be either a string (type name) or a table {type_name, method_name}
	local basic_types = {
		"VoidType",
		"BoolType",
		"Int8Type",
		"Int16Type",
		"IntType",
		"Int64Type",
		"UInt8Type",
		"UInt16Type",
		"UIntType",
		"UInt64Type",
		"HalfType",
		"FloatType",
		"DoubleType",
		"CharType",
		"IntPtrType",
		"UIntPtrType",
		"StringType",
		"NativeStringType",
		-- Batch 1 additions - simple types with no operands
		"CapabilitySetType",
		"RawPointerType",
		"RTTIType",
		"RTTIHandleType",
		"DynamicType",
		-- Batch 2 additions - types that use simple getType pattern
		"OptionalType",
		"BasicBlockType",
		"TypeKind",
		"GenericKind",
		-- Batch 3 additions - more simple no-operand types
		"MetalMeshGridPropertiesType",
		-- Batch 4 additions - rate types
		"ConstExprRate",
		"GroupSharedRate", 
		"ActualGlobalRate",
		"SpecConstRate",
		-- Batch 5 additions - layout types and other simple types
		"TorchTensorType",
		"ConjunctionType",
		"FuncType",
		"DefaultBufferLayoutType",
		"Std140BufferLayoutType",
		-- Batch 6 additions - more layout and texture shape types
		"Std430BufferLayoutType",
		"ScalarBufferLayoutType", 
		"CBufferLayoutType",
		"TextureShape2DType",
		"TextureShape3DType",
		-- Batch 7 additions - types with single operands
		"NativePtrType",
		"RTTIPointerType", 
		"AnyValueType",
		"ComPtrType",
		"ArrayListType",
		"TensorViewType",
		-- Batch 8 additions - single operand types that currently use arrays  
		-- Note: GLSLOutputParameterGroupType has operand definition mismatch - reverted
		"PseudoPtrType",
		-- Batch 9 additions - multi-parameter types with proper operand definitions
		"ResultType",
		"RateQualifiedType",
		"MetalMeshType",
		-- Batch 10 additions - types with matching operand definitions  
		-- Note: AttributedType uses dynamic operand list - can't convert
		-- Note: BackwardDiffIntermediateContextType has null->void conversion logic - can't convert
		-- Note: RefParamType and BorrowInParamType use getPtrType() instead of getType() - can't convert
		"ThisType",
		-- Batch 11 additions - variadic types
		"TupleType",
		-- Batch 12 additions - more variadic and single operand types
		"TypePack",
		"TargetTupleType", 
		"AssociatedType",
		"WitnessTableType",
		"WitnessTableIDType",
		-- Note: TypeType already handled by getTypeType() inline
		-- These types already have operand definitions and can be auto-generated
		"ArrayType",
		"UnsizedArrayType", 
		"VectorType",
		"MatrixType",
		"CoopVectorType",
	}

	local result = {}

	for _, entry in ipairs(basic_types) do
		-- Handle both string entries and table entries {type_name, method_name}
		local type_name, custom_method_name
		if type(entry) == "string" then
			type_name = entry
			custom_method_name = nil
		else
			type_name = entry[1]
			custom_method_name = entry[2]
		end

		-- Find the instruction data in the Lua definitions
		local inst_data = nil
		walk_instructions(insts, function(key, value, struct_name, parent_struct)
			if struct_name == type_name then
				inst_data = value
			end
		end)

		-- Determine method name - always add "get" prefix
		local method_name
		if custom_method_name then
			method_name = "get" .. custom_method_name
		else
			method_name = "get" .. type_name
		end

		-- Infer return type - specific type pointer for the struct
		local return_type = "IR" .. type_name .. "*"

		-- Infer opcode
		local opcode = "kIROp_" .. type_name

		-- Get operands info
		local operands = {}
		local is_variadic = false
		local has_optional = false
		local optional_started = false
		if inst_data and inst_data.operands then
			-- Handle different operand formats
			if type(inst_data.operands[1]) == "table" then
				-- Format: { { "name", "type" }, { "name2", "type2" } }
				for i, operand in ipairs(inst_data.operands) do
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
							error("Type " .. type_name .. " has both variadic and optional operands, which is not supported")
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
							error("Type " .. type_name .. " has non-optional operand '" .. operand[1] .. "' after optional operands. Optional operands must be at the end.")
						end
					end
					table.insert(operands, operand_info)
				end
			else
				-- Format: { "name", "type", variadic = true }
				local operand_info = {
					name = inst_data.operands[1],
					type = inst_data.operands[2] or "IRInst",
					index = 0,
				}
				-- Check if this operand is variadic
				if inst_data.operands.variadic then
					operand_info.variadic = true
					is_variadic = true
				end
				-- Check if this operand is optional
				if inst_data.operands.optional then
					operand_info.optional = true
					has_optional = true
				end
				table.insert(operands, operand_info)
			end
		end

		table.insert(result, {
			struct_name = type_name,
			method_name = method_name,
			return_type = return_type,
			opcode = opcode,
			operands = operands,
			is_variadic = is_variadic,
			has_optional = has_optional,
		})
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
