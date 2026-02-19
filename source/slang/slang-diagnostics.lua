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
    "macro redefined",
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

--
-- 15xxx - Preprocessing
--

-- 150xx - conditionals
err(
    "end of file in preprocessor conditional",
    15000,
    "end of file encountered during preprocessor conditional",
    span { loc = "location" },
    note { message = "see '~directive' directive", span { loc = "directive_loc" } }
)

err(
    "directive without if",
    15001,
    "'~directive' directive without '#if'",
    span { loc = "location", message = "'~directive' directive without '#if'" }
)

err(
    "directive after else",
    15002,
    "'~directive' directive after '#else'",
    span { loc = "location", message = "'~directive' directive after '#else'" },
    note { message = "see '~else_directive' directive", span { loc = "else_loc" } }
)

-- Standalone note for seeDirective - used by multiple diagnostics
standalone_note(
    "see directive",
    -1,
    "see '$0' directive"
)

-- 151xx - directive parsing
err(
    "expected preprocessor directive name",
    15100,
    "expected preprocessor directive name",
    span { loc = "location", message = "expected preprocessor directive name" }
)

err(
    "unknown preprocessor directive",
    15101,
    "unknown directive",
    span { loc = "location", message = "unknown preprocessor directive '~directive'" }
)

err(
    "expected token in preprocessor directive",
    15102,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~expected_token' in '~directive' directive" }
)

err(
    "expected 2 tokens in preprocessor directive",
    15102,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~token1' or '~token2' in '~directive' directive" }
)

err(
    "unexpected tokens after directive",
    15103,
    "unexpected tokens after directive",
    span { loc = "location", message = "unexpected tokens following '~directive' directive" }
)

-- 152xx - preprocessor expressions
err(
    "expected token in preprocessor expression",
    15200,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~expected_token' in preprocessor expression" },
    span { loc = "opening_loc", message = "opening '~opening_token'" }
)

err(
    "syntax error in preprocessor expression",
    15201,
    "preprocessor parse error",
    span { loc = "location", message = "syntax error in preprocessor expression" }
)

err(
    "divide by zero in preprocessor expression",
    15202,
    "division by zero",
    span { loc = "location", message = "division by zero in preprocessor expression" }
)

err(
    "expected token in defined expression",
    15203,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~expected_token' in 'defined' expression" },
    span { loc = "opening_loc", message = "opening '~opening_token'" }
)

warning(
    "directive expects expression",
    15204,
    "missing expression",
    span { loc = "location", message = "'~directive' directive requires an expression" }
)

warning(
    "undefined identifier in preprocessor expression",
    15205,
    "undefined identifier in preprocessor",
    span { loc = "location", message = "undefined identifier '~identifier' in preprocessor expression will evaluate to zero" }
)

err(
    "expected integral version number",
    15206,
    "expected integer version",
    span { loc = "location", message = "expected integer for #version number" }
)

err(
    "unknown language version",
    15207,
    "unknown language version",
    span { loc = "location", message = "unknown language version '~version'" }
)

err(
    "unknown language",
    15208,
    "unknown language",
    span { loc = "location", message = "unknown language '~language'" }
)

err(
    "language version differs from including module",
    15209,
    "language version mismatch",
    span { loc = "location", message = "the source file declares a different language version than the including module" }
)

-- Standalone note for includeOutput - used to output include hierarchy
standalone_note(
    "include output",
    -1,
    "include ~content"
)

-- Note for genericSignatureTried - used by overload resolution to point at the declaration
standalone_note(
    "generic signature tried",
    -1,
    "see declaration of ~signature",
    span { loc = "location" }
)

-- 153xx - #include
err(
    "include failed",
    15300,
    "include file not found",
    span { loc = "location", message = "failed to find include file '~path'" }
)

err(
    "import failed",
    15301,
    "import file not found",
    span { loc = "location", message = "failed to find imported file '~path'" }
)

err(
    "cyclic include",
    15302,
    "cyclic include",
    span { loc = "location", message = "cyclic `#include` of file '~path'" }
)

err(
    "no include handler specified",
    -1,
    "no include handler",
    span { loc = "location", message = "no `#include` handler was specified" }
)

err(
    "no unique identity",
    15302,
    "no unique file identity",
    span { loc = "location", message = "`#include` handler didn't generate a unique identity for file '~path'" }
)

err(
    "cannot resolve imported decl",
    15303,
    "cannot resolve imported declaration '~decl_name' from precompiled module '~module_name'. Make sure module '~module_name' is up-to-date. If you suspect this to be a compiler bug, file an issue on GitHub (https://github.com/shader-slang/slang/issues) or join the Slang Discord for assistance",
    span { loc = "location" }
)

-- 154xx - macro definition
warning(
    "macro not defined",
    15401,
    "undefined macro",
    span { loc = "location", message = "macro '~name' is not defined" }
)

err(
    "expected token in macro parameters",
    15403,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~expected_token' in macro parameters" }
)

warning(
    "builtin macro redefinition",
    15404,
    "builtin macro redefined",
    span { loc = "location", message = "Redefinition of builtin macro '~name'" }
)

err(
    "token paste at start",
    15405,
    "invalid '##' position",
    span { loc = "location", message = "'##' is not allowed at the start of a macro body" }
)

err(
    "token paste at end",
    15406,
    "invalid '##' position",
    span { loc = "location", message = "'##' is not allowed at the end of a macro body" }
)

err(
    "expected macro parameter after stringize",
    15407,
    "invalid '#' usage",
    span { loc = "location", message = "'#' in macro body must be followed by the name of a macro parameter" }
)

err(
    "duplicate macro parameter name",
    15408,
    "duplicate parameter",
    span { loc = "location", message = "redefinition of macro parameter '~name'" }
)

err(
    "variadic macro parameter must be last",
    15409,
    "variadic parameter must be last",
    span { loc = "location", message = "a variadic macro parameter is only allowed at the end of the parameter list" }
)

-- 155xx - macro expansion
warning(
    "expected token in macro arguments",
    15500,
    "macro invocation syntax error",
    span { loc = "location", message = "expected '~expected_token' in macro invocation" }
)

err(
    "wrong number of arguments to macro",
    15501,
    "wrong macro argument count",
    span { loc = "location", message = "wrong number of arguments to macro (expected ~expected:Int, got ~got:Int)" }
)

err(
    "error parsing to macro invocation argument",
    15502,
    "macro argument parse error",
    span { loc = "location", message = "error parsing macro '~arg_index:Int' invocation argument to '~macro_name:Name'" }
)

warning(
    "invalid token paste result",
    15503,
    "token paste failure",
    span { loc = "location", message = "token pasting with '##' resulted in the invalid token '~token'" }
)

-- 156xx - pragmas
err(
    "expected pragma directive name",
    15600,
    "expected pragma name",
    span { loc = "location", message = "expected a name after '#pragma'" }
)

warning(
    "unknown pragma directive ignored",
    15601,
    "unknown pragma ignored",
    span { loc = "location", message = "ignoring unknown directive '#pragma ~directive'" }
)

warning(
    "pragma once ignored",
    15602,
    "pragma once ignored",
    span { loc = "location", message = "pragma once was ignored - this is typically because it is not placed in an include" }
)

warning(
    "pragma warning pop empty",
    15611,
    "unmatched pop",
    span { loc = "location", message = "detected #pragma warning(pop) with no corresponding #pragma warning(push)" }
)

warning(
    "pragma warning push not popped",
    15612,
    "unmatched push",
    span { loc = "location", message = "detected #pragma warning(push) with no corresponding #pragma warning(pop)" }
)

warning(
    "pragma warning unknown specifier",
    15613,
    "unknown specifier",
    span { loc = "location", message = "unknown #pragma warning specifier '~specifier'" }
)

warning(
    "pragma warning suppress cannot identify next line",
    15614,
    "cannot identify suppress target",
    span { loc = "location", message = "cannot identify the next line to suppress in #pragma warning suppress" }
)

warning(
    "pragma warning cannot insert here",
    15615,
    "cannot insert pragma here",
    span { loc = "location", message = "cannot insert #pragma warning here for id '~id'" }
)

standalone_note(
    "pragma warning point suppress",
    15616,
    "#pragma warning for id '~id' was suppressed here",
    span { loc = "location" }
)

-- 159xx - user-defined error/warning
err(
    "user defined error",
    15900,
    "preprocessor error",
    span { loc = "location", message = "#error: ~message" }
)

warning(
    "user defined warning",
    15901,
    "preprocessor warning",
    span { loc = "location", message = "#warning: ~message" }
)

-- Include parsing diagnostics module
local parsing_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-parsing.lua")
parsing_module(helpers)

-- Load semantic checking diagnostics (part 1)
local semantic_checking_1_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-1.lua")
semantic_checking_1_module(helpers)

-- Load semantic checking diagnostics (part 2)
local semantic_checking_2_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-2.lua")
semantic_checking_2_module(helpers)

-- Load semantic checking diagnostics (part 3) - Include, Visibility, and Capability
local semantic_checking_3_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-3.lua")
semantic_checking_3_module(helpers)

-- Load semantic checking diagnostics (part 4) - Attributes
local semantic_checking_4_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-4.lua")
semantic_checking_4_module(helpers)

-- Load semantic checking diagnostics (part 5) - COM Interface, DerivativeMember, Extern Decl, Custom Derivative
local semantic_checking_5_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-5.lua")
semantic_checking_5_module(helpers)

-- Load semantic checking diagnostics (part 6) - Differentiation, Modifiers, GLSL/HLSL specifics, Interfaces, Control flow, Enums, Generics
local semantic_checking_6_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-6.lua")
semantic_checking_6_module(helpers)

-- Load semantic checking diagnostics (part 7) - Link Time, Cyclic Refs, Generics, Initializers, Variables, Parameters, Inheritance, Extensions, Subscripts
local semantic_checking_7_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-7.lua")
semantic_checking_7_module(helpers)

-- Load semantic checking diagnostics (part 8) - Accessors, Bit Fields, Integer Constants, Overloads, Switch, Generics, Ambiguity
local semantic_checking_8_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-8.lua")
semantic_checking_8_module(helpers)

-- Load semantic checking diagnostics (part 9) - Operators, Literals, Entry Points, Specialization
local semantic_checking_9_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-9.lua")
semantic_checking_9_module(helpers)

-- Load semantic checking diagnostics (part 10) - Interface Requirements, Global Generics, Differentiation, Modules
local semantic_checking_10_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-10.lua")
semantic_checking_10_module(helpers)

-- Load semantic checking diagnostics (part 11) - Type layout and parameter binding
local semantic_checking_11_module = dofile(debug.getinfo(1).source:match("@?(.*/)") .. "slang-diagnostics-semantic-checking-11.lua")
semantic_checking_11_module(helpers)

-- Process and validate all diagnostics
processed_diagnostics, validation_errors = helpers.process_diagnostics(helpers.diagnostics)

if #validation_errors > 0 then
    error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

return processed_diagnostics
