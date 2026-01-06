-- !!!!
-- THIS FILE IS PROTOTYPE WORK, PLEASE USE source/slang/slang-diagnostic-defs.h
-- !!!!
--
-- Lua-based diagnostic definitions for the Slang compiler
-- This file defines diagnostics which will be processed
-- by slang-fiddle to generate C++ diagnostic structures and builders.
--
-- Schema definition:
-- {
--     name = "diagnostic name",        -- Space-separated name (becomes PascalCase in C++, snake-case for flags)
--     code = 12345,                    -- Integer diagnostic code
--     severity = "error",              -- "error", "warning"
--     message = "text with {param : type} interpolation",
--     primary_span = {
--         location = "location_name",  -- Name of location parameter
--         message = "span message",
--     },
--     secondary_spans = {              -- Optional additional spans
--         {
--             location = "location_name",
--             message = "span message",
--         },
--     },
-- }

-- Examples of inline type interpolation:
-- "invalid conversion from {from : type} to {to : type}"
-- "wrong number of arguments, expected {expected : int} got {got : int}"

local diagnostics = {
	{
		name = "function return type mismatch",
		code = 30007,
		severity = "error",
		message = "expression type '{expression_type : type}' does not match function's return type '{return_type : type}'",
		primary_span = {
			location = "expression_location",
			message = "expression type",
		},
		secondary_spans = {
			{
				location = "function_location",
				message = "function return type",
			},
		},
	},
	{
		name = "function redefinition",
		code = 30201,
		severity = "error",
		message = "function '{name : name}' already has a body",
		primary_span = {
			location = "function_location",
			message = "redeclared here",
		},
		notes = {
			{
				location = "original_location",
				message = "see previous definition of '{name : name}'",
			},
		},
	},
}

-- Helper function to parse interpolated message strings
-- Converts "text {param : type} more text" into structured format
-- Helper function to parse interpolated message strings
-- Converts "text {param : type} more text" into structured format
local function parse_message(message)
	local parts = {}
	local pos = 1

	while pos <= #message do
		local start_brace = message:find("{", pos, true)

		if not start_brace then
			if pos <= #message then
				table.insert(parts, {
					type = "text",
					content = message:sub(pos),
				})
			end
			break
		end

		if start_brace > pos then
			table.insert(parts, {
				type = "text",
				content = message:sub(pos, start_brace - 1),
			})
		end

		local end_brace = message:find("}", start_brace + 1, true)
		if not end_brace then
			error("Unclosed brace in message: " .. message)
		end

		local interp_content = message:sub(start_brace + 1, end_brace - 1)
		local param_name, param_type = interp_content:match("^%s*([%w_]+)%s*:%s*([%w_]+)%s*$")

		if not param_name or not param_type then
			error("Invalid interpolation format: {" .. interp_content .. "}")
		end

		table.insert(parts, {
			type = "interpolation",
			param_name = param_name,
			param_type = param_type,
		})

		pos = end_brace + 1
	end

	return parts
end

-- Helper function to validate a span-like structure (location and message)
local function validate_span(span, path)
	local errors = {}
	if type(span) ~= "table" then
		table.insert(errors, path .. " must be a table")
	else
		if not span.location or type(span.location) ~= "string" then
			table.insert(errors, path .. ".location must be a string")
		end
		if not span.message or type(span.message) ~= "string" then
			table.insert(errors, path .. ".message must be a string")
		end
	end
	return errors
end

-- Helper function to validate diagnostic schema
local function validate_diagnostic(diag, index)
	local errors = {}
	local diagnostic_name = diag.name or ("diagnostic[" .. index .. "]")

	-- 1. Validate mandatory 'name' field
	if not diag.name or type(diag.name) ~= "string" then
		table.insert(errors, "diagnostic[" .. index .. "].name must be a string")
	end

	-- 2. Validate mandatory 'code' field
	if not diag.code or type(diag.code) ~= "number" then
		table.insert(errors, diagnostic_name .. ".code must be a number")
	end

	-- 3. Validate mandatory 'severity' field and allowed values
	if not diag.severity or type(diag.severity) ~= "string" then
		table.insert(errors, diagnostic_name .. ".severity must be a string")
	elseif not (diag.severity == "error" or diag.severity == "warning") then
		table.insert(errors, diagnostic_name .. ".severity must be one of: error, warning")
	end

	-- 4. Validate mandatory 'message' field
	if not diag.message or type(diag.message) ~= "string" then
		table.insert(errors, diagnostic_name .. ".message must be a string")
	end

	-- 5. Validate mandatory 'primary_span' structure
	local primary_errors = validate_span(diag.primary_span, diagnostic_name .. ".primary_span")
	for _, err in ipairs(primary_errors) do
		table.insert(errors, err)
	end

	-- 6. Validate optional 'secondary_spans' array
	if diag.secondary_spans then
		if type(diag.secondary_spans) ~= "table" then
			table.insert(errors, diagnostic_name .. ".secondary_spans must be a table")
		else
			for i, span in ipairs(diag.secondary_spans) do
				local span_errors = validate_span(span, diagnostic_name .. ".secondary_spans[" .. i .. "]")
				for _, err in ipairs(span_errors) do
					table.insert(errors, err)
				end
			end
		end
	end

	-- 7. Validate optional 'notes' array
	if diag.notes then
		if type(diag.notes) ~= "table" then
			table.insert(errors, diagnostic_name .. ".notes must be a table")
		else
			for i, note in ipairs(diag.notes) do
				local note_errors = validate_span(note, diagnostic_name .. ".notes[" .. i .. "]")
				for _, err in ipairs(note_errors) do
					table.insert(errors, err)
				end
			end
		end
	end

	return errors
end

-- Function to process and validate all diagnostics
local function process_diagnostics(diagnostics_table)
	local processed = {}
	local all_errors = {}

	if type(diagnostics_table) ~= "table" then
		table.insert(all_errors, "diagnostics must be a table")
		return nil, all_errors
	end

	local seen_names = {}
	local seen_codes = {}

	for i, diag in ipairs(diagnostics_table) do
		local diagnostic_name = diag.name or ("diagnostic[" .. i .. "]")
		local errors = validate_diagnostic(diag, i)

		if #errors > 0 then
			for _, err in ipairs(errors) do
				table.insert(all_errors, err)
			end
		else
			if seen_names[diag.name] then
				table.insert(all_errors, "duplicate diagnostic name '" .. diag.name .. "' at index " .. i)
			else
				seen_names[diag.name] = i
			end

			if seen_codes[diag.code] then
				table.insert(all_errors, diagnostic_name .. " has duplicate code " .. diag.code)
			else
				seen_codes[diag.code] = i
			end

			local params = {}
			local locations = {}
			local seen_params = {}
			local seen_locations = {}

			local function add_location(loc_name)
				if loc_name and not seen_locations[loc_name] then
					table.insert(locations, { name = loc_name })
					seen_locations[loc_name] = true
				end
			end

			-- Uniformly processes a span/message object: parses interpolants and adds locations
			local function process_message_container(container, context_path)
				local success, parts = pcall(parse_message, container.message)
				if not success then
					table.insert(all_errors, context_path .. ": " .. parts)
					return nil
				end

				-- Extract params for the global diagnostic param list
				for _, part in ipairs(parts) do
					if part.type == "interpolation" and not seen_params[part.param_name] then
						table.insert(params, { name = part.param_name, type = part.param_type })
						seen_params[part.param_name] = true
					end
				end

				-- Add location if it exists (for spans/notes)
				if container.location then
					add_location(container.location)
				end

				-- Attach the parsed parts back to the object for C++ generation
				container.message_parts = parts
				return parts
			end

			-- 1. Process main message (treated as a container without a location)
			local main_msg_container = { message = diag.message }
			local main_parts = process_message_container(main_msg_container, diagnostic_name .. ".message")

			-- 2. Process primary span
			process_message_container(diag.primary_span, diagnostic_name .. ".primary_span")

			-- 3. Process secondary spans
			if diag.secondary_spans then
				for j, span in ipairs(diag.secondary_spans) do
					process_message_container(span, diagnostic_name .. ".secondary_spans[" .. j .. "]")
				end
			end

			-- 4. Process notes
			if diag.notes then
				for j, note in ipairs(diag.notes) do
					process_message_container(note, diagnostic_name .. ".notes[" .. j .. "]")
				end
			end

			table.insert(processed, {
				name = diag.name,
				code = diag.code,
				severity = diag.severity,
				message = diag.message,
				message_parts = main_parts,
				params = params,
				locations = locations,
				primary_span = diag.primary_span,
				secondary_spans = diag.secondary_spans or {},
				notes = diag.notes or {},
			})
		end
	end

	return processed, all_errors
end

processed_diagnostics, validation_errors = process_diagnostics(diagnostics)

if #validation_errors > 0 then
	error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

return processed_diagnostics
