local function dump(value, indent, visited)
	indent = indent or 0
	visited = visited or {}

	local spacing = string.rep("  ", indent)
	local output = ""

	local value_type = type(value)

	if value_type == "nil" then
		output = "nil"
	elseif value_type == "boolean" then
		output = tostring(value)
	elseif value_type == "number" then
		output = tostring(value)
	elseif value_type == "string" then
		output = string.format("%q", value)
	elseif value_type == "function" then
		output = tostring(value)
	elseif value_type == "userdata" or value_type == "thread" then
		output = tostring(value)
	elseif value_type == "table" then
		if visited[value] then
			output = "<circular reference>"
		else
			visited[value] = true
			local items = {}
			local is_array = true
			local max_index = 0

			-- Check for metatable
			local mt = getmetatable(value)
			if mt then
				table.insert(items, "<metatable> = " .. dump(mt, indent + 1, visited))
			end

			-- Check if it's an array-like table
			for k, v in pairs(value) do
				if type(k) ~= "number" or k <= 0 or k ~= math.floor(k) then
					is_array = false
					break
				end
				max_index = math.max(max_index, k)
			end

			-- Check for gaps in array
			if is_array then
				for i = 1, max_index do
					if value[i] == nil then
						is_array = false
						break
					end
				end
			end

			if is_array and max_index > 0 then
				-- Format as array
				for i = 1, max_index do
					table.insert(items, dump(value[i], indent + 1, visited))
				end
			else
				-- Format as dictionary
				for k, v in pairs(value) do
					local key_str
					if type(k) == "string" and k:match("^[%a_][%w_]*$") then
						key_str = k
					else
						key_str = "[" .. dump(k, 0, visited) .. "]"
					end
					table.insert(items, key_str .. " = " .. dump(v, indent + 1, visited))
				end
			end

			-- Add tostring representation if it has a custom __tostring
			if mt and mt.__tostring then
				output = tostring(value) .. " "
			end

			if #items > 0 then
				output = output
					.. "{\n"
					.. spacing
					.. "  "
					.. table.concat(items, ",\n" .. spacing .. "  ")
					.. "\n"
					.. spacing
					.. "}"
			else
				output = output .. "{}"
			end
		end
	end

	if indent == 0 then
		io.stderr:write(output .. "\n")
	else
		return output
	end
end

local function isLeaf(struct)
	for _ in pairs(struct.directSubclasses) do
		return false
	end
	return true
end

local function go(struct)
	if isLeaf(struct) then
		RAW("kIROp_" .. tostring(struct) .. ",\n")
		return
	end

	local firstChild = struct.directSubclasses[1]
	local lastChild
	for _, s in pairs(struct.directSubclasses) do
		go(s)
		lastChild = s
	end

	RAW(
		"kIROp_First"
			.. tostring(struct)
			.. " = kIROp_"
			.. (isLeaf(firstChild) and "" or "First")
			.. tostring(firstChild)
			.. ",\n"
	)

	RAW(
		"kIROp_Last"
			.. tostring(struct)
			.. " = kIROp_"
			.. (isLeaf(lastChild) and "" or "Last")
			.. tostring(lastChild)
			.. ",\n"
	)
end

-- Example usage:
-- local inst = find_inst("Void")
-- if inst then
--     print("Struct name: " .. (inst.struct_name or "IR" .. "Void"))
--     print("Min operands: " .. (inst.minOperands or 0))
--     if inst.hoistable then print("Is hoistable") end
-- end
local function find_inst(name)
	local data = require("source/slang/slang-ir-insts")

	-- Cache the flat table of instructions
	if not data._flat_insts then
		data._flat_insts = {}

		-- Recursive function to flatten the instruction tree
		local function flatten_insts(tbl, inherited_flags)
			inherited_flags = inherited_flags or {}

			-- Collect flags from current level
			local current_flags = {}
			for k, v in pairs(inherited_flags) do
				current_flags[k] = v
			end

			-- Add flags from this table if it has any
			if tbl.hoistable then
				current_flags.hoistable = true
			end
			if tbl.parent then
				current_flags.parent = true
			end
			if tbl.useOther then
				current_flags.useOther = true
			end
			if tbl.global then
				current_flags.global = true
			end

			for key, value in pairs(tbl) do
				-- Skip non-instruction fields
				if
					key ~= "struct_name"
					and key ~= "minOperands"
					and key ~= "hoistable"
					and key ~= "parent"
					and key ~= "useOther"
					and key ~= "global"
				then
					if type(value) == "table" then
						-- Create entry for this instruction
						local inst_data = {}

						-- Copy instruction's own data
						for k, v in pairs(value) do
							if type(v) ~= "table" or k == "struct_name" or k == "minOperands" then
								inst_data[k] = v
							end
						end

						-- Add inherited flags
						for k, v in pairs(current_flags) do
							if inst_data[k] == nil then
								inst_data[k] = v
							end
						end

						-- Store in flat table
						data._flat_insts[key] = inst_data

						-- Recursively process nested instructions
						flatten_insts(value, current_flags)
					end
				end
			end
		end

		-- Build the flat table
		if data.insts then
			flatten_insts(data.insts)
		end
	end

	-- Simply look up in the flat table
	return data._flat_insts[name]
end

-- Check if an instruction is a leaf (has no child instructions)
local function is_leaf(inst)
	for k, v in pairs(inst) do
		if
			type(v) == "table"
			and k ~= "struct_name"
			and k ~= "minOperands"
			and k ~= "hoistable"
			and k ~= "parent"
			and k ~= "useOther"
			and k ~= "global"
		then
			return false
		end
	end
	return true
end

-- Walk the instruction tree and call a callback for each instruction
local function walk_instructions(insts, callback, parent_struct)
	local function walk_insts(tbl, parent)
		for key, value in pairs(tbl) do
			-- Skip non-instruction fields
			if
				type(value) == "table"
				and key ~= "struct_name"
				and key ~= "minOperands"
				and key ~= "hoistable"
				and key ~= "parent"
				and key ~= "useOther"
				and key ~= "global"
			then
				-- Determine struct name
				local struct_name = value.struct_name and value.struct_name or key

				-- Call the callback
				callback(key, value, struct_name, parent)

				-- Recursively process nested instructions
				walk_insts(value, struct_name)
			end
		end
	end

	-- Start walking from the top-level insts
	if insts then
		walk_insts(insts, "Inst")
	end
end

local leafInst = function(decl)
	local name = tostring(decl):gsub("^IR", "")
	return decl.isaImpl and ""
		or [[
    static bool isaImpl(IROp op)
    {
        return (kIROpMask_OpMask & op) == kIROp_]]
			.. name
			.. [[;
    }
    ]]
end

local baseInst = function(decl, args)
	args = args or {}
	local name = tostring(decl):gsub("^IR", "")
	return args.noIsaImpl
			and [[
    static bool isaImpl(IROp opIn)
    {
        const int op = (kIROpMask_OpMask & opIn);
        return op >= kIROp_First]] .. name .. [[ && op <= kIROp_Last]] .. name .. [[;}]]
		or ""
end

-- Generate struct definitions for instructions not in Slang table
local function allOtherInstStructs()
	local insts = require("source/slang/slang-ir-insts.lua").insts
	local output = {}

	walk_instructions(insts, function(key, value, struct_name, parent_struct)
		-- If this type already has a definition, skip it
		if not Slang[struct_name] then
			-- Generate struct definition
			table.insert(output, string.format("struct IR%s : IR%s", struct_name, parent_struct))
			table.insert(output, "{")
			table.insert(
				output,
				string.format("    %s", is_leaf(value) and leafInst(struct_name) or baseInst(struct_name))
			)
			table.insert(output, "};")
			table.insert(output, "")
		end
	end)

	-- Remove trailing empty line if any
	if #output > 0 and output[#output] == "" then
		table.remove(output)
	end

	return table.concat(output, "\n")
end

return {
	leafInst = function()
		return leafInst(fiddle.current_decl)
	end,
	baseInst = function(args)
		return baseInst(fiddle.current_decl)
	end,
	allOtherInstStructs = allOtherInstStructs,
}
