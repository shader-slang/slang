-- !!!!!!
-- THIS FILE IS PROTOTYPE WORK, PLEASE USE source/slang/slang-diagnostic-defs.h
-- !!!!!!
--
-- Lua-based diagnostic definitions for the Slang compiler
--
-- Code Generation Flow:
--   1. This file defines diagnostics using err()/warning() helper functions
--   2. slang-diagnostics-helpers.lua processes them, extracting:
--      - Locations from span() and note() calls (processed first)
--      - Parameters from ~interpolations (auto-deduplicated)
--      - Member accesses like ~expr.type are resolved using location types
--   3. slang-rich-diagnostics.h.lua loads processed diagnostics
--   4. FIDDLE templates in slang-rich-diagnostics.h generate C++ structs:
--      - Only direct parameters become member variables (not derived via member access)
--      - Locations become typed pointers (Expr*, Decl*) or SourceLoc
--   5. FIDDLE templates in slang-rich-diagnostics.cpp generate toGenericDiagnostic():
--      - Member accesses generate appropriate C++ (expr->type, decl->getName())
--      - Builds message string by interpolating parameters
--      - Sets primary span, secondary spans, and notes with their messages
--
-- Example usage:
--
-- err(
--     "function return type mismatch",
--     30007,
--     "expression type ~expression.type does not match function's return type ~return_type:Type",
--     span("expression:Expr", "expression type"),        -- Declares expression:Expr
--     span("function:Decl", "function return type")
-- )
-- ~expression.type automatically extracts expression->type (Type*)
-- ~return_type:Type is a direct parameter
--
-- err(
--     "function redefinition",
--     30201,
--     "function ~function already has a body",           -- Decl auto-uses .name
--     span("function:Decl", "redeclared here"),
--     note("original:Decl", "see previous definition of ~function")
-- )
-- ~function (a Decl) automatically becomes function->getName() when interpolated
--
-- err(
--     "type mismatch",
--     30008,
--     "cannot convert ~from:Type to ~to:Type",  -- Inline type declaration
--     span("expr:Expr", "expression here")
-- )
-- ~from:Type declares 'from' as Type* inline (no need to declare separately)
--
-- Interpolation syntax:
--   ~param           - String parameter (default)
--   ~param:Type      - Typed parameter (Type, Decl, Expr, Stmt, Val, Name, int)
--   ~param.member    - Member access (e.g., ~expr.type, ~decl.name)
--   ~param:Type.member - Inline type with member (e.g., ~foo:Expr.type)
--                     Parameters are automatically deduplicated.
--                     Decl types automatically use .name when interpolated directly.
--
-- Location syntax:
--   "location"         - Plain SourceLoc variable
--   "location:Type"    - Typed location (Decl->getNameLoc(), Expr->loc, etc.)
--                        Typed locations can be used as parameters in interpolations
--
-- Available functions:
--   err(name, code, message, primary_span, ...) - Define an error diagnostic
--   warning(name, code, message, primary_span, ...) - Define a warning diagnostic
--   span(location, message) - Create a secondary span
--   note(location, message) - Create a note (appears after the main diagnostic)

-- Load helper functions
local helpers = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-helpers.lua")
local span = helpers.span
local note = helpers.note
local err = helpers.err
local warning = helpers.warning

-- Define diagnostics
err(
  "function return type mismatch",
  30007,
  "expression type ~expression.type does not match function's return type ~return_type:Type",
  span("expression:Expr", "expression type"),
  span("function:Decl", "function return type")
)

err(
  "function redefinition",
  30201,
  "function '~function' already has a body",
  span("function:Decl", "redeclared here"),
  note("original:Decl", "see previous definition of '~function'")
)

-- Process and validate all diagnostics
processed_diagnostics, validation_errors = helpers.process_diagnostics(helpers.diagnostics)

if #validation_errors > 0 then
  error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

return processed_diagnostics
