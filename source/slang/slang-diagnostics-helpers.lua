-- Helper functions for defining diagnostics

local diagnostics = {}

-- Helper function to create a span
-- Accepts positional: span(location, message?)
-- Or named: span({loc = location, message = message})
local function span(location, message)
  -- Normalize named arguments to positional
  -- Detect named argument table by checking it's not an already-built span (which has is_note field)
  if type(location) == "table" and location.is_note == nil then
    -- Named argument format
    local args = location
    location = args.loc
    message = args.message
    if not location then
      error("span() requires 'loc' field in named argument format")
    end
  end

  return {
    location = location,
    message = message or "",  -- Default to empty string if not provided
    is_note = false,
    variadic = false,
  }
end

-- Helper function to create a note with message and spans
-- Accepts positional: note(message, span1, span2, ...)
-- Or named: note({message = message, span1, span2, ...})
local function note(message, ...)
  local all_spans = {...}

  -- Normalize named arguments to positional
  if type(message) == "table" and not message.is_note then
    -- Named argument format
    local args = message
    message = args.message
    if not message then
      error("note() requires 'message' field in named argument format")
    end
    -- Extract spans from the table (all non-string-key entries)
    all_spans = {}
    for i, v in ipairs(args) do
      table.insert(all_spans, v)
    end
  end

  if type(message) ~= "string" then
    error("note() first argument must be a message (string)")
  end

  if #all_spans == 0 then
    error("note() requires at least one span after the message")
  end

  -- Validate that none of the spans are notes (prevent nesting)
  for i, s in ipairs(all_spans) do
    if type(s) ~= "table" then
      error("note() argument " .. (i + 1) .. " must be a span (got " .. type(s) .. ")")
    end
    if s.is_note then
      error("note() cannot contain nested notes (argument " .. (i + 1) .. " is a note)")
    end
  end

  local primary_span = all_spans[1]
  local additional_spans = {}
  for i = 2, #all_spans do
    table.insert(additional_spans, all_spans[i])
  end

  return {
    location = primary_span.location,
    message = message,
    primary_span_message = primary_span.message,  -- Store primary span's message
    is_note = true,
    variadic = false,
    spans = #additional_spans > 0 and additional_spans or nil,
  }
end

-- Helper function to create a variadic span (generates a nested struct and List<>)
-- Accepts positional: variadic_span(struct_name, location, message)
-- Or named: variadic_span({cpp_name = struct_name, loc = location, message = message})
local function variadic_span(struct_name, location, message)
  -- Normalize named arguments to positional
  -- Detect named argument table by checking it's not an already-built span (which has is_note field)
  if type(struct_name) == "table" and struct_name.is_note == nil then
    -- Named argument format
    local args = struct_name
    struct_name = args.cpp_name
    location = args.loc
    message = args.message
    if not struct_name then
      error("variadic_span() requires 'cpp_name' field in named argument format")
    end
    if not location then
      error("variadic_span() requires 'loc' field in named argument format")
    end
    if not message then
      error("variadic_span() requires 'message' field in named argument format")
    end
  end

  return {
    location = location,
    message = message,
    is_note = false,
    variadic = true,
    struct_name = struct_name,
  }
end

-- Helper function to create a variadic note (generates a nested struct and List<>)
-- Accepts positional: variadic_note(struct_name, message, span1, span2, ...)
-- Or named: variadic_note({cpp_name = struct_name, message = message, span1, span2, ...})
local function variadic_note(struct_name, message, ...)
  local all_spans = {...}

  -- Normalize named arguments to positional
  if type(struct_name) == "table" and not struct_name.is_note then
    -- Named argument format
    local args = struct_name
    struct_name = args.cpp_name or args.struct_name
    message = args.message
    if not struct_name then
      error("variadic_note() requires 'cpp_name' or 'struct_name' field in named argument format")
    end
    if not message then
      error("variadic_note() requires 'message' field in named argument format")
    end
    -- Extract spans from the table (all non-string-key entries)
    all_spans = {}
    for i, v in ipairs(args) do
      table.insert(all_spans, v)
    end
  end

  if type(struct_name) ~= "string" then
    error("variadic_note() first argument must be a struct name (string)")
  end

  if type(message) ~= "string" then
    error("variadic_note() second argument must be a message (string)")
  end

  if #all_spans == 0 then
    error("variadic_note() requires at least one span after the message")
  end

  -- Validate that none of the spans are notes (prevent nesting)
  for i, s in ipairs(all_spans) do
    if type(s) ~= "table" then
      error("variadic_note() argument " .. (i + 2) .. " must be a span (got " .. type(s) .. ")")
    end
    if s.is_note then
      error("variadic_note() cannot contain nested notes (argument " .. (i + 2) .. " is a note)")
    end
  end

  local primary_span = all_spans[1]
  local additional_spans = {}
  for i = 2, #all_spans do
    table.insert(additional_spans, all_spans[i])
  end

  return {
    location = primary_span.location,
    message = message,
    primary_span_message = primary_span.message,  -- Store primary span's message
    is_note = true,
    variadic = true,
    struct_name = struct_name,
    spans = #additional_spans > 0 and additional_spans or nil,
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

  local extra_spans = { ... }
  if #extra_spans > 0 then
    local secondary_spans = {}
    local notes = {}

    for _, s in ipairs(extra_spans) do
      if s.is_note then
        table.insert(notes, {
          location = s.location,
          message = s.message,
          primary_span_message = s.primary_span_message,
          variadic = s.variadic,
          struct_name = s.struct_name,
          spans = s.spans,
        })
      else
        table.insert(secondary_spans, {
          location = s.location,
          message = s.message,
          variadic = s.variadic,
          struct_name = s.struct_name,
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
-- Also supports member access: ~param.member
-- Parameters are automatically deduplicated
local function parse_message(message, known_params)
  local parts = {}
  local pos = 1
  known_params = known_params or {}

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

    -- Extract parameter name and optional type or member access
    -- Formats: ~name, ~name:Type, ~name.member, ~name:Type.member
    local param_start = tilde_pos + 1
    local param_end = param_start - 1

    while param_end + 1 <= #message do
      local char = message:sub(param_end + 1, param_end + 1)
      if char:match("[%w_:.]+") then
        param_end = param_end + 1
      else
        break
      end
    end

    if param_end < param_start then
      error("Empty parameter name after ~ at position " .. tilde_pos)
    end

    local param_spec = message:sub(param_start, param_end)

    -- Try to match: ~name:Type.member or ~name.member
    local base_name, explicit_type, member_name = param_spec:match("^([%w_]+):([%w_]+)%.([%w_]+)$")
    if not base_name then
      -- Try ~name.member (type from known_params)
      base_name, member_name = param_spec:match("^([%w_]+)%.([%w_]+)$")
    end

    if base_name and member_name then
      -- Member access: determine base type
      local base_type = explicit_type and explicit_type:lower() or known_params[base_name]
      if not base_type then
        error(
          "Unknown parameter '"
            .. base_name
            .. "' in member access ~"
            .. param_spec
            .. ". You must declare the parameter with a type first (e.g., ~"
            .. base_name
            .. ":Type) or use inline type (e.g., ~"
            .. base_name
            .. ":Type.member)"
        )
      end

      table.insert(parts, {
        type = "interpolation",
        param_name = base_name,
        param_type = base_type,
        member_name = member_name,
      })
    else
      -- Regular parameter with optional type: ~name or ~name:Type
      local param_name, param_type = param_spec:match("^([%w_]+):?([%w_]*)$")

      if not param_name then
        error("Invalid parameter syntax: ~" .. param_spec)
      end

      -- Default to string if no type specified and not previously known
      if param_type == "" then
        param_type = known_params[param_name] or "string"
      else
        param_type = param_type:lower()
      end

      table.insert(parts, {
        type = "interpolation",
        param_name = param_name,
        param_type = param_type,
      })
    end

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
    loc_type = nil -- Will be treated as plain SourceLoc
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
        -- Validate spans within notes
        if note.spans then
          if type(note.spans) ~= "table" then
            table.insert(errors, diagnostic_name .. ".notes[" .. i .. "].spans must be a table")
          else
            for j, span in ipairs(note.spans) do
              local span_errors = validate_span(span, diagnostic_name .. ".notes[" .. i .. "].spans[" .. j .. "]")
              for _, err in ipairs(span_errors) do
                table.insert(errors, err)
              end
            end
          end
        end
      end
    end
  end

  return errors
end

-- Helper to make a plural name for list members
local function pluralize(name)
  -- Simple English pluralization
  if name:sub(-1) == "s" or name:sub(-2) == "ch" or name:sub(-2) == "sh" then
    return name .. "es"
  elseif name:sub(-1) == "y" and not name:sub(-2, -2):match("[aeiou]") then
    return name:sub(1, -2) .. "ies"
  else
    return name .. "s"
  end
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

      -- Track which params/locations appear in which contexts
      -- param_contexts[name] = { variadic_structs = {struct_name1, ...}, non_variadic = bool }
      local param_contexts = {}
      local seen_params = {} -- name -> type mapping
      local seen_locations = {} -- name -> {type, variadic, struct_name}
      local param_order = {} -- Array tracking order of first appearance

      -- Variadic structs: struct_name -> { params = {}, locations = {}, container_type = "span"|"note" }
      local variadic_structs = {}
      local variadic_structs_order = {} -- Array tracking order of variadic struct registration

      -- First pass: collect all locations to build the known_params map
      local function collect_location(container)
        if container.location then
          local loc_name, loc_type = parse_location_spec(container.location)
          -- Store parsed location info back to container for code generation
          container.location_name = loc_name
          container.location_type = loc_type

          -- Track this location
          if not seen_locations[loc_name] then
            seen_locations[loc_name] = { type = loc_type }
          end

          -- Track location as a parameter for type resolution
          if loc_type and not seen_params[loc_name] then
            seen_params[loc_name] = loc_type
          end

          -- Track context (variadic vs non-variadic)
          if not param_contexts[loc_name] then
            param_contexts[loc_name] = { variadic_structs = {}, non_variadic = false, is_location = true }
            table.insert(param_order, loc_name) -- Track order of first appearance
          end
          if container.variadic and container.struct_name then
            param_contexts[loc_name].variadic_structs[container.struct_name] = true
          else
            param_contexts[loc_name].non_variadic = true
          end
        end
      end

      -- Collect locations from primary span
      collect_location(diag.primary_span)

      -- Collect locations from secondary spans
      if diag.secondary_spans then
        for _, span in ipairs(diag.secondary_spans) do
          collect_location(span)
          -- Register variadic struct
          if span.variadic and span.struct_name then
            if not variadic_structs[span.struct_name] then
              variadic_structs[span.struct_name] = { params = {}, locations = {}, container_type = "span", container = span }
              table.insert(variadic_structs_order, span.struct_name) -- Track order
            end
          end
        end
      end

      -- Collect locations from notes
      if diag.notes then
        for _, note in ipairs(diag.notes) do
          collect_location(note)
          -- Register variadic struct
          if note.variadic and note.struct_name then
            if not variadic_structs[note.struct_name] then
              variadic_structs[note.struct_name] = { params = {}, locations = {}, container_type = "note", container = note }
              table.insert(variadic_structs_order, note.struct_name) -- Track order
            end
          end
          -- Collect locations from spans within notes
          if note.spans then
            for _, span in ipairs(note.spans) do
              collect_location(span)
            end
          end
        end
      end

      -- Helper to extract all required parameters from a container (for null checking)
      -- Only includes pointer types (not strings or ints) since those can be null-checked
      local function extract_required_params(container)
        local required = {}
        local required_set = {}

        -- Add location parameter if it's typed (locations are always pointers or SourceLoc)
        if container.location_name and container.location_type then
          table.insert(required, container.location_name)
          required_set[container.location_name] = true
        end

        -- Add all pointer-type parameters from message (excluding member accesses and non-pointer types)
        if container.message_parts then
          for _, part in ipairs(container.message_parts) do
            if part.type == "interpolation" and not part.member_name then
              local param_type = seen_params[part.param_name]
              -- Only add pointer types (type, decl, expr, stmt, val, name - not string or int)
              if param_type and param_type ~= "string" and param_type ~= "int" then
                if not required_set[part.param_name] then
                  table.insert(required, part.param_name)
                  required_set[part.param_name] = true
                end
              end
            end
          end
        end

        return required
      end

      -- Second pass: process messages now that we know all the location parameters
      local function process_message_container(container, context_path)
        local success, parts = pcall(parse_message, container.message, seen_params)
        if not success then
          table.insert(all_errors, context_path .. ": " .. parts)
          return nil
        end

        -- Extract params and track their contexts
        for _, part in ipairs(parts) do
          if part.type == "interpolation" then
            local param_name = part.param_name
            local param_type = part.param_type

            -- Register or validate param type
            if not seen_params[param_name] then
              seen_params[param_name] = param_type
            elseif seen_params[param_name] ~= param_type then
              table.insert(
                all_errors,
                context_path
                  .. ": parameter '"
                  .. param_name
                  .. "' is used with conflicting types: '"
                  .. seen_params[param_name]
                  .. "' and '"
                  .. param_type
                  .. "'"
              )
            end

            -- Track context (variadic vs non-variadic) - skip member accesses as they derive from the base param
            if not part.member_name then
              if not param_contexts[param_name] then
                param_contexts[param_name] = { variadic_structs = {}, non_variadic = false, is_location = false }
                table.insert(param_order, param_name) -- Track order of first appearance
              end
              if container.variadic and container.struct_name then
                param_contexts[param_name].variadic_structs[container.struct_name] = true
              else
                param_contexts[param_name].non_variadic = true
              end
            end
          end
        end

        -- Attach the parsed parts back to the object for C++ generation
        container.message_parts = parts
        return parts
      end

      -- 1. Process main message (treated as a container without a location)
      local main_msg_container = { message = diag.message }
      local main_parts = process_message_container(main_msg_container, diagnostic_name .. ".message")

      -- Extract required params from main message for assertions (only pointer types)
      main_msg_container.message_parts = main_parts
      local main_required = {}
      if main_parts then
        for _, part in ipairs(main_parts) do
          if part.type == "interpolation" and not part.member_name then
            local param_type = seen_params[part.param_name]
            -- Only add pointer types (not string or int)
            if param_type and param_type ~= "string" and param_type ~= "int" then
              table.insert(main_required, part.param_name)
            end
          end
        end
      end

      -- 2. Process primary span
      process_message_container(diag.primary_span, diagnostic_name .. ".primary_span")

      -- Extract required params from primary span for assertions (only pointer types)
      local primary_span_required = {}
      if diag.primary_span.location_name and diag.primary_span.location_type then
        table.insert(primary_span_required, diag.primary_span.location_name)
      end
      if diag.primary_span.message_parts then
        for _, part in ipairs(diag.primary_span.message_parts) do
          if part.type == "interpolation" and not part.member_name then
            local param_type = seen_params[part.param_name]
            -- Only add pointer types (not string or int)
            if param_type and param_type ~= "string" and param_type ~= "int" then
              -- Avoid duplicates
              local found = false
              for _, existing in ipairs(primary_span_required) do
                if existing == part.param_name then
                  found = true
                  break
                end
              end
              if not found then
                table.insert(primary_span_required, part.param_name)
              end
            end
          end
        end
      end

      -- 3. Process secondary spans
      if diag.secondary_spans then
        for j, span in ipairs(diag.secondary_spans) do
          process_message_container(span, diagnostic_name .. ".secondary_spans[" .. j .. "]")
          -- Extract required params for null checking (skip variadic spans, they're handled per-item)
          if not span.variadic then
            span.required_params = extract_required_params(span)
          end
        end
      end

      -- 4. Process notes
      if diag.notes then
        for j, note in ipairs(diag.notes) do
          process_message_container(note, diagnostic_name .. ".notes[" .. j .. "]")
          -- Process primary span message
          if note.primary_span_message then
            local primary_span_container = { message = note.primary_span_message }
            local primary_span_parts = process_message_container(primary_span_container, diagnostic_name .. ".notes[" .. j .. "].primary_span")
            note.primary_span_message_parts = primary_span_parts
          end
          -- Process spans within notes
          if note.spans then
            for k, span in ipairs(note.spans) do
              process_message_container(span, diagnostic_name .. ".notes[" .. j .. "].spans[" .. k .. "]")
              -- Extract required params for additional spans
              span.required_params = extract_required_params(span)
            end
          end
          -- Extract required params for null checking (skip variadic notes, they're handled per-item)
          if not note.variadic then
            note.required_params = extract_required_params(note)
          end
        end
      end

      -- Now determine which params/locations go where:
      -- - If param appears ONLY in variadic context(s), it becomes a struct member
      -- - If param appears in non-variadic context (or multiple variadic contexts), it's a direct member
      local direct_params = {}
      local direct_locations = {}

      -- Iterate in order of first appearance
      for _, param_name in ipairs(param_order) do
        local ctx = param_contexts[param_name]
        local variadic_count = 0
        local single_struct = nil
        for struct_name, _ in pairs(ctx.variadic_structs) do
          variadic_count = variadic_count + 1
          single_struct = struct_name
        end

        if ctx.non_variadic or variadic_count > 1 then
          -- Direct member of diagnostic struct
          if ctx.is_location then
            table.insert(direct_locations, { name = param_name, type = seen_locations[param_name].type })
          else
            table.insert(direct_params, { name = param_name, type = seen_params[param_name] })
          end
        elseif variadic_count == 1 then
          -- Member of the variadic struct
          local vs = variadic_structs[single_struct]
          if ctx.is_location then
            table.insert(vs.locations, { name = param_name, type = seen_locations[param_name].type })
          else
            table.insert(vs.params, { name = param_name, type = seen_params[param_name] })
          end
        end
      end

      -- Convert variadic_structs map to list and add list_name (in order of registration)
      local variadic_structs_list = {}
      for _, struct_name in ipairs(variadic_structs_order) do
        local vs = variadic_structs[struct_name]
        vs.struct_name = struct_name
        vs.list_name = pluralize(struct_name:sub(1,1):lower() .. struct_name:sub(2))
        -- Extract required params for the variadic container
        vs.required_params = extract_required_params(vs.container)
        table.insert(variadic_structs_list, vs)
      end

      table.insert(processed, {
        name = diag.name,
        code = diag.code,
        severity = diag.severity,
        message = diag.message,
        message_parts = main_parts,
        message_required_params = main_required,
        primary_span_required_params = primary_span_required,
        params = direct_params,
        locations = direct_locations,
        variadic_structs = variadic_structs_list,
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
  variadic_span = variadic_span,
  variadic_note = variadic_note,
  err = err,
  warning = warning,
  process_diagnostics = process_diagnostics,
}
