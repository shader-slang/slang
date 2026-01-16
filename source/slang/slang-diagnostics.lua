-- !!!!!!
-- THIS FILE IS PROTOTYPE WORK, PLEASE USE source/slang/slang-diagnostic-defs.h
-- !!!!!!
--
-- Lua-based diagnostic definitions for the Slang compiler
--
-- Code Generation Flow:
--   1. This file defines diagnostics using err()/warning() helper functions
--   2. slang-diagnostics-helpers.lua processes them, extracting:
--      - Parameters from ~interpolations (auto-deduplicated)
--      - Locations from span() and note() calls
--   3. slang-rich-diagnostics.h.lua loads processed diagnostics
--   4. FIDDLE templates in slang-rich-diagnostics.h generate C++ structs:
--      - Parameters become typed member variables (String, Type*, Name*, int)
--      - Locations become SourceLoc member variables
--   5. FIDDLE templates in slang-rich-diagnostics.cpp generate toGenericDiagnostic():
--      - Builds message string by interpolating parameters
--      - Sets primary span, secondary spans, and notes with their messages
--
-- Example usage:
--
-- err(
--     "function return type mismatch",
--     30007,
--     "expression type ~expr_type:Type does not match function's return type ~return_type:Type",
--     span("expression:Expr", "expression type"),        -- Primary span (uses Expr*, extracts .loc)
--     span("function:Decl", "function return type")      -- Secondary span (uses Decl*, extracts .getNameLoc())
-- )
--
-- err(
--     "function redefinition",
--     30201,
--     "function ~name already has a body",               -- ~name defaults to String type
--     span("function_location", "redeclared here"),      -- Plain SourceLoc (no type suffix)
--     note("original_location", "see previous definition of ~name")
-- )
--
-- Interpolation syntax:
--   ~param        - String parameter (default)
--   ~param:Type   - Typed parameter (Type, Decl, Expr, Stmt, Val, Name, int)
--                   Parameters are automatically deduplicated across the entire diagnostic.
--
-- Location syntax:
--   "location"         - Plain SourceLoc variable
--   "location:Type"    - Typed location (Decl->getNameLoc(), Expr->loc, Type->loc, etc.)
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
	"expression type ~expression_type:Type does not match function's return type ~return_type:Type",
	span("expression:Expr", "expression type"),
	span("function:Decl", "function return type")
)

err(
	"function redefinition",
	30201,
	"function ~func_name:Name already has a body",
	span("function:Decl", "redeclared here"),
	note("original:Decl", "see previous definition of ~func_name")
)

-- Process and validate all diagnostics
processed_diagnostics, validation_errors = helpers.process_diagnostics(helpers.diagnostics)

if #validation_errors > 0 then
	error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

return processed_diagnostics
