-- !!!!!!
-- THIS FILE IS PROTOTYPE WORK, PLEASE USE source/slang/slang-diagnostic-defs.h
-- !!!!!!
--
-- Lua-based diagnostic definitions for the Slang compiler
--
-- Example usage:
--
-- err(
--     "function return type mismatch",      -- Diagnostic name
--     30007,                                 -- Diagnostic code
--     "expression type ~expression_type does not match function's return type ~return_type",
--     span("expression_location", "expression type"),        -- Primary span (required)
--     span("function_location", "function return type")      -- Secondary span (optional)
-- )
--
-- err(
--     "function redefinition",
--     30201,
--     "function ~name already has a body",
--     span("function_location", "redeclared here"),
--     note("original_location", "see previous definition of ~name")  -- Note: ~name is auto-deduplicated
-- )
--
-- Interpolation syntax:
--   ~param - Interpolates a parameter. If used multiple times, refers to the same parameter.
--            Parameters are automatically deduplicated across the entire diagnostic.
--
-- Available functions:
--   err(name, code, message, primary_span, ...) - Define an error diagnostic
--   warning(name, code, message, primary_span, ...) - Define a warning diagnostic
--   span(location, message) - Create a secondary span
--   note(location, message) - Create a note (appears after the main diagnostic)

-- Load helper functions
local helpers = dofile(debug.getinfo(1).source:match("@?(.*/)")  .. "slang-diagnostics-helpers.lua")
local span = helpers.span
local note = helpers.note
local err = helpers.err
local warning = helpers.warning

-- Define diagnostics
err(
	"function return type mismatch",
	30007,
	"expression type ~expression_type does not match function's return type ~return_type",
	span("expression_location", "expression type"),
	span("function_location", "function return type")
)

err(
	"function redefinition",
	30201,
	"function ~name already has a body",
	span("function_location", "redeclared here"),
	note("original_location", "see previous definition of ~name")
)

-- Process and validate all diagnostics
processed_diagnostics, validation_errors = helpers.process_diagnostics(helpers.diagnostics)

if #validation_errors > 0 then
	error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

return processed_diagnostics
