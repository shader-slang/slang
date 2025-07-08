-- Helper function to flatten the instruction hierarchy
local function flatten_instructions(insts, prefix, result)
	prefix = prefix or ""
	result = result or {}

	for _, entry in ipairs(insts) do
		for name, data in pairs(entry) do
			local full_name = prefix == "" and name or (prefix .. "." .. name)

			-- Add the current instruction
			table.insert(result, full_name)

			-- If it's a table with numeric indices, it has children
			if type(data) == "table" and #data > 0 then
				flatten_instructions(data, full_name, result)
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
	if result then
		return result.insts
	end

	-- Otherwise expect the file to return the table
	return result or error("Instruction file must return a table or set global 'insts'")
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

	local ok, result = pcall(chunk)
	if not ok then
		error("Failed to execute stable names file: " .. result)
	end

	-- Validate structure
	if type(result) ~= "table" then
		error("Stable names file must return a table")
	end

	for i, entry in ipairs(result) do
		if type(entry) ~= "table" or #entry ~= 2 then
			error(string.format("Invalid entry at index %d: expected {name, id}", i))
		end
		if type(entry[1]) ~= "string" then
			error(string.format("Invalid name at index %d: expected string, got %s", i, type(entry[1])))
		end
		if type(entry[2]) ~= "number" then
			error(string.format("Invalid id at index %d: expected number, got %s", i, type(entry[2])))
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

	file:write("return {\n")
	for _, entry in ipairs(stable_names) do
		-- Escape quotes in name
		local escaped_name = entry[1]:gsub('"', '\\"')
		file:write(string.format('\t{ "%s", %d },\n', escaped_name, entry[2]))
	end
	file:write("}\n")
	file:close()
end

-- Check for unique names
local function check_unique_names(stable_names)
	local seen_names = {}
	local duplicates = {}

	for i, entry in ipairs(stable_names) do
		local name = entry[1]
		if seen_names[name] then
			if not duplicates[name] then
				duplicates[name] = { seen_names[name] }
			end
			table.insert(duplicates[name], i)
		else
			seen_names[name] = i
		end
	end

	return duplicates
end

-- Check for unique IDs
local function check_unique_ids(stable_names)
	local seen_ids = {}
	local duplicates = {}

	for i, entry in ipairs(stable_names) do
		local id = entry[2]
		if seen_ids[id] then
			if not duplicates[id] then
				duplicates[id] = { seen_ids[id] }
			end
			table.insert(duplicates[id], i)
		else
			seen_ids[id] = i
		end
	end

	return duplicates
end

-- Check bijection
local function check_bijection(inst_names, stable_names)
	local stable_name_set = {}
	local stable_id_set = {}

	for _, entry in ipairs(stable_names) do
		stable_name_set[entry[1]] = entry[2]
		stable_id_set[entry[2]] = entry[1]
	end

	local missing_from_stable = {}
	local extra_in_stable = {}

	-- Check for instructions missing from stable names
	for _, name in ipairs(inst_names) do
		if not stable_name_set[name] then
			table.insert(missing_from_stable, name)
		end
	end

	-- Check for stable names not in instructions
	local inst_name_set = {}
	for _, name in ipairs(inst_names) do
		inst_name_set[name] = true
	end

	for _, entry in ipairs(stable_names) do
		if not inst_name_set[entry[1]] then
			table.insert(extra_in_stable, entry[1])
		end
	end

	return missing_from_stable, extra_in_stable
end

-- Get next available ID
local function get_next_id(stable_names)
	local max_id = -1
	for _, entry in ipairs(stable_names) do
		if entry[2] > max_id then
			max_id = entry[2]
		end
	end
	return max_id + 1
end

-- Print usage
local function print_usage()
	print("Usage: lua check_instructions.lua [inst_file] [stable_file] [command]")
	print("Commands:")
	print("  check  - Check bijection and uniqueness (default)")
	print("  update - Add missing instructions to stable names")
	print("  all    - Do both check and update")
end

-- Main program
local function main(args)
	local inst_file = args[1] or "instructions.lua"
	local stable_file = args[2] or "stable_names.lua"
	local command = args[3] or "check"

	-- Validate command
	local valid_commands = { check = true, update = true, all = true }
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

		-- Check unique names
		local duplicate_names = check_unique_names(stable_names)
		if next(duplicate_names) then
			has_errors = true
			print("ERROR: Duplicate names found:")
			for name, indices in pairs(duplicate_names) do
				print(string.format("  - '%s' at indices: %s", name, table.concat(indices, ", ")))
			end
		else
			print("✓ All names are unique")
		end

		-- Check unique IDs
		local duplicate_ids = check_unique_ids(stable_names)
		if next(duplicate_ids) then
			has_errors = true
			print("ERROR: Duplicate IDs found:")
			for id, indices in pairs(duplicate_ids) do
				print(string.format("  - ID %d at indices: %s", id, table.concat(indices, ", ")))
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
			print("\n✓ Perfect bijection exists!")
		end
	end

	if command == "update" or command == "all" then
		print("\n=== Updating stable names ===")

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
				table.insert(stable_names, { name, next_id })
				next_id = next_id + 1
			end

			-- Sort by ID
			table.sort(stable_names, function(a, b)
				return a[2] < b[2]
			end)

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
