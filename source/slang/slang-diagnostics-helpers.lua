-- Helper functions for defining diagnostics

local diagnostics = {}

-- Helper function to create a span
local function span(location, message)
	return {
		location = location,
		message = message,
		is_note = false,
	}
end

-- Helper function to create a note
local function note(location, message)
	return {
		location = location,
		message = message,
		is_note = true,
	}
end

-- Common function to add a diagnostic
local function add_diagnostic(name, code, severity, message, primary_span, ...)
	local diag = {
		name = name,
		code = code,
		severity = severity,
		message = message,
		primary_span = primary_span,
	}

	local extra_spans = {...}
	if #extra_spans > 0 then
		local secondary_spans = {}
		local notes = {}

		for _, s in ipairs(extra_spans) do
			if s.is_note then
				table.insert(notes, {
					location = s.location,
					message = s.message,
				})
			else
				table.insert(secondary_spans, {
					location = s.location,
					message = s.message,
				})
			end
		end

		if #secondary_spans > 0 then
			diag.secondary_spans = secondary_spans
		end
		if #notes > 0 then
			diag.notes = notes
		end
	end

	table.insert(diagnostics, diag)
end

-- Helper function to add an error diagnostic
local function err(name, code, message, primary_span, ...)
	add_diagnostic(name, code, "error", message, primary_span, ...)
end

-- Helper function to add a warning diagnostic
local function warning(name, code, message, primary_span, ...)
	add_diagnostic(name, code, "warning", message, primary_span, ...)
end

-- Helper function to parse interpolated message strings
-- Converts "text ~param more text" or "text ~param:Type more text" into structured format
-- Parameters are automatically deduplicated
local function parse_message(message)
	local parts = {}
	local pos = 1

	while pos <= #message do
		local tilde_pos = message:find("~", pos, true)

		if not tilde_pos then
			if pos <= #message then
				table.insert(parts, {
					type = "text",
					content = message:sub(pos),
				})
			end
			break
		end

		if tilde_pos > pos then
			table.insert(parts, {
				type = "text",
				content = message:sub(pos, tilde_pos - 1),
			})
		end

		-- Extract parameter name and optional type: ~name or ~name:Type
		local param_start = tilde_pos + 1
		local param_end = param_start - 1

		while param_end + 1 <= #message do
			local char = message:sub(param_end + 1, param_end + 1)
			if char:match("[%w_:]") then
				param_end = param_end + 1
			else
				break
			end
		end

		if param_end < param_start then
			error("Empty parameter name after ~ at position " .. tilde_pos)
		end

		local param_spec = message:sub(param_start, param_end)
		local param_name, param_type = param_spec:match("^([%w_]+):?([%w_]*)$")

		if not param_name then
			error("Invalid parameter syntax: ~" .. param_spec)
		end

		-- Default to string if no type specified
		if param_type == "" then
			param_type = "string"
		else
			param_type = param_type:lower()
		end

		table.insert(parts, {
			type = "interpolation",
			param_name = param_name,
			param_type = param_type,
		})

		pos = param_end + 1
	end

	return parts
end

-- Helper function to parse location specification with optional type
-- Supports "location_name" or "location_name:Type"
local function parse_location_spec(location_spec)
	local name, loc_type = location_spec:match("^([%w_]+):?([%w_]*)$")
	if not name then
		error("Invalid location syntax: " .. location_spec)
	end

	-- Default to SourceLoc if no type specified
	if loc_type == "" then
		loc_type = nil  -- Will be treated as plain SourceLoc
	else
		loc_type = loc_type:lower()
	end

	return name, loc_type
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

			local function add_location(loc_name, loc_type)
				if loc_name and not seen_locations[loc_name] then
					table.insert(locations, { name = loc_name, type = loc_type })
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

				-- Extract params for the global diagnostic param list (automatically deduplicated)
				for _, part in ipairs(parts) do
					if part.type == "interpolation" and not seen_params[part.param_name] then
						table.insert(params, { name = part.param_name, type = part.param_type })
						seen_params[part.param_name] = true
					end
				end

				-- Add location if it exists (for spans/notes)
				if container.location then
					local loc_name, loc_type = parse_location_spec(container.location)
					add_location(loc_name, loc_type)
					-- Store parsed location info back to container for code generation
					container.location_name = loc_name
					container.location_type = loc_type
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

return {
	diagnostics = diagnostics,
	span = span,
	note = note,
	err = err,
	warning = warning,
	process_diagnostics = process_diagnostics,
}
