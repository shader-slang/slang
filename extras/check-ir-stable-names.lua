-- Helper function to flatten the instruction hierarchy
local function flatten_instructions(insts, prefix, result)
	prefix = prefix or ""
	result = result or {}

	for _, entry in ipairs(insts) do
		for name, data in pairs(entry) do
			local full_name = prefix == "" and name or (prefix .. "." .. name)

			-- If it's a table with numeric indices, it has children
			if type(data) == "table" and #data > 0 then
				flatten_instructions(data, full_name, result)
			else
				-- Add the current instruction
				table.insert(result, full_name)
			end
		end
	end

	return result
end

-- Load instruction definitions
local function load_instructions(filename)
	local chunk, err = loadfile(filename)
	if not chunk then
		error("Failed to load instruction file: " .. filename .. " - " .. (err or "unknown error"))
	end

	-- Just execute it normally
	local result = chunk()

	-- If the file sets a global 'insts', use that
	if result.insts then
		return result.insts
	end

	error("Instruction file must return a table with 'insts' entry")
end

-- Load stable names table
local function load_stable_names(filename)
	local file = io.open(filename, "r")
	if not file then
		-- File doesn't exist, return empty table
		return {}
	end
	file:close()

	local chunk, err = loadfile(filename)
	if not chunk then
		error("Failed to load stable names file: " .. filename .. " - " .. (err or "unknown error"))
	end

	local result = chunk()

	-- Validate structure
	if type(result) ~= "table" then
		error("Stable names file must return a table")
	end

	for name, id in pairs(result) do
		if type(name) ~= "string" then
			error(string.format("Invalid key: expected string, got %s", type(name)))
		end
		if type(id) ~= "number" then
			error(string.format("Invalid value for '%s': expected number, got %s", name, type(id)))
		end
	end

	return result
end

-- Save stable names table
local function save_stable_names(filename, stable_names)
	local file, err = io.open(filename, "w")
	if not file then
		error("Failed to open file for writing: " .. filename .. " - " .. (err or "unknown error"))
	end

	file:write("-- This file is machine generated! any entries written below will be preserved,\n")
	file:write("-- but things like comments or anything outside the schema won't be preserved\n")
	file:write("return {\n")

	-- Sort by ID for consistent output
	local sorted_entries = {}
	for name, id in pairs(stable_names) do
		table.insert(sorted_entries, { name = name, id = id })
	end
	table.sort(sorted_entries, function(a, b)
		return a.id < b.id
	end)

	for _, entry in ipairs(sorted_entries) do
		-- Escape quotes in name
		local escaped_name = entry.name:gsub('"', '\\"')
		file:write(string.format('\t["%s"] = %d,\n', escaped_name, entry.id))
	end
	file:write("}\n")
	file:close()
end

-- Check for unique IDs
local function check_unique_ids(stable_names)
	local seen_ids = {}
	local duplicates = {}

	for name, id in pairs(stable_names) do
		if seen_ids[id] then
			if not duplicates[id] then
				duplicates[id] = { seen_ids[id] }
			end
			table.insert(duplicates[id], name)
		else
			seen_ids[id] = name
		end
	end

	return duplicates
end

-- Check bijection
local function check_bijection(inst_names, stable_names)
	local missing_from_stable = {}
	local extra_in_stable = {}

	-- Check for instructions missing from stable names
	for _, name in ipairs(inst_names) do
		if stable_names[name] == nil then
			table.insert(missing_from_stable, name)
		end
	end

	-- Check for stable names not in instructions
	local inst_name_set = {}
	for _, name in ipairs(inst_names) do
		inst_name_set[name] = true
	end

	for name, _ in pairs(stable_names) do
		if not inst_name_set[name] then
			table.insert(extra_in_stable, name)
		end
	end

	return missing_from_stable, extra_in_stable
end

-- Get next available ID
local function get_next_id(stable_names)
	local max_id = -1
	for _, id in pairs(stable_names) do
		if id > max_id then
			max_id = id
		end
	end
	return max_id + 1
end

-- Print usage
local function print_usage()
	print("Usage: lua check_instructions.lua check|update [inst_file] [stable_file]")
	print("Commands:")
	print("  check  - Check bijection and uniqueness (default)")
	print("  update - Add missing instructions to stable names")
end

-- Main program
local function main(args)
	local command = args[1] or "check"
	local inst_file = args[2] or "source/slang/slang-ir-insts.lua"
	local stable_file = args[3] or "source/slang/slang-ir-insts-stable-names.lua"

	-- Validate command
	local valid_commands = { check = true, update = true }
	if not valid_commands[command] then
		print("ERROR: Invalid command: " .. command)
		print_usage()
		return 1
	end

	-- Load data with error handling
	local ok, insts_or_err = pcall(load_instructions, inst_file)
	if not ok then
		print("ERROR: " .. insts_or_err)
		return 1
	end
	local insts = insts_or_err

	ok, stable_names = pcall(load_stable_names, stable_file)
	if not ok then
		print("ERROR: " .. stable_names)
		return 1
	end

	-- Flatten instruction hierarchy
	local inst_names = flatten_instructions(insts)

	local has_errors = false

	if command == "check" or command == "all" then
		print("=== Checking stable names ===")

		-- Check unique IDs
		local duplicate_ids = check_unique_ids(stable_names)
		if next(duplicate_ids) then
			has_errors = true
			print("ERROR: Duplicate IDs found:")
			for id, names in pairs(duplicate_ids) do
				print(string.format("  - ID %d used by: %s", id, table.concat(names, ", ")))
			end
		else
			print("✓ All IDs are unique")
		end

		-- Check bijection
		local missing, extra = check_bijection(inst_names, stable_names)

		if #missing > 0 then
			has_errors = true
			print(string.format("ERROR: %d instructions missing from stable names:", #missing))
			for _, name in ipairs(missing) do
				print("  - " .. name)
			end
		else
			print("✓ All instructions have stable names")
		end

		if #extra > 0 then
			print(string.format("WARNING: %d extra entries in stable names (not in instructions):", #extra))
			for _, name in ipairs(extra) do
				print("  - " .. name)
			end
		else
			print("✓ No extra entries in stable names")
		end

		if not has_errors and #extra == 0 then
			print("✓ Is a bijection")
		end
	end

	if command == "update" or command == "all" then
		print("=== Updating stable names ===")

		-- Don't update if there are errors
		if has_errors then
			print("ERROR: Cannot update due to errors in existing stable names")
			return 1
		end

		local missing, _ = check_bijection(inst_names, stable_names)

		if #missing > 0 then
			-- Add missing instructions
			local next_id = get_next_id(stable_names)

			for _, name in ipairs(missing) do
				stable_names[name] = next_id
				next_id = next_id + 1
			end

			-- Save updated file
			local ok, err = pcall(save_stable_names, stable_file, stable_names)
			if not ok then
				print("ERROR: Failed to save: " .. err)
				return 1
			end

			print(string.format("Added %d new instructions to %s", #missing, stable_file))
		else
			print("No missing instructions to add")
		end
	end

	return has_errors and 1 or 0
end

-- Run the program
os.exit(main(arg) or 0)
