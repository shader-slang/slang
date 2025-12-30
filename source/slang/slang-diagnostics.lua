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
	},
}

-- Helper function to parse interpolated message strings
-- Converts "text {param : type} more text" into structured format
local function parse_message(message)
	local parts = {}
	local pos = 1

	while pos <= #message do
		local start_brace = message:find("{", pos, true)

		if not start_brace then
			-- No more interpolations, add remaining text
			if pos <= #message then
				table.insert(parts, {
					type = "text",
					content = message:sub(pos),
				})
			end
			break
		end

		-- Add text before the brace
		if start_brace > pos then
			table.insert(parts, {
				type = "text",
				content = message:sub(pos, start_brace - 1),
			})
		end

		-- Find the closing brace
		local end_brace = message:find("}", start_brace + 1, true)
		if not end_brace then
			error("Unclosed brace in message: " .. message)
		end

		-- Parse the interpolation content
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

-- Helper function to validate diagnostic schema
local function validate_diagnostic(diag, index)
	local errors = {}
	local diagnostic_name = diag.name or ("diagnostic[" .. index .. "]")

	-- Check required fields
	if not diag.name or type(diag.name) ~= "string" then
		table.insert(errors, "diagnostic[" .. index .. "].name must be a string")
	end

	if not diag.code or type(diag.code) ~= "number" then
		table.insert(errors, diagnostic_name .. ".code must be a number")
	end

	if not diag.severity or type(diag.severity) ~= "string" then
		table.insert(errors, diagnostic_name .. ".severity must be a string")
	elseif not (diag.severity == "error" or diag.severity == "warning") then
		table.insert(errors, diagnostic_name .. ".severity must be one of: error, warning")
	end

	if not diag.message or type(diag.message) ~= "string" then
		table.insert(errors, diagnostic_name .. ".message must be a string")
	end

	if not diag.primary_span or type(diag.primary_span) ~= "table" then
		table.insert(errors, diagnostic_name .. ".primary_span must be a table")
	else
		if not diag.primary_span.location or type(diag.primary_span.location) ~= "string" then
			table.insert(errors, diagnostic_name .. ".primary_span.location must be a string")
		end
		if not diag.primary_span.message or type(diag.primary_span.message) ~= "string" then
			table.insert(errors, diagnostic_name .. ".primary_span.message must be a string")
		end
	end

	-- Check optional secondary_spans
	if diag.secondary_spans then
		if type(diag.secondary_spans) ~= "table" then
			table.insert(errors, diagnostic_name .. ".secondary_spans must be a table")
		else
			for i, span in ipairs(diag.secondary_spans) do
				if type(span) ~= "table" then
					table.insert(errors, diagnostic_name .. ".secondary_spans[" .. i .. "] must be a table")
				else
					if not span.location or type(span.location) ~= "string" then
						table.insert(
							errors,
							diagnostic_name .. ".secondary_spans[" .. i .. "].location must be a string"
						)
					end
					if not span.message or type(span.message) ~= "string" then
						table.insert(
							errors,
							diagnostic_name .. ".secondary_spans[" .. i .. "].message must be a string"
						)
					end
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

	-- Track names and codes for uniqueness validation
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
			-- Check for duplicate names
			if diag.name and seen_names[diag.name] then
				table.insert(
					all_errors,
					"duplicate diagnostic name '"
						.. diag.name
						.. "' at index "
						.. i
						.. ", previously seen at index "
						.. seen_names[diag.name]
				)
			else
				seen_names[diag.name] = i
			end

			-- Check for duplicate codes
			if diag.code and seen_codes[diag.code] then
				table.insert(
					all_errors,
					diagnostic_name
						.. " has duplicate code "
						.. diag.code
						.. ", previously used by diagnostic at index "
						.. seen_codes[diag.code]
				)
			else
				seen_codes[diag.code] = i
			end

			-- Parse the message
			local success, message_parts = pcall(parse_message, diag.message)
			if not success then
				table.insert(all_errors, diagnostic_name .. ".message: " .. message_parts)
			else
				local processed_diag = {
					name = diag.name,
					code = diag.code,
					severity = diag.severity,
					message = diag.message,
					message_parts = message_parts,
					primary_span = diag.primary_span,
					secondary_spans = diag.secondary_spans,
				}
				table.insert(processed, processed_diag)
			end
		end
	end

	return processed, all_errors
end

-- Process and validate diagnostics at load time
processed_diagnostics, validation_errors = process_diagnostics(diagnostics)

-- Report any validation errors
if #validation_errors > 0 then
	error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

-- Make processed diagnostics available for code generation
return processed_diagnostics
