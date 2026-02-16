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
--     span({loc = "expression:Expr", message = "expression type"}),        -- Declares expression:Expr
--     span({loc = "function:Decl", message = "function return type"})
-- )
-- ~expression.type automatically extracts expression->type (Type*)
-- ~return_type:Type is a direct parameter
--
-- err(
--     "function redefinition",
--     30201,
--     "function ~function already has a body",           -- Decl auto-uses .name
--     span({loc = "function:Decl", message = "redeclared here"}),
--     note({message = "see previous definition of '~function'", span({loc = "original:Decl"})})
-- )
-- ~function (a Decl) automatically becomes function->getName() when interpolated
-- note() requires a message and at least one span
--
-- err(
--     "type mismatch",
--     30008,
--     "cannot convert ~from:Type to ~to:Type",  -- Inline type declaration
--     span({loc = "expr:Expr", message = "expression here"})
-- )
-- ~from:Type declares 'from' as Type* inline (no need to declare separately)
--
-- err(
--     "multiple type errors",
--     30009,
--     "expression has multiple type errors",
--     span({loc = "expression:Expr", message = "expression here"}),
--     variadic_span({cpp_name = "Error", loc = "error_expr:Expr", message = "type error: ~error_expr.type"})
-- )
-- Variadic span: Creates nested struct Error with error_expr member, List<Error> errors
--
-- err(
--     "conflicting attributes",
--     30010,
--     "conflicting attributes on declaration",
--     span({loc = "decl:Decl", message = "declaration here"}),
--     note({message = "first attribute here",
--          span({loc = "attr1:Decl"}),
--          span({loc = "attr1_arg:Expr", message = "with argument"}),
--          span({loc = "attr1_type:Type", message = "of type"})})
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
--   err(name, code, message, [primary_span], ...) - Define an error diagnostic
--   warning(name, code, message, [primary_span], ...) - Define a warning diagnostic
--   Note: primary_span is optional for locationless diagnostics (e.g., command-line errors)
--
--   span(location, message?) - Create a span (message defaults to empty string)
--     Positional: span("location:Type", "message text")
--     Named:      span({loc = "location:Type", message = "message text"})
--
--   note(message, span1, span2, ...) - Create a note (appears after the main diagnostic)
--     Positional: note("message text", span(...), span(...))
--     Named:      note({message = "message text", span(...), span(...)})
--     message: The note's message
--     Requires at least one span. First span is primary, additional spans are secondary.
--     Cannot nest notes inside notes.
--
--   variadic_span(struct_name, location, message) - Create a variadic span
--     Positional: variadic_span("Error", "error_expr:Expr", "type error: ~error_expr.type")
--     Named:      variadic_span({cpp_name = "Error", loc = "error_expr:Expr", message = "type error: ~error_expr.type"})
--     struct_name/cpp_name: Name for nested struct (e.g., "Error" -> struct Error, List<Error> errors)
--     Exclusive interpolants become members of the nested struct
--
--   variadic_note(struct_name, message, span1, span2, ...) - Create a variadic note
--     Positional: variadic_note("Candidate", "candidate: ~sig", span(...))
--     Named:      variadic_note({cpp_name = "Candidate", message = "candidate: ~sig", span(...)})
--     struct_name/cpp_name: Name for nested struct (e.g., "Candidate" -> struct Candidate, List<Candidate> candidates)
--     message: The note's message
--     Requires at least one span. First span is primary, additional spans are secondary.
--     Cannot nest notes inside variadic notes.

-- Load helper functions
local helpers = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-helpers.lua")
local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

-- Define diagnostics
err(
    "function return type mismatch",
    30007,
    "expression type ~expression.type does not match function's return type ~return_type:Type",
    span { loc = "expression:Expr", message = "expression type" },
    span { loc = "function:Decl", message = "function return type" }
)

err(
    "function redefinition",
    30201,
    "function '~function' already has a body",
    span { loc = "function:Decl", message = "redeclared here" },
    note { message = "see previous definition of '~function'", span { loc = "original:Decl" } }
)

err(
    "multiple type errors",
    30202,
    "expression has multiple type errors",
    span { loc = "expression:Expr", message = "expression here" },
    variadic_span { cpp_name = "Error", loc = "error_expr:Expr", message = "type error: ~error_expr.type" }
)

err(
    "ambiguous overload for name with args",
    39999,
    "ambiguous call to '~name' with arguments of type ~args",
    span { loc = "expr:Expr", message = "in call expression" },
    variadic_note { cpp_name = "Candidate", message = "candidate: ~candidate_signature", span { loc = "candidate:Decl" } }
)

err(
    "note with spans test",
    39998,
    "this is a test diagnostic with spans in notes",
    span { loc = "expr:Expr", message = "main expression" },
    note {
        message = "a note",
        span { loc = "decl:Decl", message = "see declaration here" },
        span { loc = "attr:Decl", message = "with attribute" },
        span { loc = "param:Expr", message = "used here" },
    }
)

err(
    "subscript non array",
    30013,
    "invalid subscript expression",
    span { loc = "expr:Expr", message = "no subscript declarations found for type '~type:Type'" }
)

-- Conversion examples from slang-diagnostic-defs.h

err(
    "type mismatch",
    30019,
    "type mismatch in expression",
    span {
        loc = "expr:Expr",
        message = "expected an expression of type '~expected_type:Type', got '~actual_type:QualType'",
    }
)

warning(
    "macro redefinition",
    15400,
    "macro '~name:Name' is being redefined",
    span { loc = "location", message = "redefinition of macro '~name:Name'" },
    note { message = "see previous definition of '~name'", span { loc = "original_location" } }
)

err(
    "invalid swizzle expr",
    30052,
    "invalid swizzle expression",
    span { loc = "expr:Expr", message = "invalid swizzle pattern '~pattern' on type '~type:Type'" }
)

err(
    "expected prefix operator",
    39999,
    "function called as prefix operator was not declared `__prefix`",
    span { loc = "call_loc", message = "function called as prefix operator was not declared `__prefix`" },
    note { message = "see definition of '~decl'", span { loc = "decl:Decl" } }
)

err(
    "too many initializers",
    30500,
    "too many initializers in initializer list",
    span { loc = "init_list:Expr", message = "too many initializers (expected ~expected:int, got ~got:int)" }
)

err(
    "cannot convert array of smaller to larger size",
    30024,
    "array size mismatch prevents conversion",
    span {
        loc = "location",
        message = "Cannot convert array of size ~source_size:int to array of size ~target_size:int as this would truncate data",
    }
)

err(
    "unknown stage",
    15,
    "unknown stage '~stage_name'",
    span { loc = "location", message = "unknown stage '~stage_name'" }
)

--
-- 0xxxx - Command line and interaction with host platform APIs.
--

err(
    "cannot open file",
    1,
    "cannot open file '~path'",
    span { loc = "location", message = "cannot open file '~path'" }
)

err(
    "cannot find file",
    2,
    "cannot find file '~path'",
    span { loc = "location", message = "cannot find file '~path'" }
)

err(
    "cannot write output file",
    4,
    "cannot write output file '~path'",
    span { loc = "location", message = "cannot write output file '~path'" }
)

err(
    "failed to load dynamic library",
    5,
    "failed to load dynamic library '~path'",
    span { loc = "location", message = "failed to load dynamic library '~path'" }
)

err(
    "too many output paths specified",
    6,
    "too many output paths specified",
    span { loc = "location", message = "~output_count:int output paths specified, but only ~entry_point_count:int entry points given" }
)

err(
    "cannot deduce source language",
    12,
    "can't deduce language for input file '~path'"
)

err(
    "unknown code generation target",
    13,
    "unknown code generation target '~target'",
    span { loc = "location", message = "unknown code generation target '~target'" }
)

err(
    "unknown profile",
    14,
    "unknown profile '~profile'",
    span { loc = "location", message = "unknown profile '~profile'" }
)

err(
    "unknown pass through target",
    16,
    "unknown pass-through target '~target'",
    span { loc = "location", message = "unknown pass-through target '~target'" }
)

err(
    "unknown command line option",
    17,
    "unknown command-line option '~option'",
    span { loc = "location", message = "unknown command-line option '~option'" }
)

warning(
    "separate debug info unsupported for target",
    18,
    "'-separate-debug-info' is not supported for target '~target'"
)

err(
    "unknown source language",
    19,
    "unknown source language '~language'",
    span { loc = "location", message = "unknown source language '~language'" }
)

err(
    "entry points need to be associated with translation units",
    20,
    "when using multiple source files, entry points must be specified after their corresponding source file(s)"
)

err(
    "unknown downstream compiler",
    22,
    "unknown downstream compiler '~compiler'",
    span { loc = "location", message = "unknown downstream compiler '~compiler'" }
)

err(
    "unable to generate code for target",
    28,
    "unable to generate code for target '~target'"
)

warning(
    "same stage specified more than once",
    30,
    "the stage '~stage' was specified more than once for entry point '~entry_point'"
)

err(
    "conflicting stages for entry point",
    31,
    "conflicting stages have been specified for entry point '~entry_point'"
)

warning(
    "explicit stage doesnt match implied stage",
    32,
    "the stage specified for entry point '~entry_point' ('~specified_stage') does not match the stage implied by the source file name ('~implied_stage')"
)

err(
    "stage specification ignored because no entry points",
    33,
    "one or more stages were specified, but no entry points were specified with '-entry'"
)

err(
    "stage specification ignored because before all entry points",
    34,
    "when compiling multiple entry points, any '-stage' options must follow the '-entry' option that they apply to"
)

err(
    "no stage specified in pass through mode",
    35,
    "no stage was specified for entry point '~entry_point'; when using the '-pass-through' option, stages must be fully specified on the command line"
)

err(
    "expecting an integer",
    36,
    "expecting an integer value"
)

warning(
    "same profile specified more than once",
    40,
    "the '~profile' was specified more than once for target '~target'"
)

err(
    "conflicting profiles specified for target",
    41,
    "conflicting profiles have been specified for target '~target'"
)

err(
    "profile specification ignored because no targets",
    42,
    "a '-profile' option was specified, but no target was specified with '-target'"
)

err(
    "profile specification ignored because before all targets",
    43,
    "when using multiple targets, any '-profile' option must follow the '-target' it applies to"
)

err(
    "target flags ignored because no targets",
    44,
    "target options were specified, but no target was specified with '-target'"
)

err(
    "target flags ignored because before all targets",
    45,
    "when using multiple targets, any target options must follow the '-target' they apply to"
)

err(
    "duplicate targets",
    50,
    "the target '~target' has been specified more than once"
)

err(
    "unhandled language for source embedding",
    51,
    "unhandled source language for source embedding"
)

err(
    "cannot deduce output format from path",
    60,
    "cannot infer an output format from the output path '~path'"
)

err(
    "cannot match output file to target",
    61,
    "no specified '-target' option matches the output path '~path', which implies the '~format' format"
)

err(
    "unknown command line value",
    62,
    "unknown value for option. Valid values are '~valid_values'"
)

err(
    "unknown help category",
    63,
    "unknown help category"
)

err(
    "cannot match output file to entry point",
    70,
    "the output path '~path' is not associated with any entry point; a '-o' option for a compiled kernel must follow the '-entry' option for its corresponding entry point"
)

err(
    "invalid type conformance option string",
    71,
    "syntax error in type conformance option '~option'."
)

err(
    "invalid type conformance option no type",
    72,
    "invalid conformance option '~option', type '~type_name' is not found."
)

err(
    "cannot create type conformance",
    73,
    "cannot create type conformance '~conformance'."
)

err(
    "duplicate output paths for entry point and target",
    80,
    "multiple output paths have been specified entry point '~entry_point:Name' on target '~target'"
)

err(
    "duplicate output paths for target",
    81,
    "multiple output paths have been specified for target '~target'"
)

err(
    "duplicate dependency output paths",
    82,
    "the -dep argument can only be specified once"
)

err(
    "unable to write repro file",
    82,
    "unable to write repro file '~path'"
)

err(
    "unable to create module container",
    86,
    "unable to create module container"
)

err(
    "unable to set default downstream compiler",
    87,
    "unable to set default downstream compiler for source language '~language' to '~compiler'"
)

err(
    "expecting slang riff container",
    89,
    "expecting a slang riff container"
)

err(
    "incompatible riff semantic version",
    90,
    "incompatible riff semantic version ~actual_version expecting ~expected_version"
)

err(
    "riff hash mismatch",
    91,
    "riff hash mismatch - incompatible riff"
)

err(
    "unable to create directory",
    92,
    "unable to create directory '~path'"
)

err(
    "unable to extract repro to directory",
    93,
    "unable to extract repro to directory '~path'"
)

err(
    "unable to read riff",
    94,
    "unable to read as 'riff'/not a 'riff' file"
)

err(
    "unknown library kind",
    95,
    "unknown library kind '~kind'"
)

err(
    "kind not linkable",
    96,
    "not a known linkable kind '~kind'"
)

err(
    "library does not exist",
    97,
    "library '~path' does not exist"
)

err(
    "cannot access as blob",
    98,
    "cannot access as a blob"
)

err(
    "failed to load downstream compiler",
    100,
    "failed to load downstream compiler '~compiler'"
)

err(
    "downstream compiler doesnt support whole program compilation",
    101,
    "downstream compiler '~compiler' doesn't support whole program compilation"
)

standalone_note(
    "downstream compile time",
    102,
    "downstream compile time: ~time"
)

standalone_note(
    "performance benchmark result",
    103,
    "compiler performance benchmark:\\n~benchmark_output"
)

err(
    "need to enable experiment feature",
    104,
    "'~module' is an experimental module, need to enable '-experimental-feature' to load this module",
    span { loc = "loc" }
)

err(
    "null component type",
    105,
    "componentTypes[~index:Int] is `nullptr`"
)

standalone_note(
    "note failed to load dynamic library",
    99999,
    "failed to load dynamic library '~path'"
)

-- Process and validate all diagnostics
processed_diagnostics, validation_errors = helpers.process_diagnostics(helpers.diagnostics)

if #validation_errors > 0 then
    error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

return processed_diagnostics
