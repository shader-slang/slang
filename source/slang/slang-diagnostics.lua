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
--     note("see previous definition of '~function'", span("original:Decl"))
-- )
-- ~function (a Decl) automatically becomes function->getName() when interpolated
-- note(message, span, ...) requires a message and at least one span
--
-- err(
--     "type mismatch",
--     30008,
--     "cannot convert ~from:Type to ~to:Type",  -- Inline type declaration
--     span("expr:Expr", "expression here")
-- )
-- ~from:Type declares 'from' as Type* inline (no need to declare separately)
--
-- err(
--     "multiple type errors",
--     30009,
--     "expression has multiple type errors",
--     span("expression:Expr", "expression here"),
--     variadic_span("Error", "error_expr:Expr", "type error: ~error_expr.type")
-- )
-- Variadic span: Creates nested struct Error with error_expr member, List<Error> errors
--
-- err(
--     "conflicting attributes",
--     30010,
--     "conflicting attributes on declaration",
--     span("decl:Decl", "declaration here"),
--     note("first attribute here",
--          span("attr1:Decl"),
--          span("attr1_arg:Expr", "with argument"),
--          span("attr1_type:Type", "of type"))
-- )
-- Notes can have additional spans: First span is primary, additional spans are secondary
-- span() message parameter is optional and defaults to empty string
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
--   span(location, message?) - Create a span (message defaults to empty string)
--   note(message, span1, span2, ...) - Create a note (appears after the main diagnostic)
--     message: The note's message
--     Requires at least one span. First span is primary, additional spans are secondary.
--     Cannot nest notes inside notes.
--   variadic_span(struct_name, location, message) - Create a variadic span
--     struct_name: Name for nested struct (e.g., "Error" -> struct Error, List<Error> errors)
--     Exclusive interpolants become members of the nested struct
--   variadic_note(struct_name, message, span1, span2, ...) - Create a variadic note
--     struct_name: Name for nested struct (e.g., "Candidate" -> struct Candidate, List<Candidate> candidates)
--     message: The note's message
--     Requires at least one span. First span is primary, additional spans are secondary.
--     Cannot nest notes inside variadic notes.

-- Load helper functions
local helpers = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-helpers.lua")
local span = helpers.span
local note = helpers.note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
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
  note("see previous definition of '~function'", span("original:Decl"))
)

err(
  "multiple type errors",
  30202,
  "expression has multiple type errors",
  span("expression:Expr", "expression here"),
  variadic_span("Error", "error_expr:Expr", "type error: ~error_expr.type")
)

err(
  39999,
  "ambiguous overload for name with args",
  "ambiguous call to '~name' with arguments of type ~args",
  span({ loc = "expr:Expr", message = "in call expression" }),
  variadic_note({ cpp_name = "Candidate", message = "candidate: ~candidate_signature", span("candidate:Decl") })
)

err(
  "note with spans test",
  39998,
  "this is a test diagnostic with spans in notes",
  span("expr:Expr", "main expression"),
  note(
    "a note",
    span("decl:Decl", "see declaration here"),
    span("attr:Decl", "with attribute"),
    span("param:Expr", "used here")
  )
)

err("subscript non array", 30013, "no subscript operation found for type '~type:Type'", span("expr:Expr", "here"))

-- Process and validate all diagnostics
processed_diagnostics, validation_errors = helpers.process_diagnostics(helpers.diagnostics)

if #validation_errors > 0 then
  error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

return processed_diagnostics
