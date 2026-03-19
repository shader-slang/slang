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
--     "expression type ~expression.type does not match function's return type ~returnType:Type",
--     span({loc = "expression:Expr", message = "expression type"}),        -- Declares expression:Expr
--     span({loc = "function:Decl", message = "function return type"})
-- )
-- ~expression.type automatically extracts expression->type (Type*)
-- ~returnType:Type is a direct parameter
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
--     variadic_span({cpp_name = "Error", loc = "errorExpr:Expr", message = "type error: ~errorExpr.type"})
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
--          span({loc = "attr1Arg:Expr", message = "with argument"}),
--          span({loc = "attr1Type:Type", message = "of type"})})
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
--     Positional: variadic_span("Error", "errorExpr:Expr", "type error: ~errorExpr.type")
--     Named:      variadic_span({cpp_name = "Error", loc = "errorExpr:Expr", message = "type error: ~errorExpr.type"})
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
local internal = helpers.internal
local fatal = helpers.fatal

--
-- 0xxxx - Command line and interaction with host platform APIs.
--

err("cannot-open-file", 1, "cannot open file '~path'", span { loc = "location", message = "cannot open file '~path'" })

err("cannot-find-file", 2, "cannot find file '~path'", span { loc = "location", message = "cannot find file '~path'" })

err(
    "cannot-write-output-file",
    4,
    "cannot write output file '~path'",
    span { loc = "location", message = "cannot write output file '~path'" }
)

err(
    "failed-to-load-dynamic-library",
    5,
    "failed to load dynamic library '~path'",
    span { loc = "location", message = "failed to load dynamic library '~path'" }
)

err(
    "too-many-output-paths-specified",
    6,
    "too many output paths specified",
    span {
        loc = "location",
        message = "~outputCount:int output paths specified, but only ~entryPointCount:int entry points given",
    }
)

err("cannot-deduce-source-language", 12, "can't deduce language for input file '~path'")

err(
    "unknown-code-generation-target",
    13,
    "unknown code generation target '~target'",
    span { loc = "location", message = "unknown code generation target '~target'" }
)

err(
    "unknown-profile",
    14,
    "unknown profile '~profile'",
    span { loc = "location", message = "unknown profile '~profile'" }
)

err(
    "unknown-stage",
    15,
    "unknown stage '~stageName'",
    span { loc = "location", message = "unknown stage '~stageName'" }
)

err(
    "unknown-pass-through-target",
    16,
    "unknown pass-through target '~target'",
    span { loc = "location", message = "unknown pass-through target '~target'" }
)

err(
    "unknown-command-line-option",
    17,
    "unknown command-line option '~option'",
    span { loc = "location", message = "unknown command-line option '~option'" }
)

warning(
    "separate-debug-info-unsupported-for-target",
    18,
    "'-separate-debug-info' is not supported for target '~target'"
)

err(
    "unknown-source-language",
    19,
    "unknown source language '~language'",
    span { loc = "location", message = "unknown source language '~language'" }
)

err(
    "entry-points-need-to-be-associated-with-translation-units",
    20,
    "when using multiple source files, entry points must be specified after their corresponding source file(s)"
)

err(
    "unknown-downstream-compiler",
    22,
    "unknown downstream compiler '~compiler'",
    span { loc = "location", message = "unknown downstream compiler '~compiler'" }
)

err("unable-to-generate-code-for-target", 28, "unable to generate code for target '~target'")

warning(
    "same-stage-specified-more-than-once",
    30,
    "the stage '~stage' was specified more than once for entry point '~entryPoint'"
)

err("conflicting-stages-for-entry-point", 31, "conflicting stages have been specified for entry point '~entryPoint'")

warning(
    "explicit-stage-doesnt-match-implied-stage",
    32,
    "the stage specified for entry point '~entryPoint' ('~specifiedStage') does not match the stage implied by the source file name ('~impliedStage')"
)

err(
    "stage-specification-ignored-because-no-entry-points",
    33,
    "one or more stages were specified, but no entry points were specified with '-entry'"
)

err(
    "stage-specification-ignored-because-before-all-entry-points",
    34,
    "when compiling multiple entry points, any '-stage' options must follow the '-entry' option that they apply to"
)

err(
    "no-stage-specified-in-pass-through-mode",
    35,
    "no stage was specified for entry point '~entryPoint'; when using the '-pass-through' option, stages must be fully specified on the command line"
)

err(
    "expecting-an-integer",
    36,
    "expecting an integer value",
    span { loc = "location", message = "expecting an integer value for option '~value'." }
)

err(
    "expecting-a-unsigned-integer",
    37,
    "expecting an unsigned integer value",
    span { loc = "location", message = "expecting an unsigned integer value for option '~value', but got '~parsedValue:int'." }
)

warning("same-profile-specified-more-than-once", 40, "the '~profile' was specified more than once for target '~target'")

err("conflicting-profiles-specified-for-target", 41, "conflicting profiles have been specified for target '~target'")

err(
    "profile-specification-ignored-because-no-targets",
    42,
    "a '-profile' option was specified, but no target was specified with '-target'"
)

err(
    "profile-specification-ignored-because-before-all-targets",
    43,
    "when using multiple targets, any '-profile' option must follow the '-target' it applies to"
)

err(
    "target-flags-ignored-because-no-targets",
    44,
    "target options were specified, but no target was specified with '-target'"
)

err(
    "target-flags-ignored-because-before-all-targets",
    45,
    "when using multiple targets, any target options must follow the '-target' they apply to"
)

err("duplicate-targets", 50, "the target '~target' has been specified more than once")

err("unhandled-language-for-source-embedding", 51, "unhandled source language for source embedding")

err("cannot-deduce-output-format-from-path", 60, "cannot infer an output format from the output path '~path'")

err(
    "cannot-match-output-file-to-target",
    61,
    "no specified '-target' option matches the output path '~path', which implies the '~format' format"
)

err("unknown-command-line-value", 62, "unknown value for option. Valid values are '~validValues'")

err("unknown-help-category", 63, "unknown help category")

err(
    "cannot-match-output-file-to-entry-point",
    70,
    "the output path '~path' is not associated with any entry point; a '-o' option for a compiled kernel must follow the '-entry' option for its corresponding entry point"
)

err("invalid-type-conformance-option-string", 71, "syntax error in type conformance option '~option'.")

err(
    "invalid-type-conformance-option-no-type",
    72,
    "invalid conformance option '~option', type '~typeName' is not found."
)

err("cannot-create-type-conformance", 73, "cannot create type conformance '~conformance'.")

err(
    "duplicate-output-paths-for-entry-point-and-target",
    80,
    "multiple output paths have been specified entry point '~entryPoint:Name' on target '~target'"
)

err("duplicate-output-paths-for-target", 81, "multiple output paths have been specified for target '~target'")

err("duplicate-dependency-output-paths", 82, "the -dep argument can only be specified once")

err("unable-to-write-repro-file", 82, "unable to write repro file '~path'")

err("unable-to-create-module-container", 86, "unable to create module container")

err(
    "unable-to-set-default-downstream-compiler",
    87,
    "unable to set default downstream compiler for source language '~language' to '~compiler'"
)

err("expecting-slang-riff-container", 89, "expecting a slang riff container")

err(
    "incompatible-riff-semantic-version",
    90,
    "incompatible riff semantic version ~actualVersion expecting ~expectedVersion"
)

err("riff-hash-mismatch", 91, "riff hash mismatch - incompatible riff")

err("unable-to-create-directory", 92, "unable to create directory '~path'")

err("unable-to-extract-repro-to-directory", 93, "unable to extract repro to directory '~path'")

err("unable-to-read-riff", 94, "unable to read as 'riff'/not a 'riff' file")

err("unknown-library-kind", 95, "unknown library kind '~kind'")

err("kind-not-linkable", 96, "not a known linkable kind '~kind'")

err("library-does-not-exist", 97, "library '~path' does not exist")

err("cannot-access-as-blob", 98, "cannot access as a blob")

err("failed-to-load-downstream-compiler", 100, "failed to load downstream compiler '~compiler'")

err(
    "downstream-compiler-doesnt-support-whole-program-compilation",
    101,
    "downstream compiler '~compiler' doesn't support whole program compilation"
)

standalone_note("downstream-compile-time", 102, "downstream compile time: ~time")

standalone_note("performance-benchmark-result", 103, "compiler performance benchmark:\\n~benchmarkOutput")

err(
    "need-to-enable-experiment-feature",
    104,
    "'~module' is an experimental module, need to enable '-experimental-feature' to load this module",
    span { loc = "loc" }
)

err("null-component-type", 105, "componentTypes[~index:Int] is `nullptr`")

standalone_note("note-failed-to-load-dynamic-library", 99999, "failed to load dynamic library '~path'")

--
-- 15xxx - Preprocessing
--

-- 150xx - conditionals
err(
    "end-of-file-in-preprocessor-conditional",
    15000,
    "end of file encountered during preprocessor conditional",
    span { loc = "location" },
    note { message = "see '~directive' directive", span { loc = "directiveLoc" } }
)

err(
    "directive-without-if",
    15001,
    "'~directive' directive without '#if'",
    span { loc = "location", message = "'~directive' directive without '#if'" }
)

err(
    "directive-after-else",
    15002,
    "'~directive' directive after '#else'",
    span { loc = "location", message = "'~directive' directive after '#else'" },
    note { message = "see '~elseDirective' directive", span { loc = "elseLoc" } }
)

-- 151xx - directive parsing
err(
    "expected-preprocessor-directive-name",
    15100,
    "expected preprocessor directive name",
    span { loc = "location", message = "expected preprocessor directive name" }
)

err(
    "unknown-preprocessor-directive",
    15101,
    "unknown directive",
    span { loc = "location", message = "unknown preprocessor directive '~directive'" }
)

err(
    "expected-token-in-preprocessor-directive",
    15102,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~expectedToken' in '~directive' directive" }
)

err(
    "expected-2-tokens-in-preprocessor-directive",
    15102,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~token1' or '~token2' in '~directive' directive" }
)

err(
    "unexpected-tokens-after-directive",
    15103,
    "unexpected tokens after directive",
    span { loc = "location", message = "unexpected tokens following '~directive' directive" }
)

-- 152xx - preprocessor expressions
err(
    "expected-token-in-preprocessor-expression",
    15200,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~expectedToken' in preprocessor expression" },
    span { loc = "openingLoc", message = "opening '~openingToken'" }
)

err(
    "syntax-error-in-preprocessor-expression",
    15201,
    "preprocessor parse error",
    span { loc = "location", message = "syntax error in preprocessor expression" }
)

err(
    "divide-by-zero-in-preprocessor-expression",
    15202,
    "division by zero",
    span { loc = "location", message = "division by zero in preprocessor expression" }
)

err(
    "expected-token-in-defined-expression",
    15203,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~expectedToken' in 'defined' expression" },
    span { loc = "openingLoc", message = "opening '~openingToken'" }
)

warning(
    "directive-expects-expression",
    15204,
    "missing expression",
    span { loc = "location", message = "'~directive' directive requires an expression" }
)

warning(
    "undefined-identifier-in-preprocessor-expression",
    15205,
    "undefined identifier in preprocessor",
    span {
        loc = "location",
        message = "undefined identifier '~identifier' in preprocessor expression will evaluate to zero",
    }
)

err(
    "expected-integral-version-number",
    15206,
    "expected integer version",
    span { loc = "location", message = "expected integer for #version number" }
)

err(
    "unknown-language-version",
    15207,
    "unknown language version",
    span { loc = "location", message = "unknown language version '~version'" }
)

err("unknown-language", 15208, "unknown language", span { loc = "location", message = "unknown language '~language'" })

err(
    "language-version-differs-from-including-module",
    15209,
    "language version mismatch",
    span { loc = "location", message = "the source file declares a different language version than the including module" }
)

-- Standalone note for includeOutput - used to output include hierarchy
standalone_note("include-output", -1, "include ~content")

-- Note for genericSignatureTried - used by overload resolution to point at the declaration
standalone_note("generic-signature-tried", -1, "see declaration of ~signature", span { loc = "location" })

-- 153xx - #include
err(
    "include-failed",
    15300,
    "include file not found",
    span { loc = "location", message = "failed to find include file '~path'" }
)

err(
    "import-failed",
    15301,
    "import file not found",
    span { loc = "location", message = "failed to find imported file '~path'" }
)

err("cyclic-include", 15302, "cyclic include", span { loc = "location", message = "cyclic `#include` of file '~path'" })

err(
    "no-include-handler-specified",
    -1,
    "no include handler",
    span { loc = "location", message = "no `#include` handler was specified" }
)

err(
    "no-unique-identity",
    15302,
    "no unique file identity",
    span { loc = "location", message = "`#include` handler didn't generate a unique identity for file '~path'" }
)

err(
    "cannot-resolve-imported-decl",
    15303,
    "cannot resolve imported declaration '~declName' from precompiled module '~moduleName'. Make sure module '~moduleName' is up-to-date. If you suspect this to be a compiler bug, file an issue on GitHub (https://github.com/shader-slang/slang/issues) or join the Slang Discord for assistance",
    span { loc = "location" }
)

-- 154xx - macro definition
warning(
    "macro-redefinition",
    15400,
    "macro redefined",
    span { loc = "location", message = "redefinition of macro '~name:Name'" },
    note { message = "see previous definition of '~name'", span { loc = "originalLocation" } }
)

warning(
    "macro-not-defined",
    15401,
    "undefined macro",
    span { loc = "location", message = "macro '~name' is not defined" }
)

err(
    "expected-token-in-macro-parameters",
    15403,
    "preprocessor parse error",
    span { loc = "location", message = "expected '~expectedToken' in macro parameters" }
)

warning(
    "builtin-macro-redefinition",
    15404,
    "builtin macro redefined",
    span { loc = "location", message = "Redefinition of builtin macro '~name'" }
)

err(
    "token-paste-at-start",
    15405,
    "invalid '##' position",
    span { loc = "location", message = "'##' is not allowed at the start of a macro body" }
)

err(
    "token-paste-at-end",
    15406,
    "invalid '##' position",
    span { loc = "location", message = "'##' is not allowed at the end of a macro body" }
)

err(
    "expected-macro-parameter-after-stringize",
    15407,
    "invalid '#' usage",
    span { loc = "location", message = "'#' in macro body must be followed by the name of a macro parameter" }
)

err(
    "duplicate-macro-parameter-name",
    15408,
    "duplicate parameter",
    span { loc = "location", message = "redefinition of macro parameter '~name'" }
)

err(
    "variadic-macro-parameter-must-be-last",
    15409,
    "variadic parameter must be last",
    span { loc = "location", message = "a variadic macro parameter is only allowed at the end of the parameter list" }
)

-- 155xx - macro expansion
warning(
    "expected-token-in-macro-arguments",
    15500,
    "macro invocation syntax error",
    span { loc = "location", message = "expected '~expectedToken' in macro invocation" }
)

err(
    "wrong-number-of-arguments-to-macro",
    15501,
    "wrong macro argument count",
    span { loc = "location", message = "wrong number of arguments to macro (expected ~expected:Int, got ~got:Int)" }
)

err(
    "error-parsing-to-macro-invocation-argument",
    15502,
    "macro argument parse error",
    span { loc = "location", message = "error parsing macro '~argIndex:Int' invocation argument to '~macroName:Name'" }
)

warning(
    "invalid-token-paste-result",
    15503,
    "token paste failure",
    span { loc = "location", message = "token pasting with '##' resulted in the invalid token '~token'" }
)

-- 156xx - pragmas
err(
    "expected-pragma-directive-name",
    15600,
    "expected pragma name",
    span { loc = "location", message = "expected a name after '#pragma'" }
)

warning(
    "unknown-pragma-directive-ignored",
    15601,
    "unknown pragma ignored",
    span { loc = "location", message = "ignoring unknown directive '#pragma ~directive'" }
)

warning(
    "pragma-once-ignored",
    15602,
    "pragma once ignored",
    span {
        loc = "location",
        message = "pragma once was ignored - this is typically because it is not placed in an include",
    }
)

warning(
    "pragma-warning-pop-empty",
    15611,
    "unmatched pop",
    span { loc = "location", message = "detected #pragma warning(pop) with no corresponding #pragma warning(push)" }
)

warning(
    "pragma-warning-push-not-popped",
    15612,
    "unmatched push",
    span { loc = "location", message = "detected #pragma warning(push) with no corresponding #pragma warning(pop)" }
)

warning(
    "pragma-warning-unknown-specifier",
    15613,
    "unknown specifier",
    span { loc = "location", message = "unknown #pragma warning specifier '~specifier'" }
)

warning(
    "pragma-warning-suppress-cannot-identify-next-line",
    15614,
    "cannot identify suppress target",
    span { loc = "location", message = "cannot identify the next line to suppress in #pragma warning suppress" }
)

warning(
    "pragma-warning-cannot-insert-here",
    15615,
    "cannot insert pragma here",
    span { loc = "location", message = "cannot insert #pragma warning here for id '~id'" }
)

standalone_note(
    "pragma-warning-point-suppress",
    15616,
    "#pragma warning for id '~id' was suppressed here",
    span { loc = "location" }
)

-- 159xx - user-defined error/warning
err("user-defined-error", 15900, "preprocessor error", span { loc = "location", message = "#error: ~message" })

warning(
    "user-defined-warning",
    15901,
    "preprocessor warning",
    span { loc = "location", message = "#warning: ~message" }
)

-- Include parsing diagnostics module
-- (inlined from slang-diagnostics-parsing.lua)

--
-- 2xxxx - Parsing
--

err(
    "unexpected-token",
    20003,
    "unexpected token",
    span { loc = "location", message = "unexpected ~tokenType" }
)

err(
    "unexpected-token-expected-token-type",
    20001,
    "unexpected token",
    span { loc = "location", message = "unexpected ~actualToken, expected ~expectedToken" }
)

err(
    "unexpected-token-expected-token-name",
    20001,
    "unexpected token",
    span { loc = "location", message = "unexpected ~actualToken, expected '~expectedTokenName'" }
)

err(
    "token-name-expected-but-eof",
    0,
    "unexpected end of file",
    span { loc = "location", message = "\"~tokenName\" expected but end of file encountered." }
)

err(
    "token-type-expected-but-eof",
    0,
    "unexpected end of file",
    span { loc = "location", message = "~tokenType expected but end of file encountered." }
)

err(
    "token-name-expected",
    20001,
    "expected token",
    span { loc = "location", message = "\"~tokenName\" expected" }
)

err(
    "token-name-expected-but-eof2",
    20001,
    "unexpected end of file",
    span { loc = "location", message = "\"~tokenName\" expected but end of file encountered." }
)

err(
    "token-type-expected",
    20001,
    "expected token",
    span { loc = "location", message = "~tokenType expected" }
)

err(
    "token-type-expected-but-eof2",
    20001,
    "unexpected end of file",
    span { loc = "location", message = "~tokenType expected but end of file encountered." }
)

err(
    "type-name-expected-but",
    20001,
    "expected type name",
    span { loc = "location", message = "unexpected ~token, expected type name" }
)

err(
    "type-name-expected-but-eof",
    20001,
    "expected type name",
    span { loc = "location", message = "type name expected but end of file encountered." }
)

err(
    "unexpected-eof",
    20001,
    "unexpected end of file",
    span { loc = "location", message = "Unexpected end of file." }
)

err(
    "syntax-error",
    20002,
    "syntax error",
    span { loc = "location", message = "syntax error." }
)

err(
    "invalid-empty-parenthesis-expr",
    20005,
    "invalid empty parentheses expression",
    span { loc = "location", message = "empty parenthesis '()' is not a valid expression." }
)

err(
    "invalid-operator",
    20008,
    "invalid operator",
    span { loc = "location", message = "invalid operator '~op'." }
)

err(
    "invalid-spirv-version",
    20012,
    "invalid SPIR-V version",
    span { loc = "location", message = "Expecting SPIR-V version as either 'major.minor', or quoted if has patch (eg for SPIR-V 1.2, '1.2' or \"1.2\"')" }
)

err(
    "invalid-cuda-sm-version",
    20013,
    "invalid CUDA SM version",
    span { loc = "location", message = "Expecting CUDA SM version as either 'major.minor', or quoted if has patch (eg for '7.0' or \"7.0\"')" }
)

err(
    "missing-layout-binding-modifier",
    20016,
    "missing 'binding' modifier",
    span { loc = "location", message = "Expecting 'binding' modifier in the layout qualifier here" }
)

err(
    "const-not-allowed-on-c-style-ptr-decl",
    20017,
    "'const' not allowed on C-style pointer declaration",
    span { loc = "location", message = "'const' not allowed on pointer typed declarations using the C style '*' operator. If the intent is to restrict the pointed-to value to read-only, use 'Ptr<T, Access.Read>'; if the intent is to make the pointer itself immutable, use 'let' or 'const Ptr<...>'." }
)

err(
    "const-not-allowed-on-type",
    20018,
    "invalid 'const' usage",
    span { loc = "location", message = "cannot use 'const' as a type modifier" }
)

warning(
    "unintended-empty-statement",
    20101,
    "potentially unintended empty statement",
    span { loc = "location", message = "potentially unintended empty statement at this location; use {} instead." }
)

err(
    "unexpected-body-after-semicolon",
    20102,
    "unexpected function body after semicolon",
    span { loc = "location", message = "unexpected function body after signature declaration, is this ';' a typo?" }
)

err(
    "decl-not-allowed",
    30102,
    "declaration not allowed here",
    span { loc = "location", message = "~declType is not allowed here." }
)

-- 29xxx - Snippet parsing and inline asm

err(
    "snippet-parsing-failed",
    29000,
    "snippet parsing failed",
    span { loc = "location", message = "unable to parse target intrinsic snippet: ~snippet" }
)

err(
    "unrecognized-spirv-opcode",
    29100,
    "unrecognized SPIR-V opcode",
    span { loc = "location", message = "unrecognized spirv opcode: ~opcode" }
)

err(
    "misplaced-result-id-marker",
    29101,
    "misplaced result-id marker",
    span { loc = "location", message = "the result-id marker must only be used in the last instruction of a spriv_asm expression" }
)

standalone_note(
    "consider-op-copy-object",
    29102,
    "consider adding an OpCopyObject instruction to the end of the spirv_asm expression"
)

standalone_note(
    "no-such-address",
    29103,
    "unable to take the address of this address-of asm operand",
    span { loc = "location" }
)

err(
    "spirv-instruction-without-result-id",
    29104,
    "instruction has no result-id operand",
    span { loc = "location", message = "cannot use this 'x = ~opcode ...' syntax because ~opcode does not have a <result-id> operand" }
)

err(
    "spirv-instruction-without-result-type-id",
    29105,
    "instruction has no result-type-id operand",
    span { loc = "location", message = "cannot use this 'x : <type> = ~opcode ...' syntax because ~opcode does not have a <result-type-id> operand" }
)

warning(
    "spirv-instruction-with-too-many-operands",
    29106,
    "too many operands",
    span { loc = "location", message = "too many operands for ~opcode (expected max ~maxOperands:Int), did you forget a semicolon?" }
)

err(
    "spirv-unable-to-resolve-name",
    29107,
    "unknown SPIR-V identifier",
    span { loc = "location", message = "unknown SPIR-V identifier ~name, it's not a known enumerator or opcode" }
)

err(
    "spirv-non-constant-bitwise-or",
    29108,
    "invalid bitwise or operand",
    span { loc = "location", message = "only integer literals and enum names can appear in a bitwise or expression" }
)

err(
    "spirv-operand-range",
    29109,
    "literal integer out of range",
    span { loc = "location", message = "Literal ints must be in the range 0 to 0xffffffff" }
)

err(
    "unknown-target-name",
    29110,
    "unknown target name",
    span { loc = "location", message = "unknown target name '~name'" }
)

err(
    "spirv-invalid-truncate",
    29111,
    "invalid truncate operation",
    span { loc = "location", message = "__truncate has been given a source smaller than its target" }
)

err(
    "spirv-instruction-with-not-enough-operands",
    29112,
    "not enough operands",
    span { loc = "location", message = "not enough operands for ~opcode" }
)

err(
    "spirv-id-redefinition",
    29113,
    "SPIR-V id redefinition",
    span { loc = "location", message = "SPIRV id '%~id' is already defined in the current assembly block" }
)

err(
    "spirv-undefined-id",
    29114,
    "undefined SPIR-V id",
    span { loc = "location", message = "SPIRV id '%~id' is not defined in the current assembly block location" }
)

err(
    "target-switch-case-cannot-be-a-stage",
    29115,
    "cannot use stage name in __target_switch",
    span { loc = "location", message = "cannot use a stage name in '__target_switch', use '__stage_switch' for stage-specific code." }
)


-- Load semantic checking diagnostics (part 1)
-- (inlined from slang-diagnostics-semantic-checking-1.lua)

--
-- 3xxxx - Semantic analysis
--

err(
    "divide-by-zero",
    30002,
    "divide by zero",
    span { loc = "location" }
)

err(
    "break-outside-loop",
    30003,
    "'break' must appear inside loop or switch constructs",
    span { loc = "stmt:Stmt", message = "'break' must appear inside loop or switch constructs." }
)

err(
    "continue-outside-loop",
    30004,
    "'continue' must appear inside loop constructs",
    span { loc = "stmt:Stmt", message = "'continue' must appear inside loop constructs." }
)

err(
    "return-needs-expression",
    30006,
    "'return' should have an expression",
    span { loc = "stmt:Stmt", message = "'return' should have an expression." }
)

err(
    "invalid-type-void",
    30009,
    "invalid type 'void'",
    span { loc = "location", message = "invalid type 'void'." }
)

err(
    "assign-non-lvalue",
    30011,
    "left of '=' is not an l-value",
    span { loc = "expr:Expr", message = "left of '=' is not an l-value." }
)

err(
    "subscript-non-array",
    30013,
    "invalid subscript expression",
    span { loc = "expr:Expr", message = "no subscript declarations found for type '~type:Type'" }
)

err(
    "call-operator-not-found",
    30016,
    "no call operation found for type",
    span { loc = "expr:Expr", message = "no call operation found for type '~type:Type'" }
)

err(
    "undefined-identifier",
    30015,
    "undefined identifier",
    span { loc = "location", message = "undefined identifier '~name:Name'." }
)

err(
    "type-mismatch",
    30019,
    "type mismatch in expression",
    span {
        loc = "expr:Expr",
        message = "expected an expression of type '~expectedType:Type', got '~actualType:QualType'",
    }
)

err(
    "cannot-convert-array-of-smaller-to-larger-size",
    30024,
    "array size mismatch prevents conversion",
    span {
        loc = "location",
        message = "Cannot convert array of size ~sourceSize:int to array of size ~targetSize:int as this would truncate data",
    }
)

err(
    "invalid-array-size",
    30025,
    "array size must be non-negative",
    span { loc = "location", message = "array size must be non-negative." }
)

err(
    "disallowed-array-of-non-addressable-type",
    30028,
    "arrays of non-addressable type not allowed",
    span { loc = "location", message = "Arrays of non-addressable type '~elementType:Type' are not allowed" }
)

err(
    "non-addressable-type-in-structured-buffer",
    30028,
    "non-addressable type cannot be used in StructuredBuffer",
    span { loc = "location", message = "'~type:Type' is non-addressable and cannot be used in StructuredBuffer" }
)

err(
    "array-index-out-of-bounds",
    30029,
    "array index out of bounds",
    span { loc = "location", message = "array index '~index' is out of bounds for array of size '~size'." }
)

err(
    "no-member-of-name-in-type",
    30027,
    "member not found",
    span { loc = "expr:Expr", message = "'~name:Name' is not a member of '~type:Type'." }
)

err(
    "argument-expected-lvalue",
    30047,
    "argument must be l-value",
    span { loc = "arg:Expr", message = "argument passed to parameter '~param' must be l-value." }
)

err(
    "argument-has-more-memory-qualifiers-than-param",
    30048,
    "memory qualifier mismatch",
    span { loc = "arg:Expr", message = "argument passed in to parameter has a memory qualifier the parameter type is missing: '~qualifier'" }
)

standalone_note(
    "attempting-to-assign-to-const-variable",
    30049,
    "attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification",
    span { loc = "expr:Expr" }
)

err(
    "mutating-method-on-immutable-value",
    30050,
    "mutating method cannot be called on immutable value",
    span { loc = "location", message = "mutating method '~methodName:Name' cannot be called on an immutable value" }
)

err(
    "invalid-swizzle-expr",
    30052,
    "invalid swizzle expression",
    span { loc = "expr:Expr", message = "invalid swizzle pattern '~pattern' on type '~type:Type'" }
)

err(
    "break-label-not-found",
    30053,
    "break label not found",
    span { loc = "stmt:Stmt", message = "label '~label:Name' used as break target is not found." }
)

err(
    "target-label-does-not-mark-breakable-stmt",
    30054,
    "invalid break target",
    span { loc = "stmt:Stmt", message = "invalid break target: statement labeled '~label:Name' is not breakable." }
)

err(
    "use-of-non-short-circuiting-operator-in-diff-func",
    30055,
    "non-short-circuiting `?:` not allowed in differentiable function",
    span { loc = "location", message = "non-short-circuiting `?:` operator is not allowed in a differentiable function, use `select` instead." }
)

warning(
    "use-of-non-short-circuiting-operator",
    30056,
    "non-short-circuiting `?:` is deprecated",
    span { loc = "location", message = "non-short-circuiting `?:` operator is deprecated, use 'select' instead." }
)

err(
    "assignment-in-predicate-expr",
    30057,
    "assignment in predicate expression not allowed",
    span { loc = "expr:Expr", message = "use an assignment operation as predicate expression is not allowed, wrap the assignment with '()' to clarify the intent." }
)

warning(
    "dangling-equality-expr",
    30058,
    "result of '==' not used",
    span { loc = "expr:Expr", message = "result of '==' not used, did you intend '='?" }
)

err(
    "expected-a-type",
    30060,
    "expected a type",
    span { loc = "expr:Expr", message = "expected a type, got a '~whatWeGot'" }
)

err(
    "expected-a-namespace",
    30061,
    "expected a namespace",
    span { loc = "expr:Expr", message = "expected a namespace, got a '~expr.type'" }
)

standalone_note(
    "implicit-cast-used-as-lvalue-ref",
    30062,
    "argument was implicitly cast from '~from:Type' to '~to:Type', and Slang does not support using an implicit cast as an l-value with a reference",
    span { loc = "expr:Expr" }
)

standalone_note(
    "implicit-cast-used-as-lvalue-type",
    30063,
    "argument was implicitly cast from '~from:Type' to '~to:Type', and Slang does not support using an implicit cast as an l-value with this type",
    span { loc = "expr:Expr" }
)

standalone_note(
    "implicit-cast-used-as-lvalue",
    30064,
    "argument was implicitly cast from '~from:Type' to '~to:Type', and Slang does not support using an implicit cast as an l-value for this usage",
    span { loc = "expr:Expr" }
)

err(
    "new-can-only-be-used-to-initialize-a-class",
    30065,
    "`new` can only be used to initialize a class",
    span { loc = "expr:Expr", message = "`new` can only be used to initialize a class" }
)

err(
    "class-can-only-be-initialized-with-new",
    30066,
    "class can only be initialized by `new`",
    span { loc = "expr:Expr", message = "a class can only be initialized by a `new` clause" }
)

err(
    "mutating-method-on-function-input-parameter-error",
    30067,
    "mutating method called on `in` parameter",
    span { loc = "location", message = "mutating method '~method:Name' called on `in` parameter '~param:Name'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended" }
)

warning(
    "mutating-method-on-function-input-parameter-warning",
    30068,
    "mutating method called on `in` parameter",
    span { loc = "location", message = "mutating method '~method:Name' called on `in` parameter '~param:Name'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended" }
)

err(
    "unsized-member-must-appear-last",
    30070,
    "unsized member must be last",
    span { loc = "decl:Decl", message = "member with unknown size at compile time can only appear as the last member in a composite type." }
)

err(
    "var-cannot-be-unsized",
    30071,
    "cannot instantiate unsized type",
    span { loc = "decl:Decl", message = "cannot instantiate a variable of unsized type." }
)

err(
    "param-cannot-be-unsized",
    30072,
    "function parameter cannot be unsized",
    span { loc = "decl:Decl", message = "function parameter cannot be unsized." }
)

err(
    "cannot-specialize-generic",
    30075,
    "cannot specialize generic",
    span { loc = "location", message = "cannot specialize generic '~generic:Decl' with the provided arguments." }
)

err(
    "global-var-cannot-have-opaque-type",
    30076,
    "global variable cannot have opaque type",
    span { loc = "decl:Decl", message = "global variable cannot have opaque type." }
)

err(
    "concrete-argument-to-output-interface",
    30077,
    "concrete type passed to interface-typed output parameter",
    span { loc = "location", message = "argument passed to parameter '~paramName' is of concrete type '~argType:QualType', but interface-typed output parameters require interface-typed arguments. To allow passing a concrete type to this function, you can replace '~paramType:Type ~paramName' with a generic 'T ~paramName' and a 'where T : ~paramType:Type' constraint." }
)

standalone_note(
    "do-you-mean-static-const",
    -1,
    "do you intend to define a `static const` instead?",
    span { loc = "decl:Decl" }
)

standalone_note(
    "do-you-mean-uniform",
    -1,
    "do you intend to define a `uniform` parameter instead?",
    span { loc = "decl:Decl" }
)

err(
    "coherent-keyword-on-a-pointer",
    30078,
    "cannot have coherent qualifier on pointer",
    span { loc = "decl:Decl", message = "cannot have a `globallycoherent T*` or a `coherent T*`, use explicit methods for coherent operations instead" }
)

err(
    "cannot-take-constant-pointers",
    30079,
    "cannot take address of immutable object",
    span { loc = "expr:Expr", message = "Not allowed to take the address of an immutable object" }
)

err(
    "cannot-specialize-generic-with-existential",
    33180,
    "cannot specialize generic with existential type",
    span { loc = "location", message = "specializing '~generic' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time." }
)

err(
    "static-ref-to-non-static-member",
    30100,
    "type cannot refer to non-static member",
    span { loc = "location", message = "type '~type:Type' cannot be used to refer to non-static member '~member:Name'" }
)

err(
    "cannot-dereference-type",
    30101,
    "cannot dereference type",
    span { loc = "location", message = "cannot dereference type '~type:Type', do you mean to use '.'?" }
)

err(
    "static-ref-to-this",
    30102,
    "static function cannot refer to non-static member via `this`",
    span { loc = "location", message = "static function cannot refer to non-static member `~member:Name` via `this`" }
)

err(
    "redeclaration",
    30200,
    "conflicting declaration",
    span { loc = "decl:Decl", message = "declaration of '~decl' conflicts with existing declaration" }
)

err(
    "function-redefinition",
    30201,
    "function '~function' already has a body",
    span { loc = "function:Decl", message = "redeclared here" },
    note { message = "see previous definition of '~function'", span { loc = "original:Decl" } }
)

err(
    "function-redeclaration-with-different-return-type",
    30202,
    "function return type mismatch",
    span { loc = "decl:Decl", message = "function '~decl' declared to return '~newReturnType:Type' was previously declared to return '~prevReturnType:Type'" }
)

err(
    "is-operator-value-must-be-interface-type",
    30300,
    "'is'/'as' operator requires interface-typed expression",
    span { loc = "expr:Expr", message = "'is'/'as' operator requires an interface-typed expression." }
)

err(
    "is-operator-cannot-use-interface-as-rhs",
    30301,
    "cannot use 'is' with interface on right-hand side",
    span { loc = "expr:Expr", message = "cannot use 'is' operator with an interface type as the right-hand side without a corresponding optional constraint. Use a concrete type instead, or add an optional constraint for the interface type." }
)

err(
    "as-operator-cannot-use-interface-as-rhs",
    30302,
    "cannot use 'as' with interface on right-hand side",
    span { loc = "expr:Expr", message = "cannot use 'as' operator with an interface type as the right-hand side. Use a concrete type instead. If you want to use an optional constraint, use an 'if (T is IInterface)' block instead." }
)

err(
    "expected-function",
    33070,
    "expected a function",
    span { loc = "expr:Expr", message = "expected a function, got '~expr.type'" }
)

err(
    "expected-a-string-literal",
    33071,
    "expected a string literal",
    span { loc = "expr:Expr", message = "expected a string literal" }
)

err(
    "expected-array-expression",
    30020,
    "expected an array expression",
    span { loc = "expr:Expr", message = "expected an array expression, got '~actualType:QualType'" }
)


-- Load semantic checking diagnostics (part 2)
-- (inlined from slang-diagnostics-semantic-checking-2.lua)

--
-- 3xxxx - Semantic analysis (continued)
--

-- `dyn` and `some` errors

err(
    "cannot-have-generic-dyn-interface",
    33072,
    "dyn interfaces cannot be generic",
    span { loc = "decl:Decl", message = "dyn interfaces cannot be generic: '~decl'." }
)

err(
    "cannot-have-associated-type-in-dyn-interface",
    33073,
    "dyn interfaces cannot have associatedType members",
    span { loc = "member:Decl", message = "dyn interfaces cannot have associatedType members." }
)

err(
    "cannot-have-generic-method-in-dyn-interface",
    33074,
    "dyn interfaces cannot have generic methods",
    span { loc = "member:Decl", message = "dyn interfaces cannot have generic methods." }
)

err(
    "cannot-have-mutating-method-in-dyn-interface",
    33075,
    "dyn interfaces cannot have [mutating] methods",
    span { loc = "member:Decl", message = "dyn interfaces cannot have [mutating] methods." }
)

err(
    "cannot-have-differentiable-method-in-dyn-interface",
    33076,
    "dyn interfaces cannot have [Differentiable] methods",
    span { loc = "member:Decl", message = "dyn interfaces cannot have [Differentiable] methods." }
)

err(
    "dyn-interface-cannot-inherit-non-dyn-interface",
    33077,
    "dyn interface inheritance error",
    span { loc = "inheritance:Decl", message = "dyn interface '~dynInterface:Decl' may only inherit 'dyn' interfaces. '~inherited:Decl' is not a dyn interface." }
)

err(
    "cannot-use-extension-to-make-type-conform-to-dyn-interface",
    33078,
    "extension cannot conform to dyn interface",
    span { loc = "extension:Decl", message = "cannot use a extension to conform to a dyn interface '~interface:Decl'." }
)

err(
    "cannot-have-unsized-member-when-inheriting-dyn-interface",
    33079,
    "unsized member with dyn interface inheritance",
    span { loc = "member:Decl", message = "cannot have unsized member '~member' when inheriting from dyn interface '~interface:Decl'." }
)

err(
    "cannot-have-opaque-member-when-inheriting-dyn-interface",
    33080,
    "opaque member with dyn interface inheritance",
    span { loc = "member:Decl", message = "cannot have opaque member '~member' when inheriting from dyn interface '~interface:Decl'." }
)

err(
    "cannot-have-non-copyable-member-when-inheriting-dyn-interface",
    33081,
    "non-copyable member with dyn interface inheritance",
    span { loc = "member:Decl", message = "cannot have non-copyable member '~member' when inheriting from dyn interface '~interface:Decl'." }
)

err(
    "cannot-conform-generic-to-dyn-interface",
    33082,
    "generic type cannot conform to dyn interface",
    span { loc = "inheritance:Decl", message = "cannot conform generic type '~generic:Decl' to dyn interface '~interface:Decl'." }
)

-- Conversion diagnostics

err(
    "ambiguous-conversion",
    30080,
    "ambiguous conversion",
    span { loc = "expr:Expr", message = "more than one conversion exists from '~fromType:Type' to '~toType:Type'" }
)

warning(
    "unrecommended-implicit-conversion",
    30081,
    "implicit conversion not recommended",
    span { loc = "expr:Expr", message = "implicit conversion from '~fromType:Type' to '~toType:Type' is not recommended" }
)

warning(
    "implicit-conversion-to-double",
    30082,
    "implicit float-to-double conversion",
    span { loc = "expr:Expr", message = "implicit float-to-double conversion may cause unexpected performance issues, use explicit cast if intended." }
)

-- try/throw diagnostics

err(
    "try-clause-must-apply-to-invoke-expr",
    30090,
    "expression in 'try' must be a call",
    span { loc = "expr:Expr", message = "expression in a 'try' clause must be a call to a function or operator overload." }
)

err(
    "try-invoke-callee-should-throw",
    30091,
    "callee in 'try' does not throw",
    span { loc = "expr:Expr", message = "'~callee:Decl' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'" }
)

err(
    "callee-of-try-call-must-be-func",
    30092,
    "callee in 'try' must be a function",
    span { loc = "expr:Expr", message = "callee in a 'try' clause must be a function" }
)

err(
    "uncaught-try-call-in-non-throw-func",
    30093,
    "uncaught 'try' in non-throwing function",
    span { loc = "expr:Expr", message = "the current function or environment is not declared to throw any errors, but the 'try' clause is not caught" }
)

err(
    "must-use-try-clause-to-call-a-throw-func",
    30094,
    "callee may throw, use 'try'",
    span { loc = "invoke:Expr", message = "the callee may throw an error, and therefore must be called within a 'try' clause" }
)

err(
    "error-type-of-callee-incompatible-with-caller",
    30095,
    "incompatible error types",
    span { loc = "expr:Expr", message = "the error type `~calleeErrorType:Type` of callee `~callee:Decl` is not compatible with the caller's error type `~callerErrorType:Type`." }
)

-- Differentiable diagnostics

err(
    "differential-type-should-serve-as-its-own-differential-type",
    30096,
    "invalid differential type",
    span { loc = "inheritance:Decl", message = "cannot use type '~type:Type' as a `Differential` type. A differential type's differential must be itself. However, the Differential of '~type:Type' is '~diffType:Type'." }
)

err(
    "function-not-marked-as-differentiable",
    30097,
    "function not differentiable",
    span { loc = "expr:Expr", message = "function '~func:Decl' is not marked as ~kind-differentiable." }
)

err(
    "non-static-member-function-not-allowed-as-diff-operand",
    30098,
    "non-static function reference not allowed",
    span { loc = "expr:Expr", message = "non-static function reference '~func:Decl' is not allowed here." }
)

-- sizeof/countof diagnostics

err(
    "size-of-argument-is-invalid",
    30099,
    "invalid sizeof argument",
    span { loc = "expr:Expr", message = "argument to sizeof is invalid" }
)

err(
    "count-of-argument-is-invalid",
    30083,
    "invalid countof argument",
    span { loc = "expr:Expr", message = "argument to countof can only be a type pack or tuple" }
)

err(
    "pack-query-argument-is-invalid",
    30410,
    "invalid ~queryName argument",
    span { loc = "expr:Expr", message = "argument to ~queryName is invalid" }
)

err(
    "empty-pack-query-is-invalid",
    30411,
    "cannot apply ~queryName to an empty pack",
    span { loc = "expr:Expr", message = "~queryName requires a non-empty pack" }
)

err(
    "pack-query-requires-non-empty-pack",
    30412,
    "cannot apply ~queryName to a pack that may be empty",
    span { loc = "expr:Expr", message = "~queryName requires a non-empty pack; add a `where nonempty(...)` constraint or use a structurally non-empty pack" }
)

err(
    "invalid-non-empty-pack-constraint-target",
    30413,
    "`nonempty(...)` requires a generic type pack or value pack parameter",
    span { loc = "expr:Expr", message = "expected a direct reference to a generic pack parameter here" }
)

err(
    "empty-pack-does-not-satisfy-non-empty-constraint",
    30414,
    "empty pack does not satisfy `nonempty(...)` constraint",
    span { loc = "location", message = "pack argument here is empty" }
)

err(
    "optional-non-empty-pack-constraint-is-invalid",
    30415,
    "`optional nonempty(...)` is not meaningful",
    span { loc = "expr:Expr", message = "remove `optional` from this `nonempty(...)` constraint" }
)

err(
    "non-empty-pack-constraint-target-must-be-from-current-generic",
    30416,
    "`nonempty(...)` target must be a generic pack parameter declared in the current generic",
    span { loc = "expr:Expr", message = "this pack parameter is declared outside the current generic" }
)

-- Float bit cast diagnostics

err(
    "float-bit-cast-type-mismatch",
    30084,
    "bit cast type mismatch",
    span { loc = "expr:Expr", message = "'~intrinsic' requires a ~expectedType argument" }
)

err(
    "float-bit-cast-requires-constant",
    30085,
    "bit cast requires constant",
    span { loc = "expr:Expr", message = "'__floatAsInt' requires a compile-time constant floating-point expression" }
)

-- writeonly diagnostic

err(
    "reading-from-write-only",
    30101,
    "cannot read from writeonly",
    span { loc = "expr:Expr", message = "cannot read from writeonly, check modifiers." }
)

-- differentiable member diagnostic

err(
    "differentiable-member-should-have-corresponding-field-in-diff-type",
    30102,
    "missing field in differential type",
    span { loc = "location", message = "differentiable member '~member:Name' should have a corresponding field in '~diffType:Type'. Use [DerivativeMember(...)] or mark as no_diff" }
)

-- type pack diagnostics

err(
    "expect-type-pack-after-each",
    30103,
    "expected type pack after 'each'",
    span { loc = "expr:Expr", message = "expected a type pack or a tuple after 'each'." }
)

err(
    "each-expr-must-be-inside-expand-expr",
    30104,
    "'each' must be inside 'expand'",
    span { loc = "expr:Expr", message = "'each' expression must be inside 'expand' expression." }
)

err(
    "expand-term-captures-no-type-packs",
    30105,
    "'expand' captures no type packs",
    span { loc = "expr:Expr", message = "'expand' term captures no type packs. At least one type pack must be referenced via an 'each' term inside an 'expand' term." }
)

err(
    "improper-use-of-type",
    30106,
    "type cannot be used in this context",
    span { loc = "expr:Expr", message = "type '~type:Type' cannot be used in this context." }
)

err(
    "parameter-pack-must-be-const",
    30107,
    "parameter pack must be 'const'",
    span { loc = "modifier:Modifier", message = "a parameter pack must be declared as 'const'." }
)

-- defer diagnostics

err(
    "break-inside-defer",
    30108,
    "'break' inside defer",
    span { loc = "stmt:Stmt", message = "'break' must not appear inside a defer statement." }
)

err(
    "continue-inside-defer",
    30109,
    "'continue' inside defer",
    span { loc = "stmt:Stmt", message = "'continue' must not appear inside a defer statement." }
)

err(
    "return-inside-defer",
    30110,
    "'return' inside defer",
    span { loc = "stmt:Stmt", message = "'return' must not appear inside a defer statement." }
)

-- lambda diagnostics

err(
    "return-type-mismatch-inside-lambda",
    30111,
    "lambda return type mismatch",
    span { loc = "stmt:Stmt", message = "returned values must have the same type among all 'return' statements inside a lambda expression: returned '~returnedType:Type' here, but '~previousType:Type' previously." }
)

err(
    "non-copyable-type-captured-in-lambda",
    30112,
    "non-copyable type captured",
    span { loc = "expr:Expr", message = "cannot capture non-copyable type '~type:Type' in a lambda expression." }
)

-- uncaught throw diagnostics

err(
    "uncaught-throw-inside-defer",
    30113,
    "'throw' requires 'catch' inside defer",
    span { loc = "stmt:Stmt", message = "'throw' expressions require a matching 'catch' inside a defer statement." }
)

err(
    "uncaught-try-inside-defer",
    30114,
    "'try' requires 'catch' inside defer",
    span { loc = "expr:Expr", message = "'try' expressions require a matching 'catch' inside a defer statement." }
)

err(
    "uncaught-throw-in-non-throw-func",
    30115,
    "uncaught 'throw' in non-throwing function",
    span { loc = "stmt:Stmt", message = "the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement." }
)

err(
    "throw-type-incompatible-with-error-type",
    30116,
    "throw type incompatible with error type",
    span { loc = "expr:Expr", message = "the type `~throwType:Type` of `throw` is not compatible with function's error type `~errorType:Type`." }
)

err(
    "forward-reference-in-generic-constraint",
    30117,
    "forward reference in generic constraint",
    span { loc = "expr:Expr", message = "generic constraint for parameter '~param:Type' references type parameter '~referenced:Decl' before it is declared" }
)

err(
    "cannot-mix-differentiable-value-and-ptr-outputs",
    30118,
    "cannot mix differentiable value types with differentiable pointer outputs",
    span { loc = "location", message = "function has both IDifferentiable value types and IDifferentiablePtrType outputs, which is not currently supported. Please split the function so that differentiable value parameters and pointer differentiable outputs are in separate functions." }
)


-- Load semantic checking diagnostics (part 3) - Include, Visibility, and Capability
-- (inlined from slang-diagnostics-semantic-checking-3.lua)

--
-- 305xx - Include
--

err(
    "included-file-missing-implementing",
    30500,
    "missing 'implementing' declaration",
    span { loc = "location", message = "missing 'implementing' declaration in the included source file '~fileName'." }
)

err(
    "included-file-missing-implementing-do-you-mean-import",
    30501,
    "missing 'implementing' declaration",
    span { loc = "location", message = "missing 'implementing' declaration in the included source file '~fileName'. The file declares that it defines module '~moduleName', do you mean 'import' instead?" }
)

err(
    "included-file-does-not-implement-current-module",
    30502,
    "module name mismatch in included file",
    span { loc = "location", message = "the included source file is expected to implement module '~expectedModule:Name', but it is implementing '~actualModule' instead." }
)

err(
    "primary-module-file-cannot-start-with-implementing-decl",
    30503,
    "primary module file cannot start with 'implementing'",
    span { loc = "decl:Decl", message = "a primary source file for a module cannot start with 'implementing'." }
)

warning(
    "primary-module-file-must-start-with-module-decl",
    30504,
    "primary module file should start with 'module'",
    span { loc = "decl:Decl", message = "a primary source file for a module should start with 'module'." }
)

err(
    "implementing-must-reference-primary-module-file",
    30505,
    "'implementing' must reference primary module file",
    span { loc = "location", message = "the source file referenced by 'implementing' must be a primary module file starting with a 'module' declaration." }
)

warning(
    "module-implementation-has-file-extension",
    30506,
    "file extension in module name",
    span { loc = "location", message = "implementing directive contains file extension in module name '~moduleName'. Module names should not include extensions. The compiler will use '~normalizedName' as the module name." }
)

--
-- 306xx - Visibility
--

err(
    "decl-is-not-visible",
    30600,
    "declaration not accessible",
    span { loc = "location", message = "'~decl:Decl' is not accessible from the current context." }
)

err(
    "decl-cannot-have-higher-visibility",
    30601,
    "visibility higher than parent",
    span { loc = "decl:Decl", message = "'~decl' cannot have a higher visibility than '~parent:Decl'." }
)

err(
    "invalid-use-of-private-visibility",
    30603,
    "invalid private visibility",
    span { loc = "location", message = "'~decl:Decl' cannot have private visibility because it is not a member of a type." }
)

err(
    "use-of-less-visible-type",
    30604,
    "references less visible type",
    span { loc = "decl:Decl", message = "'~decl' references less visible type '~type:Type'." }
)

err(
    "invalid-visibility-modifier-on-type-of-decl",
    36005,
    "visibility modifier not allowed",
    span { loc = "location", message = "visibility modifier is not allowed on '~astNodeType:ASTNodeType'." }
)

--
-- 361xx - Capability
--

err(
    "conflicting-capability-due-to-use-of-decl",
    36100,
    "conflicting capability requirement",
    span { loc = "location", message = "'~referencedDecl:Decl' requires capability '~requiredCaps' that is conflicting with the '~contextDecl:Decl's current capability requirement '~existingCaps'." }
)

err(
    "conflicting-capability-due-to-statement",
    36101,
    "conflicting capability requirement",
    span { loc = "location", message = "statement requires capability '~requiredCaps' that is conflicting with the '~context's current capability requirement '~existingCaps'." }
)

err(
    "conflicting-capability-due-to-statement-enclosing-func",
    36102,
    "conflicting capability requirement",
    span { loc = "location", message = "statement requires capability '~requiredCaps' that is conflicting with the current function's capability requirement '~existingCaps'." }
)

err(
    "use-of-undeclared-capability",
    36104,
    "undeclared capability used",
    span { loc = "decl:Decl", message = "'~decl' uses undeclared capability '~caps'" }
)

err(
    "use-of-undeclared-capability-of-interface-requirement",
    36104,
    "capability incompatible with interface",
    span { loc = "decl:Decl", message = "'~decl' uses capability '~caps' that is incompatable with the interface requirement" }
)

err(
    "use-of-undeclared-capability-of-inheritance-decl",
    36104,
    "capability incompatible with supertype",
    span { loc = "decl:Decl", message = "'~decl' uses capability '~caps' that is incompatable with the supertype" }
)

err(
    "unknown-capability",
    36105,
    "unknown capability",
    span { loc = "location", message = "unknown capability name '~capability'." }
)

err(
    "expect-capability",
    36106,
    "expect a capability name",
    span { loc = "location", message = "expect a capability name." }
)

err(
    "entry-point-uses-unavailable-capability",
    36107,
    "unavailable features in entry point",
    span { loc = "decl:Decl", message = "entrypoint '~decl' uses features that are not available in '~stage' stage for '~target' compilation target." }
)

err(
    "decl-has-dependencies-not-compatible-on-target",
    36108,
    "dependencies not compatible on target",
    span { loc = "decl:Decl", message = "'~decl' has dependencies that are not compatible on the required compilation target '~target'." }
)

err(
    "invalid-target-switch-case",
    36109,
    "invalid target_switch case",
    span { loc = "location", message = "'~capability' cannot be used as a target_switch case." }
)

err(
    "unexpected-capability",
    36111,
    "disallowed capability",
    span { loc = "location", message = "'~decl' resolves into a disallowed `~capability` Capability." }
)

warning(
    "entry-point-and-profile-are-incompatible",
    36112,
    "entry point incompatible with profile",
    span { loc = "location", message = "'~decl:Decl' is defined for stage '~stage', which is incompatible with the declared profile '~profile'." }
)

warning(
    "using-internal-capability-name",
    36113,
    "using internal capability name",
    span { loc = "location", message = "'~decl' resolves into a '_Internal' '_~capability' Capability, use '~capability' instead." }
)

err(
    "capability-has-multiple-stages",
    36116,
    "capability targets multiple stages",
    span { loc = "location", message = "Capability '~capability' is targeting stages '~stages:CapabilityAtomList', only allowed to use 1 unique stage here." }
)

err(
    "decl-has-dependencies-not-compatible-on-stage",
    36117,
    "dependencies not compatible on stage",
    span { loc = "decl:Decl", message = "'~decl' requires support for stage '~stage', but stage is unsupported." }
)

err(
    "sub-type-has-subset-of-abstract-atoms-to-super-type",
    36118,
    "subtype missing target/stage support",
    span { loc = "decl:Decl", message = "subtype '~decl' must have the same target/stage support as the supertype; '~decl' is missing '~missingCaps'" }
)

err(
    "requirment-has-subset-of-abstract-atoms-to-implementation",
    36119,
    "requirement missing target/stage support",
    span { loc = "decl:Decl", message = "requirement '~decl' must have the same target/stage support as the implementation; '~decl' is missing '~missingCaps'" }
)

err(
    "target-switch-cap-cases-conflict",
    36120,
    "capability cases conflict in target_switch",
    span { loc = "location", message = "the capability for case '~caseName' is '~caseCaps', which conflicts with previous case which requires '~prevCaps'. In target_switch, if two cases are belong to the same target, then one capability set has to be a subset of the other." }
)


-- Load semantic checking diagnostics (part 4) - Attributes
-- (inlined from slang-diagnostics-semantic-checking-4.lua)

--
-- 310xx - Attributes
--

warning(
    "unknown-attribute-name",
    31000,
    "unknown attribute",
    span { loc = "attr:Modifier", message = "unknown attribute '~attrName:Name'" }
)

err(
    "attribute-argument-count-mismatch",
    31001,
    "wrong number of attribute arguments",
    span { loc = "attr:Modifier", message = "attribute '~attrName:Name' expects ~expected arguments (~provided:Int provided)" }
)

err(
    "attribute-not-applicable",
    31002,
    "invalid attribute placement",
    span { loc = "attr:Modifier", message = "attribute '~attrName:Name' is not valid here" }
)

err(
    "badly-defined-patch-constant-func",
    31003,
    "invalid 'patchconstantfunc' attribute",
    span { loc = "location:Modifier", message = "hull shader '~entryPointName:Name' has badly defined 'patchconstantfunc' attribute." }
)

err(
    "expected-single-int-arg",
    31004,
    "expected single int argument",
    span { loc = "attr:Modifier", message = "attribute '~attrName:Name' expects a single int argument" }
)

err(
    "expected-single-string-arg",
    31005,
    "expected single string argument",
    span { loc = "attr:Modifier", message = "attribute '~attrName:Name' expects a single string argument" }
)

err(
    "attribute-function-not-found",
    31006,
    "function not found for attribute",
    span { loc = "location:Expr", message = "Could not find function '~funcName:Name' for attribute '~attrName'" }
)

err(
    "attribute-expected-int-arg",
    31007,
    "expected int argument",
    span { loc = "attr:Modifier", message = "attribute '~attrName:Name' expects argument ~argIndex:Int to be int" }
)

err(
    "attribute-expected-string-arg",
    31008,
    "expected string argument",
    span { loc = "attr:Modifier", message = "attribute '~attrName:Name' expects argument ~argIndex:Int to be string" }
)

err(
    "expected-single-float-arg",
    31009,
    "expected single float argument",
    span { loc = "attr:Modifier", message = "attribute '~attrName:Name' expects a single floating point argument" }
)

--
-- 311xx - Attributes (continued)
--

err(
    "unknown-stage-name",
    31100,
    "unknown stage name",
    span { loc = "location", message = "unknown stage name '~stageName'" }
)

err(
    "unknown-image-format-name",
    31101,
    "unknown image format",
    span { loc = "location:Expr", message = "unknown image format '~formatName'" }
)

err(
    "unknown-diagnostic-name",
    31101,
    "unknown diagnostic",
    span { loc = "location", message = "unknown diagnostic '~diagnosticName'" }
)

err(
    "non-positive-num-threads",
    31102,
    "invalid 'numthreads' value",
    span { loc = "attr:Modifier", message = "expected a positive integer in 'numthreads' attribute, got '~value:Int'" }
)

err(
    "invalid-wave-size",
    31103,
    "invalid 'WaveSize' value",
    span { loc = "attr:Modifier", message = "expected a power of 2 between 4 and 128, inclusive, in 'WaveSize' attribute, got '~value:Int'" }
)

warning(
    "explicit-uniform-location",
    31104,
    "explicit binding of uniform discouraged",
    span { loc = "var:Decl", message = "Explicit binding of uniform locations is discouraged. Prefer 'ConstantBuffer<~type:Type>' over 'uniform ~type:Type'" }
)

warning(
    "image-format-unsupported-by-backend",
    31105,
    "Image format '~format' is not explicitly supported by the ~backend backend, using supported format '~replacement' instead.",
    span { loc = "location" }  -- No span message: this diagnostic has no meaningful source location
)

err(
    "invalid-attribute-target",
    31120,
    "invalid syntax target for user defined attribute",
    span { loc = "attr:Modifier", message = "invalid syntax target for user defined attribute" }
)

err(
    "attribute-usage-attribute-must-be-on-non-generic-struct",
    31125,
    "[__AttributeUsage] requires non-generic struct",
    span { loc = "attr:Modifier", message = "[__AttributeUsage] can only be applied to non-generic struct definitions" }
)

err(
    "any-value-size-exceeds-limit",
    31121,
    "'anyValueSize' exceeds limit",
    span { loc = "location", message = "'anyValueSize' cannot exceed ~maxSize:Int" }
)

err(
    "associated-type-not-allowed-in-com-interface",
    31122,
    "associatedtype not allowed in [COM] interface",
    span { loc = "decl:Decl", message = "associatedtype not allowed in a [COM] interface" }
)

err(
    "invalid-guid",
    31123,
    "invalid GUID",
    span { loc = "attr:Modifier", message = "'~guid' is not a valid GUID" }
)

-- Note: 31124 (structCannotImplementComInterface, interfaceInheritingComMustBeCom)
-- moved to slang-diagnostics-semantic-checking-5.lua


-- Load semantic checking diagnostics (part 5) - COM Interface, DerivativeMember, Extern Decl, Custom Derivative
-- (inlined from slang-diagnostics-semantic-checking-5.lua)

--
-- 311xx - COM Interface
--

err(
    "struct-cannot-implement-com-interface",
    31124,
    "struct types cannot implement COM interfaces",
    span { loc = "decl:Decl", message = "a struct type cannot implement a [COM] interface" }
)

err(
    "interface-inheriting-com-must-be-com",
    31124,
    "non-COM interface inheriting from COM interface",
    span { loc = "decl:Decl", message = "an interface type that inherits from a [COM] interface must itself be a [COM] interface" }
)

--
-- 3113x - DerivativeMember Attribute
--

err(
    "derivative-member-attribute-must-name-a-member-in-expected-differential-type",
    31130,
    "invalid DerivativeMember target",
    span { loc = "attr", message = "[DerivativeMember] must reference to a member in the associated differential type '~diffType:Type'." }
)

err(
    "invalid-use-of-derivative-member-attribute-parent-type-is-not-differentiable",
    31131,
    "DerivativeMember on non-differentiable type",
    span { loc = "attr", message = "invalid use of [DerivativeMember], parent type is not differentiable." }
)

err(
    "derivative-member-attribute-can-only-be-used-on-members",
    31132,
    "DerivativeMember on non-member",
    span { loc = "attr", message = "[DerivativeMember] is allowed on members only." }
)

--
-- 3114x - Extern Decl
--

err(
    "type-of-extern-decl-mismatches-original-definition",
    31140,
    "extern decl type mismatch",
    span { loc = "decl:Decl", message = "type of `extern` decl '~decl' differs from its original definition. expected '~expectedType:Type'." }
)

err(
    "definition-of-extern-decl-mismatches-original-definition",
    31141,
    "extern decl definition mismatch",
    span { loc = "decl:Decl", message = "`extern` decl '~decl' is not consistent with its original definition." }
)

err(
    "ambiguous-original-defintion-of-extern-decl",
    31142,
    "ambiguous extern decl target",
    span { loc = "decl:Decl", message = "`extern` decl '~decl' has ambiguous original definitions." }
)

err(
    "missing-original-defintion-of-extern-decl",
    31143,
    "no original definition for extern decl",
    span { loc = "decl:Decl", message = "no original definition found for `extern` decl '~decl'." }
)

--
-- 3114x - Attribute
--

err(
    "decl-already-has-attribute",
    31146,
    "duplicate attribute",
    span { loc = "attr", message = "'~decl:Decl' already has attribute '[~attrName]'." }
)

--
-- 3114x-3116x - Custom Derivative
--

err(
    "cannot-resolve-original-function-for-derivative",
    31147,
    "cannot resolve original function for derivative",
    span { loc = "attr", message = "cannot resolve the original function for the the custom derivative." }
)

err(
    "cannot-resolve-derivative-function",
    31148,
    "cannot resolve derivative function",
    span { loc = "attr", message = "cannot resolve the custom derivative function" }
)

err(
    "custom-derivative-signature-mismatch-at-position",
    31149,
    "custom derivative parameter type mismatch",
    span { loc = "attr", message = "invalid custom derivative. parameter type mismatch at position ~position:int. expected '~expectedType', got '~actualType'" }
)

err(
    "custom-derivative-signature-mismatch",
    31150,
    "custom derivative signature mismatch",
    span { loc = "attr", message = "invalid custom derivative. could not resolve function with expected signature '~expectedSignature'" }
)

err(
    "cannot-resolve-generic-argument-for-derivative-function",
    31151,
    "cannot deduce generic arguments for derivative",
    span { loc = "attr", message = "The generic arguments to the derivative function cannot be deduced from the parameter list of the original function. Consider using [ForwardDerivative], [BackwardDerivative] or [PrimalSubstitute] attributes on the primal function with explicit generic arguments to associate it with a generic derivative function. Note that [ForwardDerivativeOf], [BackwardDerivativeOf], and [PrimalSubstituteOf] attributes are not supported when the generic arguments to the derivatives cannot be automatically deduced." }
)

err(
    "cannot-associate-interface-requirement-with-derivative",
    31152,
    "interface requirement cannot have derivative",
    span { loc = "attr", message = "cannot associate an interface requirement with a derivative." }
)

err(
    "cannot-use-interface-requirement-as-derivative",
    31153,
    "interface requirement cannot be used as derivative",
    span { loc = "attr", message = "cannot use an interface requirement as a derivative." }
)

err(
    "custom-derivative-signature-this-param-mismatch",
    31154,
    "custom derivative 'this' type mismatch",
    span { loc = "attr", message = "custom derivative does not match expected signature on `this`. Both original and derivative function must have the same `this` type." }
)

err(
    "custom-derivative-expected-static",
    31156,
    "expected static custom derivative",
    span { loc = "attr", message = "expected a static definition for the custom derivative." }
)

err(
    "overloaded-func-used-with-derivative-of-attributes",
    31157,
    "overloaded function in derivative-of attribute",
    span { loc = "attr", message = "cannot resolve overloaded functions for derivative-of attributes." }
)


-- Load semantic checking diagnostics (part 6) - Differentiation, Modifiers, GLSL/HLSL specifics, Interfaces, Control flow, Enums, Generics
-- (inlined from slang-diagnostics-semantic-checking-6.lua)

-- 3115x-3116x - Differentiation continued

err(
    "primal-substitute-target-must-have-higher-differentiability-level",
    31158,
    "primal substitute requires differentiable target",
    span { loc = "attr:Modifier", message = "primal substitute function for differentiable method must also be differentiable. Use [Differentiable] or [TreatAsDifferentiable] (for empty derivatives)" }
)

warning(
    "no-derivative-on-non-differentiable-this-type",
    31159,
    "no derivative for member on non-differentiable struct",
    span { loc = "member:Expr", message = "There is no derivative calculated for member '~memberDecl:Decl' because the parent struct is not differentiable. If this is intended, consider using [NoDiffThis] on the function '~func:Decl' to suppress this warning. Alternatively, users can mark the parent struct as [Differentiable] to propagate derivatives." }
)

err(
    "invalid-address-of",
    31160,
    "invalid __getAddress usage",
    span { loc = "expr:Expr", message = "'__getAddress' only supports groupshared variables and members of groupshared/device memory." }
)

-- 312xx - Modifiers and Deprecation

warning(
    "deprecated-usage",
    31200,
    "use of deprecated declaration",
    span { loc = "location", message = "~declName:Name has been deprecated: ~message" }
)

err(
    "modifier-not-allowed",
    31201,
    "modifier not allowed",
    span { loc = "modifier:Modifier", message = "modifier '~modifier' is not allowed here." }
)

err(
    "duplicate-modifier",
    31202,
    "duplicate modifier",
    span { loc = "modifier:Modifier", message = "modifier '~modifier' is redundant or conflicting with existing modifier '~existingModifier:Modifier'" }
)

err(
    "cannot-export-incomplete-type",
    31203,
    "cannot export incomplete type",
    span { loc = "decl:Decl", message = "cannot export incomplete type '~decl'" }
)

warning(
    "deprecated-bracket-attributes-placement",
    31204,
    "deprecated bracketed attribute list placement. Bracketed attributes should be placed before 'struct'.",
    span { loc = "location", message = "deprecated placement of bracketed attributes list." }
)

err(
    "invalid-bracket-attributes-placement",
    31205,
    "invalid bracketed attribute list placement. Bracketed attributes must be placed before 'struct'.",
    span { loc = "location", message = "invalid placement of bracketed attributes list." }
)

err(
    "memory-qualifier-not-allowed-on-a-non-image-type-parameter",
    31206,
    "invalid memory qualifier",
    span { loc = "param:Decl", message = "modifier ~modifier:Modifier is not allowed on a non image type parameter." }
)

err(
    "require-input-decorated-var-for-parameter",
    31208,
    "shader input required",
    span { loc = "expr:Expr", message = "~func:Decl expects for argument ~paramNumber:Int a type which is a shader input (`in`) variable." }
)

-- 3121x - Derivative group requirements

err(
    "derivative-group-quad-must-be-multiple-2-for-xy-threads",
    31210,
    "derivative group quad thread count error",
    span { loc = "location", message = "compute derivative group quad requires thread dispatch count of X and Y to each be at a multiple of 2" }
)

err(
    "derivative-group-linear-must-be-multiple-4-for-total-thread-count",
    31211,
    "derivative group linear thread count error",
    span { loc = "location", message = "compute derivative group linear requires total thread dispatch count to be at a multiple of 4" }
)

err(
    "only-one-of-derivative-group-linear-or-quad-can-be-set",
    31212,
    "conflicting derivative group settings",
    span { loc = "location", message = "cannot set compute derivative group linear and compute derivative group quad at the same time" }
)

-- CUDA

err(
    "cuda-kernel-must-return-void",
    31213,
    "CUDA kernel return type error",
    span { loc = "decl:Decl", message = "return type of a CUDA kernel function cannot be non-void." }
)

err(
    "differentiable-kernel-entry-point-cannot-have-differentiable-params",
    31214,
    "differentiable kernel param restriction",
    span { loc = "param:Decl", message = "differentiable kernel entry point cannot have differentiable parameters. Consider using DiffTensorView to pass differentiable data, or marking this parameter with 'no_diff'" }
)

err(
    "cannot-use-unsized-type-in-constant-buffer",
    31215,
    "unsized type in constant buffer",
    span { loc = "field:Decl", message = "cannot use unsized type '~type:Type' in a constant buffer." }
)

-- GLSL layout qualifiers

err(
    "unrecognized-glsl-layout-qualifier",
    31216,
    "unrecognized GLSL layout qualifier",
    span { loc = "attr:Modifier", message = "GLSL layout qualifier is unrecognized" }
)

err(
    "unrecognized-glsl-layout-qualifier-or-requires-assignment",
    31217,
    "unrecognized GLSL layout qualifier",
    span { loc = "location", message = "GLSL layout qualifier is unrecognized or requires assignment" }
)

-- Specialization and push constants

err(
    "specialization-constant-must-be-scalar",
    31218,
    "specialization constant type error",
    span { loc = "modifier:Modifier", message = "specialization constant must be a scalar." }
)

err(
    "push-or-specialization-constant-cannot-be-static",
    31219,
    "push/specialization constant storage class error",
    span { loc = "decl:Decl", message = "push or specialization constants cannot be 'static'." }
)

err(
    "variable-cannot-be-push-and-specialization-constant",
    31220,
    "conflicting constant qualifiers",
    span { loc = "decl:Decl", message = "'~decl' cannot be a push constant and a specialization constant at the same time" }
)

-- HLSL register names

err(
    "invalid-hlsl-register-name",
    31221,
    "invalid HLSL register name",
    span { loc = "location", message = "invalid HLSL register name '~registerName'." }
)

err(
    "invalid-hlsl-register-name-for-type",
    31222,
    "invalid HLSL register name for type",
    span { loc = "location", message = "invalid HLSL register name '~registerName' for type '~type:Type'." }
)

-- Extern/export and const variables

err(
    "extern-and-export-var-decl-must-be-const",
    31223,
    "extern/export requires static const",
    span { loc = "decl:Decl", message = "extern and export variables must be static const: '~decl'" }
)

err(
    "const-global-var-with-init-requires-static",
    31224,
    "global const requires static",
    span { loc = "decl:Decl", message = "global const variable with initializer must be declared static: '~decl'" }
)

err(
    "static-const-variable-requires-initializer",
    31225,
    "missing initializer for static const",
    span { loc = "decl:Decl", message = "static const variable '~decl' must have an initializer" }
)

-- Enums (320xx)

err(
    "invalid-enum-tag-type",
    32000,
    "invalid enum tag type",
    span { loc = "location", message = "invalid tag type for 'enum': '~type:Type'" }
)

err(
    "unexpected-enum-tag-expr",
    32003,
    "unexpected enum tag expression",
    span { loc = "expr:Expr", message = "unexpected form for 'enum' tag value expression" }
)

-- 303xx: interfaces and associated types

err(
    "assoc-type-in-interface-only",
    30300,
    "associatedtype outside interface",
    span { loc = "decl:Decl", message = "'associatedtype' can only be defined in an 'interface'." }
)

err(
    "global-gen-param-in-global-scope-only",
    30301,
    "type_param outside global scope",
    span { loc = "decl:Decl", message = "'type_param' can only be defined global scope." }
)

err(
    "static-const-requirement-must-be-int-or-bool",
    30302,
    "invalid static const requirement type",
    span { loc = "decl:Decl", message = "'static const' requirement can only have int or bool type." }
)

err(
    "value-requirement-must-be-compile-time-const",
    30303,
    "value requirement requires static const",
    span { loc = "decl:Decl", message = "requirement in the form of a simple value must be declared as 'static const'." }
)

err(
    "type-is-not-differentiable",
    30310,
    "type is not differentiable",
    span { loc = "attr:Modifier", message = "type '~type:Type' is not differentiable." }
)

err(
    "non-method-interface-requirement-cannot-have-body",
    30311,
    "interface requirement has body",
    span { loc = "decl:Decl", message = "non-method interface requirement cannot have a body." }
)

err(
    "interface-requirement-cannot-be-override",
    30312,
    "interface requirement cannot override",
    span { loc = "decl:Decl", message = "interface requirement cannot override a base declaration." }
)

-- Interop (304xx)

err(
    "cannot-define-ptr-type-to-managed-resource",
    30400,
    "pointer to managed resource invalid",
    span { loc = "typeExp:Expr", message = "pointer to a managed resource is invalid, use `NativeRef<T>` instead" }
)

-- Control flow (305xx)

warning(
    "for-loop-side-effect-changing-different-var",
    30500,
    "for loop modifies wrong variable",
    span { loc = "sideEffect:Expr", message = "the for loop initializes and checks variable '~initVar:Decl' but the side effect expression is modifying '~modifiedVar:Decl'." }
)

warning(
    "for-loop-predicate-checking-different-var",
    30501,
    "for loop predicate checks wrong variable",
    span { loc = "predicate:Expr", message = "the for loop initializes and modifies variable '~initVar:Decl' but the predicate expression is checking '~predicateVar:Decl'." }
)

warning(
    "for-loop-changing-iteration-variable-in-oppsoite-direction",
    30502,
    "for loop modifies variable in wrong direction",
    span { loc = "sideEffect:Expr", message = "the for loop is modifiying variable '~var:Decl' in the opposite direction from loop exit condition." }
)

warning(
    "for-loop-not-modifying-iteration-variable",
    30503,
    "for loop step is zero",
    span { loc = "sideEffect:Expr", message = "the for loop is not modifiying variable '~var:Decl' because the step size evaluates to 0." }
)

warning(
    "for-loop-terminates-in-fewer-iterations-than-max-iters",
    30504,
    "MaxIters exceeds actual iterations",
    span { loc = "attr:Modifier", message = "the for loop is statically determined to terminate within ~iterations:Int iterations, which is less than what [MaxIters] specifies." }
)

warning(
    "loop-runs-for-zero-iterations",
    30505,
    "loop runs zero times",
    span { loc = "stmt:Stmt", message = "the loop runs for 0 iterations and will be removed." }
)

err(
    "loop-in-diff-func-require-unroll-or-max-iters",
    30510,
    "loop in differentiable function needs attributes",
    span { loc = "location", message = "loops inside a differentiable function need to provide either '[MaxIters(n)]' or '[ForceUnroll]' attribute." }
)

-- Switch (306xx)

err(
    "switch-multiple-default",
    30600,
    "multiple default cases in switch",
    span { loc = "stmt:Stmt", message = "multiple 'default' cases not allowed within a 'switch' statement" }
)

err(
    "switch-duplicate-cases",
    30601,
    "duplicate cases in switch",
    span { loc = "stmt:Stmt", message = "duplicate cases not allowed within a 'switch' statement" }
)

-- 310xx: link time specialization
-- (definitions moved to slang-diagnostics-semantic-checking-7.lua)

-- Cyclic references and misc errors (39xxx)
-- (definitions moved to slang-diagnostics-semantic-checking-7.lua)

-- 304xx: generics
-- (definitions moved to slang-diagnostics-semantic-checking-7.lua)


-- Load semantic checking diagnostics (part 7) - Link Time, Cyclic Refs, Generics, Initializers, Variables, Parameters, Inheritance, Extensions, Subscripts
-- (inlined from slang-diagnostics-semantic-checking-7.lua)

--
-- 310xx: link time specialization
--
warning(
    "link-time-constant-array-size",
    31000,
    "Link-time constant sized arrays are a work in progress feature, some aspects of the reflection API may not work",
    span { loc = "decl:Decl" }
)

--
-- 39xxx: cyclic references
--
-- TODO: need to assign numbers to all these extra diagnostics...
fatal(
    "cyclic-reference",
    39999,
    "cyclic reference '~decl'",
    span { loc = "decl:Decl", message = "cyclic reference '~decl'" }
)

err(
    "cyclic-reference-in-inheritance",
    39999,
    "cyclic reference in inheritance graph '~decl'",
    span { loc = "decl:Decl", message = "cyclic reference in inheritance graph '~decl'" }
)

err(
    "variable-used-in-its-own-definition",
    39999,
    "the initial-value expression for variable '~decl' depends on the value of the variable itself",
    span { loc = "decl:Decl", message = "the initial-value expression for variable '~decl' depends on the value of the variable itself" }
)

err(
    "generic-evaluation-recursion-limit-exceeded",
    39998,
    "recursive generic evaluation exceeded maximum depth",
    span { loc = "decl:Decl", message = "evaluation of '~decl' exceeded the recursive generic evaluation budget (~budget:int)" }
)

fatal(
    "maximum-type-nesting-level-exceeded",
    39997,
    "maximum type nesting level exceeded",
    span { loc = "location", message = "maximum type nesting level exceeded" }
)

fatal(
    "cannot-process-include",
    39901,
    "internal compiler error: cannot process '__include' in the current semantic checking context",
    span { loc = "location", message = "internal compiler error: cannot process '__include' in the current semantic checking context" }
)

--
-- 304xx: generics
--
err(
    "generic-type-needs-args",
    30400,
    "generic type '~type:Type' used without argument",
    span { loc = "typeExp:Expr", message = "generic type '~type' used without argument" }
)

err(
    "invalid-type-for-constraint",
    30401,
    "type '~type:Type' cannot be used as a constraint",
    span { loc = "typeExp:Expr", message = "type '~type' cannot be used as a constraint" }
)

standalone_note(
    "use-let-for-generic-value-param",
    30499,
    "use 'let' keyword to declare a generic value parameter: 'let ~paramName:Name : ~type:Type'"
)

standalone_note(
    "use-let-each-for-generic-value-pack-param",
    30501,
    "use 'let each' to declare a variadic generic value parameter: 'let each ~paramName:Name : ~type:Type'"
)

err(
    "pack-param-must-be-last",
    30500,
    "generic parameter after a variadic pack parameter is not allowed",
    span { loc = "param:Decl", message = "generic parameter '~param' cannot appear after a variadic pack parameter" }
)

err(
    "invalid-constraint-sub-type",
    30402,
    "type '~type:Type' is not a valid left hand side of a type constraint",
    span { loc = "typeExp:Expr", message = "type '~type' is not a valid left hand side of a type constraint" }
)

err(
    "required-constraint-is-not-checked",
    30403,
    "the constraint providing '~decl:Decl' is optional and must be checked with an 'is' statement before usage",
    span { loc = "location", message = "the constraint providing '~decl' is optional and must be checked with an 'is' statement before usage" }
)

err(
    "invalid-equality-constraint-sup-type",
    30404,
    "type '~type:Type' is not a proper type to use in a generic equality constraint",
    span { loc = "typeExp:Expr", message = "type '~type' is not a proper type to use in a generic equality constraint" }
)

err(
    "no-valid-equality-constraint-sub-type",
    30405,
    "generic equality constraint requires at least one operand to be dependant on the generic declaration",
    span { loc = "decl:Decl", message = "generic equality constraint requires at least one operand to be dependant on the generic declaration" }
)

standalone_note(
    "invalid-equality-constraint-sub-type",
    30402,
    "type '~type:Type' cannot be constrained by a type equality",
    span { loc = "typeExp:Expr", message = "type '~type' cannot be constrained by a type equality" }
)

warning(
    "failed-equality-constraint-canonical-order",
    30407,
    "failed to resolve canonical order of generic equality constraint",
    span { loc = "decl:Decl", message = "failed to resolve canonical order of generic equality constraint" }
)

--
-- 305xx: initializer lists
--

err(
    "too-many-initializers",
    30500,
    "too many initializers in initializer list",
    span { loc = "initList:Expr", message = "too many initializers (expected ~expected:int, got ~got:int)" }
)

err(
    "cannot-use-initializer-list-for-vector-of-unknown-size",
    30502,
    "cannot use initializer list for vector of statically unknown size '~elementCount:Val'",
    span { loc = "initList:Expr", message = "cannot use initializer list for vector of statically unknown size '~elementCount'" }
)

err(
    "cannot-use-initializer-list-for-matrix-of-unknown-size",
    30503,
    "cannot use initializer list for matrix of statically unknown size '~rowCount:Val' rows",
    span { loc = "initList:Expr", message = "cannot use initializer list for matrix of statically unknown size '~rowCount' rows" }
)

err(
    "cannot-use-initializer-list-for-type",
    30504,
    "cannot use initializer list for type '~type:Type'",
    span { loc = "initList:Expr", message = "cannot use initializer list for type '~type'" }
)

err(
    "cannot-use-initializer-list-for-coop-vector-of-unknown-size",
    30505,
    "cannot use initializer list for CoopVector of statically unknown size '~elementCount:Val'",
    span { loc = "initList:Expr", message = "cannot use initializer list for CoopVector of statically unknown size '~elementCount'" }
)

warning(
    "interface-default-initializer",
    30506,
    "initializing an interface variable with defaults is deprecated and may cause unexpected behavior",
    span { loc = "expr:Expr", message = "initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized" }
)

--
-- 3062x: variables
--
err(
    "var-without-type-must-have-initializer",
    30620,
    "a variable declaration without an initial-value expression must be given an explicit type",
    span { loc = "decl:Decl", message = "a variable declaration without an initial-value expression must be given an explicit type" }
)

err(
    "param-without-type-must-have-initializer",
    30621,
    "a parameter declaration without an initial-value expression must be given an explicit type",
    span { loc = "decl:Decl", message = "a parameter declaration without an initial-value expression must be given an explicit type" }
)

err(
    "ambiguous-default-initializer-for-type",
    30622,
    "more than one default initializer was found for type '~type:Type'",
    span { loc = "decl:Decl", message = "more than one default initializer was found for type '~type'" }
)

err(
    "cannot-have-initializer",
    30623,
    "'~decl:Decl' cannot have an initializer because it is ~reason",
    span { loc = "decl:Decl", message = "'~decl' cannot have an initializer because it is ~reason" }
)

err(
    "generic-value-parameter-must-have-type",
    30623,
    "a generic value parameter must be given an explicit type",
    span { loc = "decl:Decl", message = "a generic value parameter must be given an explicit type" }
)

err(
    "generic-value-parameter-type-not-supported",
    30624,
    "generic value parameter type '~type:Type' is not supported; only integer and enum types are allowed",
    span { loc = "decl:Decl", message = "generic value parameter type '~type' is not supported; only integer and enum types are allowed" }
)

--
-- 307xx: parameters
--
err(
    "output-parameter-cannot-have-default-value",
    30700,
    "an 'out' or 'inout' parameter cannot have a default-value expression",
    span { loc = "initExpr:Expr", message = "an 'out' or 'inout' parameter cannot have a default-value expression" }
)

err(
    "system-value-semantic-invalid-type",
    30701,
    "type '~type:Type' is not valid for system value semantic '~semantic'; expected '~expectedTypes'",
    span { loc = "location" }
)

err(
    "system-value-semantic-invalid-direction",
    30702,
    "system value semantic '~semantic' cannot be used as ~direction in '~stage' shader stage",
    span { loc = "location" }
)

--
-- 308xx: inheritance
--
err(
    "base-of-interface-must-be-interface",
    30810,
    "interface '~decl:Decl' cannot inherit from non-interface type '~baseType:Type'",
    span { loc = "inheritanceDecl:Decl", message = "interface '~decl' cannot inherit from non-interface type '~baseType'" }
)

err(
    "base-of-struct-must-be-interface",
    30811,
    "struct '~decl:Decl' cannot inherit from non-interface type '~baseType:Type'",
    span { loc = "inheritanceDecl:Decl", message = "struct '~decl' cannot inherit from non-interface type '~baseType'" }
)

err(
    "base-of-enum-must-be-integer-or-interface",
    30812,
    "enum '~decl:Decl' cannot inherit from type '~baseType:Type' that is neither an interface not a builtin integer type",
    span { loc = "inheritanceDecl:Decl", message = "enum '~decl' cannot inherit from type '~baseType' that is neither an interface not a builtin integer type" }
)

err(
    "base-of-extension-must-be-interface",
    30813,
    "extension cannot inherit from non-interface type '~baseType:Type'",
    span { loc = "inheritanceDecl:Decl", message = "extension cannot inherit from non-interface type '~baseType'" }
)

err(
    "base-of-class-must-be-class-or-interface",
    30814,
    "class '~decl:Decl' cannot inherit from type '~baseType:Type' that is neither a class nor an interface",
    span { loc = "inheritanceDecl:Decl", message = "class '~decl' cannot inherit from type '~baseType' that is neither a class nor an interface" }
)

err(
    "circularity-in-extension",
    30815,
    "circular extension is not allowed",
    span { loc = "decl:Decl", message = "circular extension is not allowed" }
)

warning(
    "inheritance-unstable",
    30816,
    "support for inheritance is unstable and will be removed in future language versions, consider using composition instead",
    span { loc = "inheritanceDecl:Decl", message = "support for inheritance is unstable and will be removed in future language versions, consider using composition instead" }
)

err(
    "base-struct-must-be-listed-first",
    30820,
    "a struct type may only inherit from one other struct type, and that type must appear first in the list of bases",
    span { loc = "inheritanceDecl:Decl", message = "a struct type may only inherit from one other struct type, and that type must appear first in the list of bases" }
)

err(
    "tag-type-must-be-listed-first",
    30821,
    "an enum type may only have a single tag type, and that type must be listed first in the list of bases",
    span { loc = "inheritanceDecl:Decl", message = "an enum type may only have a single tag type, and that type must be listed first in the list of bases" }
)

err(
    "base-class-must-be-listed-first",
    30822,
    "a class type may only inherit from one other class type, and that type must appear first in the list of bases",
    span { loc = "inheritanceDecl:Decl", message = "a class type may only inherit from one other class type, and that type must appear first in the list of bases" }
)

err(
    "cannot-inherit-from-explicitly-sealed-declaration-in-another-module",
    30830,
    "cannot inherit from type '~baseType:Type' marked 'sealed' in module '~moduleName:Name'",
    span { loc = "inheritanceDecl:Decl", message = "cannot inherit from type '~baseType' marked 'sealed' in module '~moduleName'" }
)

err(
    "cannot-inherit-from-implicitly-sealed-declaration-in-another-module",
    30831,
    "cannot inherit from type '~baseType:Type' in module '~moduleName:Name' because it is implicitly 'sealed'",
    span { loc = "inheritanceDecl:Decl", message = "cannot inherit from type '~baseType' in module '~moduleName' because it is implicitly 'sealed'; mark the base type 'open' to allow inheritance across modules" }
)

err(
    "invalid-type-for-inheritance",
    30832,
    "type '~type:Type' cannot be used for inheritance",
    span { loc = "inheritanceDecl:Decl", message = "type '~type' cannot be used for inheritance" }
)

--
-- 308xx: extensions
--
err(
    "invalid-extension-on-type",
    30850,
    "type '~type:Type' cannot be extended",
    span { loc = "typeExp:Expr", message = "type '~type' cannot be extended. `extension` can only be used to extend a nominal type" }
)

err(
    "invalid-member-type-in-extension",
    30851,
    "~nodeType cannot be a part of an `extension`",
    span { loc = "decl:Decl", message = "~nodeType cannot be a part of an `extension`" }
)

err(
    "invalid-extension-on-interface",
    30852,
    "cannot extend interface type '~type:Type'",
    span { loc = "typeExp:Expr", message = "cannot extend interface type '~type'. consider using a generic extension: `extension<T:~type> T {...}`" }
)

err(
    "missing-override",
    30853,
    "missing 'override' keyword for methods that overrides the default implementation in the interface",
    span { loc = "decl:Decl", message = "missing 'override' keyword for methods that overrides the default implementation in the interface" }
)

err(
    "override-modifier-not-overriding-base-decl",
    30854,
    "'~decl:Decl' marked as 'override' is not overriding any base declarations",
    span { loc = "decl:Decl", message = "'~decl' marked as 'override' is not overriding any base declarations" }
)

err(
    "unreferenced-generic-param-in-extension",
    30855,
    "generic parameter '~paramName:Name' is not referenced by extension target type '~targetType:Type'",
    span { loc = "decl:Decl", message = "generic parameter '~paramName' is not referenced by extension target type '~targetType'" }
)

warning(
    "generic-param-in-extension-not-referenced-by-target-type",
    30856,
    "the extension is non-standard and may not work as intended because the generic parameter '~paramName:Name' is not referenced by extension target type '~targetType:Type'",
    span { loc = "decl:Decl", message = "the extension is non-standard and may not work as intended because the generic parameter '~paramName' is not referenced by extension target type '~targetType'" }
)

--
-- 309xx: subscripts
--
err(
    "multi-dimensional-array-not-supported",
    30900,
    "multi-dimensional array is not supported",
    span { loc = "expr:Expr", message = "multi-dimensional array is not supported" }
)

err(
    "subscript-must-have-return-type",
    30901,
    "__subscript declaration must have a return type specified after '->'",
    span { loc = "location", message = "__subscript declaration must have a return type specified after '->'" }
)


-- Load semantic checking diagnostics (part 8) - Accessors, Bit Fields, Integer Constants, Overloads, Switch, Generics, Ambiguity
-- (inlined from slang-diagnostics-semantic-checking-8.lua)

--
-- 311xx: accessors
--
err(
    "accessor-must-be-inside-subscript-or-property",
    31100,
    "invalid accessor declaration location",
    span { loc = "decl:Decl", message = "an accessor declaration is only allowed inside a subscript or property declaration" }
)

err(
    "non-set-accessor-must-not-have-params",
    31101,
    "accessors other than 'set' must not have parameters",
    span { loc = "decl:Decl", message = "accessors other than 'set' must not have parameters" }
)

err(
    "set-accessor-may-not-have-more-than-one-param",
    31102,
    "a 'set' accessor may not have more than one parameter",
    span { loc = "param:Decl", message = "a 'set' accessor may not have more than one parameter" }
)

err(
    "set-accessor-param-wrong-type",
    31102,
    "'set' parameter type mismatch",
    span { loc = "param:Decl", message = "'set' parameter '~param' has type '~actualType:Type' which does not match the expected type '~expectedType:Type'" }
)

--
-- 313xx: bit fields
--
err(
    "bit-field-too-wide",
    31300,
    "bit-field size exceeds type width",
    span { loc = "location", message = "bit-field size (~fieldWidth:Int) exceeds the width of its type ~type:Type (~typeWidth:Int)" }
)

err(
    "bit-field-non-integral",
    31301,
    "bit-field type must be integral",
    span { loc = "location", message = "bit-field type (~type:Type) must be an integral type" }
)

--
-- 39999: waiting to be placed in the right range
--
err(
    "expected-integer-constant-not-constant",
    39999,
    "expression does not evaluate to a compile-time constant",
    span { loc = "location", message = "expression does not evaluate to a compile-time constant" }
)

err(
    "expected-integer-constant-not-literal",
    39999,
    "could not extract value from integer constant",
    span { loc = "location", message = "could not extract value from integer constant" }
)

err(
    "expected-ray-tracing-payload-object-at-location-but-missing",
    39999,
    "raytracing payload missing",
    span { loc = "location", message = "raytracing payload expected at location ~payloadLocation:Int but it is missing" }
)

err(
    "no-applicable-overload-for-name-with-args",
    39999,
    "no overload for '~name:Name' applicable to arguments of type ~args",
    span { loc = "expr:Expr", message = "no overload for '~name' applicable to arguments of type ~args" }
)

err(
    "no-applicable-with-args",
    39999,
    "no overload applicable to arguments of type ~args",
    span { loc = "expr:Expr", message = "no overload applicable to arguments of type ~args" }
)

err(
    "ambiguous-overload-for-name-with-args",
    39999,
    "ambiguous call to '~name' with arguments of type ~args",
    span { loc = "expr:Expr", message = "ambiguous call to '~name' with arguments of type ~args" },
    variadic_note { cpp_name = "Candidate", message = "candidate: ~candidateSignature", span { loc = "candidate:Decl" } }
)

err(
    "ambiguous-overload-with-args",
    39999,
    "ambiguous call to overloaded operation with arguments of type ~args",
    span { loc = "expr:Expr", message = "ambiguous call to overloaded operation with arguments of type ~args" }
)

standalone_note(
    "overload-candidate",
    39999,
    "candidate: ~candidate",
    span { loc = "location" }
)

standalone_note(
    "invisible-overload-candidate",
    39999,
    "candidate (invisible): ~candidate",
    span { loc = "location" }
)

standalone_note(
    "more-overload-candidates",
    39999,
    "~count:Int more overload candidates",
    span { loc = "location" }
)

err(
    "case-outside-switch",
    39999,
    "'case' not allowed outside of a 'switch' statement",
    span { loc = "stmt:Stmt", message = "'case' not allowed outside of a 'switch' statement" }
)

err(
    "default-outside-switch",
    39999,
    "'default' not allowed outside of a 'switch' statement",
    span { loc = "stmt:Stmt", message = "'default' not allowed outside of a 'switch' statement" }
)

err(
    "expected-a-generic",
    39999,
    "expected a generic when using '<...>'",
    span { loc = "expr:Expr", message = "expected a generic when using '<...>' (found: '~found:Type')" }
)

err(
    "generic-argument-inference-failed",
    39999,
    "could not specialize generic for arguments of type ~args",
    span { loc = "location", message = "could not specialize generic for arguments of type ~args" }
)

err(
    "ambiguous-reference",
    39999,
    "ambiguous reference to '~name'",
    span { loc = "location", message = "ambiguous reference to '~name'" }
)

err(
    "ambiguous-reference-ir",
    39999,
    "ambiguous reference to '~inst:IRInst'",
    span { loc = "location", message = "ambiguous reference to '~inst:IRInst'" }
)

err(
    "ambiguous-expression",
    39999,
    "ambiguous reference",
    span { loc = "expr:Expr", message = "ambiguous reference" }
)

err(
    "declaration-didnt-declare-anything",
    39999,
    "declaration does not declare anything",
    span { loc = "location", message = "declaration does not declare anything" }
)


-- Load semantic checking diagnostics (part 9) - Operators, Literals, Entry Points, Specialization
-- (inlined from slang-diagnostics-semantic-checking-9.lua)

--
-- 39999: operators and other errors
--

err(
    "expected-prefix-operator",
    39999,
    "function called as prefix operator was not declared `__prefix`",
    span { loc = "callLoc", message = "function called as prefix operator was not declared `__prefix`" },
    note { message = "see definition of '~decl'", span { loc = "decl:Decl" } }
)

err(
    "expected-postfix-operator",
    39999,
    "function called as postfix operator was not declared `__postfix`",
    span { loc = "callLoc", message = "function called as postfix operator was not declared `__postfix`" },
    note { message = "see definition of '~decl'", span { loc = "decl:Decl" } }
)

err(
    "not-enough-arguments",
    39999,
    "not enough arguments to call",
    span { loc = "location", message = "not enough arguments to call (got ~got:Int, expected ~expected:Int)" }
)

err(
    "too-many-arguments",
    39999,
    "too many arguments to call",
    span { loc = "location", message = "too many arguments to call (got ~got:Int, expected ~expected:Int)" }
)

err(
    "invalid-integer-literal-suffix",
    39999,
    "invalid suffix on integer literal",
    span { loc = "location", message = "invalid suffix '~suffix' on integer literal" }
)

err(
    "invalid-floating-point-literal-suffix",
    39999,
    "invalid suffix on floating-point literal",
    span { loc = "location", message = "invalid suffix '~suffix' on floating-point literal" }
)

warning(
    "integer-literal-too-large",
    39999,
    "integer literal is too large to be represented in a signed integer type, interpreting as unsigned",
    span { loc = "location", message = "integer literal is too large to be represented in a signed integer type, interpreting as unsigned" }
)

warning(
    "integer-literal-truncated",
    39999,
    "integer literal truncated",
    span { loc = "location", message = "integer literal '~literal' too large for type '~type' truncated to '~truncatedValue'" }
)

warning(
    "float-literal-unrepresentable",
    39999,
    "floating-point literal unrepresentable",
    span { loc = "location", message = "~type literal '~literal' unrepresentable, converted to '~convertedValue'" }
)

warning(
    "float-literal-too-small",
    39999,
    "floating-point literal too small",
    span { loc = "location", message = "'~literal' is smaller than the smallest representable value for type ~type, converted to '~convertedValue'" }
)

err(
    "matrix-column-or-row-count-is-one",
    39999,
    "matrices with 1 column or row are not supported by the current code generation target",
    span { loc = "location", message = "matrices with 1 column or row are not supported by the current code generation target" }
)

--
-- 38xxx: entry points and specialization
--

err(
    "entry-point-function-not-found",
    38000,
    "no function found matching entry point name '~name'",
    span { loc = "location", message = "no function found matching entry point name '~name'" }
)

err(
    "expected-type-for-specialization-arg",
    38005,
    "expected a type as argument for specialization parameter",
    span { loc = "location", message = "expected a type as argument for specialization parameter '~param'" }
)

warning(
    "specified-stage-doesnt-match-attribute",
    38006,
    "entry point stage mismatch",
    span { loc = "location", message = "entry point '~entryPoint' being compiled for the '~compiledStage' stage has a '[shader(...)]' attribute that specifies the '~attributeStage' stage" }
)

err(
    "entry-point-has-no-stage",
    38007,
    "no stage specified for entry point",
    span { loc = "location", message = "no stage specified for entry point '~entryPoint'; use either a '[shader(\"name\")]' function attribute or the '-stage <name>' command-line option to specify a stage" }
)

err(
    "specialization-parameter-of-name-not-specialized",
    38008,
    "no specialization argument was provided for specialization parameter",
    span { loc = "location", message = "no specialization argument was provided for specialization parameter '~param'" }
)

err(
    "specialization-parameter-not-specialized",
    38008,
    "no specialization argument was provided for specialization parameter",
    span { loc = "location", message = "no specialization argument was provided for specialization parameter" }
)

err(
    "expected-value-of-type-for-specialization-arg",
    38009,
    "expected a constant value for specialization parameter",
    span { loc = "location", message = "expected a constant value of type '~type:Type' as argument for specialization parameter '~param'" }
)

warning(
    "unhandled-mod-on-entry-point-parameter",
    38010,
    "modifier on entry point parameter is unsupported",
    span { loc = "location", message = "~modifier on parameter '~param:Name' is unsupported on entry point parameters and will be ignored" }
)

err(
    "entry-point-cannot-return-resource-type",
    38011,
    "entry point cannot return type that contains resource types",
    span { loc = "location", message = "entry point '~entryPoint:Name' cannot return type '~returnType:Type' that contains resource types" }
)

err(
    "entry-point-cannot-return-array-type",
    38012,
    "entry point cannot return array type",
    span { loc = "location", message = "entry point '~entryPoint:Name' cannot return array type '~returnType:Type'" }
)


-- Load semantic checking diagnostics (part 10) - Interface Requirements, Global Generics, Differentiation, Modules
-- (inlined from slang-diagnostics-semantic-checking-10.lua)

--
-- 381xx: interface requirements
--

err(
    "type-doesnt-implement-interface-requirement",
    38100,
    "missing interface member",
    span { loc = "location", message = "type '~type:Type' does not provide required interface member '~member'" }
)

err(
    "member-does-not-match-requirement-signature",
    38105,
    "interface requirement mismatch",
    span { loc = "member:Decl", message = "member '~member' does not match interface requirement." }
)

err(
    "member-return-type-mismatch",
    38106,
    "return type mismatch",
    span { loc = "member:Decl", message = "member '~member' return type '~actualType:Type' does not match interface requirement return type '~expectedType:Type'." }
)

err(
    "generic-signature-does-not-match-requirement",
    38107,
    "generic signature mismatch",
    span { loc = "location", message = "generic signature of '~member:Name' does not match interface requirement." }
)

err(
    "parameter-direction-does-not-match-requirement",
    38108,
    "parameter direction mismatch",
    span { loc = "param:Decl", message = "parameter '~param' direction '~actualDirection:ParamPassingMode' does not match interface requirement '~expectedDirection:ParamPassingMode'." }
)

--
-- 381xx: this/init/return_val
--

err(
    "this-expression-outside-of-type-decl",
    38101,
    "'this' used outside aggregate type",
    span { loc = "expr:Expr", message = "'this' expression can only be used in members of an aggregate type" }
)

err(
    "initializer-not-inside-type",
    38102,
    "'init' used outside type",
    span { loc = "decl:Decl", message = "an 'init' declaration is only allowed inside a type or 'extension' declaration" }
)

err(
    "this-type-outside-of-type-decl",
    38103,
    "'This' used outside aggregate type",
    span { loc = "expr:Expr", message = "'This' type can only be used inside of an aggregate type" }
)

err(
    "return-val-not-available",
    38104,
    "'__return_val' not available",
    span { loc = "expr:Expr", message = "cannot use '__return_val' here. '__return_val' is defined only in functions that return a non-copyable value." }
)

--
-- 380xx: generics and type arguments
--

err(
    "type-argument-for-generic-parameter-does-not-conform-to-interface",
    38021,
    "type argument doesn't conform to interface",
    span { loc = "location", message = "type argument `~typeArg:Type` for generic parameter `~param:Name` does not conform to interface `~interface:Type`." }
)

err(
    "cannot-specialize-global-generic-to-itself",
    38022,
    "cannot specialize global type parameter to itself",
    span { loc = "location", message = "the global type parameter '~param:Name' cannot be specialized to itself" }
)

err(
    "cannot-specialize-global-generic-to-another-generic-param",
    38023,
    "cannot specialize using another global type parameter",
    span { loc = "location", message = "the global type parameter '~param:Name' cannot be specialized using another global type parameter ('~otherParam:Name')" }
)

err(
    "invalid-dispatch-thread-id-type",
    38024,
    "invalid SV_DispatchThreadID type",
    span { loc = "location", message = "parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int but is ~type" }
)

err(
    "mismatch-specialization-arguments",
    38025,
    "wrong number of specialization arguments",
    span { loc = "location", message = "expected ~expected:Int specialization arguments (~provided:Int provided)" }
)

err(
    "invalid-form-of-specialization-arg",
    38028,
    "invalid specialization argument form",
    span { loc = "location", message = "global specialization argument ~index:Int has an invalid form." }
)

err(
    "type-argument-does-not-conform-to-interface",
    38029,
    "type argument doesn't conform to interface",
    span { loc = "location", message = "type argument '~typeArg:Type' does not conform to the required interface '~interface:Type'" }
)

--
-- 380xx: differentiation modifiers
--

err(
    "invalid-use-of-no-diff",
    38031,
    "invalid 'no_diff' usage",
    span { loc = "expr:Expr", message = "'no_diff' can only be used to decorate a call or a subscript operation" }
)

err(
    "use-of-no-diff-on-differentiable-func",
    38032,
    "'no_diff' on differentiable function has no meaning",
    span { loc = "expr:Expr", message = "use 'no_diff' on a call to a differentiable function has no meaning." }
)

err(
    "cannot-use-no-diff-in-non-differentiable-func",
    38033,
    "'no_diff' in non-differentiable function",
    span { loc = "expr:Expr", message = "cannot use 'no_diff' in a non-differentiable function." }
)

err(
    "cannot-use-borrow-in-on-differentiable-parameter",
    38034,
    "'borrow in' on differentiable parameter",
    span { loc = "modifier:Modifier", message = "cannot use 'borrow in' on a differentiable parameter." }
)

err(
    "cannot-use-constref-on-differentiable-member-method",
    38034,
    "'[constref]' on differentiable member method",
    span { loc = "attr:Modifier", message = "cannot use '[constref]' on a differentiable member method of a differentiable type." }
)

--
-- 380xx: entry point parameters
--

warning(
    "non-uniform-entry-point-parameter-treated-as-uniform",
    38040,
    "entry point parameter treated as uniform",
    span { loc = "param:Decl", message = "parameter '~param' is treated as 'uniform' because it does not have a system-value semantic." }
)

err(
    "int-val-from-non-int-spec-const-encountered",
    38041,
    "cannot cast non-integer specialization constant to integer",
    span { loc = "location", message = "cannot cast non-integer specialization constant to compile-time integer" }
)

err(
    "implicit-type-coerce-constraint-with-non-implicit-conversion",
    38042,
    "implicit conversion constraint not satisfied",
    span { loc = "location", message = "'~fromType:Type' is not implicitly convertible to '~toType:Type', not satisfying the type coerce constraint '~toType:Type(~fromType:Type)'" }
)

err(
    "type-coerce-constraint-missing-conversion",
    38043,
    "type coerce constraint not satisfied",
    span { loc = "location", message = "'~fromType:Type' is not convertible to '~toType:Type', not satisfying the type coerce constraint '~toType:Type(~fromType:Type)'" }
)

--
-- 382xx: module imports
--

err(
    "recursive-module-import",
    38200,
    "recursive module import",
    span { loc = "location", message = "module `~module:Name` recursively imports itself" }
)

err(
    "error-in-imported-module",
    39999,
    "import failed due to compilation error",
    span { loc = "location", message = "import of module '~module' failed because of a compilation error" }
)

err(
    "glsl-module-not-available",
    38201,
    "'glsl' module not available",
    span { loc = "location", message = "'glsl' module is not available from the current global session. To enable GLSL compatibility mode, specify 'SlangGlobalSessionDesc::enableGLSL' when creating the global session." }
)

-- Note: compilationCeased is a fatal diagnostic that is locationless
fatal(
    "compilation-ceased",
    39999,
    "compilation ceased",
    span { loc = "location" }
)

--
-- 382xx: vector and buffer types
--

err(
    "vector-with-disallowed-element-type-encountered",
    38203,
    "disallowed vector element type",
    span { loc = "location", message = "vector with disallowed element type '~type:IRInst' encountered" }
)

err(
    "vector-with-invalid-element-count-encountered",
    38203,
    "invalid vector element count",
    span { loc = "location", message = "vector has invalid element count '~count', valid values are between '~min' and '~max' inclusive" }
)

err(
    "cannot-use-resource-type-in-structured-buffer",
    38204,
    "resource type in StructuredBuffer",
    span { loc = "location", message = "StructuredBuffer element type '~type:IRInst' cannot contain resource or opaque handle types" }
)

err(
    "recursive-types-found-in-structured-buffer",
    38205,
    "recursive type in structured buffer",
    span { loc = "location", message = "structured buffer element type '~type:Type' contains recursive type references" }
)


-- Load semantic checking diagnostics (part 11) - Type layout and parameter binding
-- (inlined from slang-diagnostics-semantic-checking-11.lua)

-- 39xxx - Type layout and parameter binding.

err(
    "conflicting-explicit-bindings-for-parameter",
    39000,
    "conflicting explicit bindings",
    span { loc = "decl:Decl", message = "conflicting explicit bindings for parameter '~paramName:Name'" }
)

warning(
    "parameter-bindings-overlap",
    39001,
    "explicit binding overlap",
    span { loc = "paramA:Decl", message = "explicit binding for parameter '~paramA' overlaps with parameter '~paramB'" },
    note { message = "see declaration of '~paramB'", span { loc = "paramB:Decl" } }
)

err(
    "unknown-register-class",
    39007,
    "unknown register class",
    span { loc = "location", message = "unknown register class: '~className'" }
)

err(
    "expected-a-register-index",
    39008,
    "expected a register index",
    span { loc = "location", message = "expected a register index after '~className'" }
)

err(
    "expected-space",
    39009,
    "expected 'space'",
    span { loc = "location", message = "expected 'space', got '~got'" }
)

err(
    "expected-space-index",
    39010,
    "expected a register space index after 'space'",
    span { loc = "location", message = "expected a register space index after 'space'" }
)

err(
    "invalid-component-mask",
    39011,
    "invalid register component mask",
    span { loc = "location", message = "invalid register component mask '~mask'." }
)

warning(
    "requested-bindless-space-index-unavailable",
    39012,
    "bindless space index unavailable",
    span { loc = "location", message = "requested bindless space index '~requested:Int' is unavailable, using the next available index '~available:Int'." }
)

warning(
    "register-modifier-but-no-vulkan-layout",
    39013,
    "D3D register without Vulkan binding",
    span { loc = "location", message = "shader parameter '~paramName:Name' has a 'register' specified for D3D, but no '[[vk::binding(...)]]` specified for Vulkan" }
)

err(
    "unexpected-specifier-after-space",
    39014,
    "unexpected specifier after register space",
    span { loc = "location", message = "unexpected specifier after register space: '~specifier'" }
)

err(
    "whole-space-parameter-requires-zero-binding",
    39015,
    "whole descriptor set requires binding 0",
    span { loc = "location", message = "shader parameter '~paramName:Name' consumes whole descriptor sets, so the binding must be in the form '[[vk::binding(0, ...)]]'; the non-zero binding '~binding:Int' is not allowed" }
)

err(
    "dont-expect-out-parameters-for-stage",
    39017,
    "stage does not support out/inout parameters",
    span { loc = "location", message = "the '~stage' stage does not support `out` or `inout` entry point parameters" }
)

err(
    "dont-expect-in-parameters-for-stage",
    39018,
    "stage does not support in parameters",
    span { loc = "location", message = "the '~stage' stage does not support `in` entry point parameters" }
)

warning(
    "global-uniform-not-expected",
    39019,
    "implicit global shader parameter",
    span { loc = "decl:Decl", message = "'~decl' is implicitly a global shader parameter, not a global variable. If a global variable is intended, add the 'static' modifier. If a uniform shader parameter is intended, add the 'uniform' modifier to silence this warning." }
)

err(
    "too-many-shader-record-constant-buffers",
    39020,
    "too many shader record constant buffers",
    span { loc = "location", message = "can have at most one 'shader record' attributed constant buffer; found ~count:Int." }
)

warning(
    "vk-index-without-vk-location",
    39022,
    "vk::index without vk::location",
    span { loc = "location", message = "ignoring '[[vk::index(...)]]` attribute without a corresponding '[[vk::location(...)]]' attribute" }
)

err(
    "mixing-implicit-and-explicit-binding-for-varying-params",
    39023,
    "mixing implicit and explicit varying bindings",
    span { loc = "location", message = "mixing explicit and implicit bindings for varying parameters is not supported (see '~implicitName:Name' and '~explicitName:Name')" }
)

err(
    "conflicting-vulkan-inferred-binding-for-parameter",
    39025,
    "conflicting Vulkan inferred binding",
    span { loc = "decl:Decl", message = "conflicting vulkan inferred binding for parameter '~paramName:Name' overlap is ~overlap1 and ~overlap2" }
)

err(
    "matrix-layout-modifier-on-non-matrix-type",
    39026,
    "matrix layout modifier on non-matrix type",
    span { loc = "location", message = "matrix layout modifier cannot be used on non-matrix type '~type:Type'." }
)

err(
    "get-attribute-at-vertex-must-refer-to-per-vertex-input",
    39027,
    "'GetAttributeAtVertex' must reference vertex input",
    span { loc = "location", message = "'GetAttributeAtVertex' must reference a vertex input directly, and the vertex input must be decorated with 'pervertex' or 'nointerpolation'." }
)

err(
    "not-valid-varying-parameter",
    39028,
    "not a valid varying parameter",
    span { loc = "decl:Decl", message = "parameter '~paramName:Name' is not a valid varying parameter." }
)

err(
    "target-does-not-support-descriptor-handle",
    39029,
    "target does not support 'DescriptorHandle' types",
    span { loc = "location", message = "the current compilation target does not support 'DescriptorHandle' types." }
)

warning(
    "register-modifier-but-no-vk-binding-nor-shift",
    39029,
    "D3D register without Vulkan binding or shift",
    span { loc = "location", message = "shader parameter '~paramName:Name' has a 'register' specified for D3D, but no '[[vk::binding(...)]]` specified for Vulkan, nor is `-fvk-~className-shift` used." }
)

warning(
    "binding-attribute-ignored-on-uniform",
    39071,
    "binding attribute ignored",
    span { loc = "decl:Decl", message = "binding attribute on uniform '~decl' will be ignored since it will be packed into the default constant buffer at descriptor set 0 binding 0. To use explicit bindings, declare the uniform inside a constant buffer." }
)


-- Load semantic checking diagnostics (part 12) - IL code generation
-- (inlined from slang-diagnostics-semantic-checking-12.lua)

--
-- 4xxxx - IL code generation.
--

err(
    "unimplemented-system-value-semantic",
    40006,
    "unknown system-value semantic",
    span { loc = "location", message = "unknown system-value semantic '~semanticName'" }
)

err(
    "unknown-system-value-semantic",
    49999,
    "unknown system-value semantic",
    span { loc = "location", message = "unknown system-value semantic '~semanticName'" }
)

internal(
    "ir-validation-failed",
    40007,
    "IR validation failed",
    span { loc = "location", message = "IR validation failed: ~message" }
)

err(
    "invalid-l-value-for-ref-parameter",
    40008,
    "the form of this l-value argument is not valid for a `ref` parameter",
    span { loc = "location", message = "the form of this l-value argument is not valid for a `ref` parameter" }
)

err(
    "need-compile-time-constant",
    40012,
    "expected a compile-time constant",
    span { loc = "location", message = "expected a compile-time constant" }
)

err(
    "arg-is-not-constexpr",
    40013,
    "argument is not a compile-time constant",
    span { loc = "location", message = "arg ~argIndex:Int in '~funcName:IRInst' is not a compile-time constant" }
)

err(
    "cannot-unroll-loop",
    40020,
    "loop unrolling failed",
    span { loc = "location", message = "loop does not terminate within the limited number of iterations, unrolling is aborted." }
)

fatal(
    "function-never-returns-fatal",
    40030,
    "function never returns",
    span { loc = "location", message = "function '~funcName:IRInst' never returns, compilation ceased." }
)

-- 41000 - IR-level validation issues

warning(
    "unreachable-code",
    41000,
    "unreachable code detected",
    span { loc = "stmt:Stmt", message = "unreachable code detected" }
)

err(
    "recursive-type",
    41001,
    "type contains cyclic reference",
    span { loc = "location", message = "type '~typeName:IRInst' contains cyclic reference to itself." }
)

err(
    "cyclic-interface-dependency",
    41002,
    "interface has cyclic dependency on itself",
    span { loc = "interfaceType:IRInst", message = "interface '~interfaceType' has cyclic dependency on itself through its implementations." }
)

err(
    "missing-return-error",
    41009,
    "non-void function must return",
    span { loc = "location", message = "non-void function must return in all cases for target '~targetName'" }
)

warning(
    "missing-return",
    41010,
    "non-void function does not return in all cases",
    span { loc = "location", message = "non-void function does not return in all cases" }
)

err(
    "profile-incompatible-with-target-switch",
    41011,
    "__target_switch has no compatible target",
    span { loc = "location", message = "__target_switch has no compatable target with current profile '~profile'" }
)

warning(
    "profile-implicitly-upgraded",
    41012,
    "profile implicitly upgraded",
    span { loc = "location", message = "entry point '~entryPoint' uses additional capabilities that are not part of the specified profile '~profile'. The profile setting is automatically updated to include these capabilities: '~capabilities'" }
)

err(
    "profile-implicitly-upgraded-restrictive",
    41012,
    "entry point uses capabilities not in specified profile",
    span { loc = "location", message = "entry point '~entryPoint' uses capabilities that are not part of the specified profile '~profile'. Missing capabilities are: '~capabilities'" }
)

warning(
    "using-uninitialized-out",
    41015,
    "use of uninitialized out parameter",
    span { loc = "location", message = "use of uninitialized out parameter '~paramName'" }
)

warning(
    "using-uninitialized-variable",
    41016,
    "use of uninitialized variable",
    span { loc = "location", message = "use of uninitialized variable '~varName'" }
)

warning(
    "using-uninitialized-value",
    41016,
    "use of uninitialized value",
    span { loc = "location", message = "use of uninitialized value of type '~typeName'" }
)

warning(
    "using-uninitialized-global-variable",
    41017,
    "use of uninitialized global variable",
    span { loc = "location", message = "use of uninitialized global variable '~varName'" }
)

warning(
    "returning-with-uninitialized-out",
    41018,
    "returning without initializing out parameter",
    span { loc = "location", message = "returning without initializing out parameter '~paramName'" }
)

warning(
    "constructor-uninitialized-field",
    41020,
    "exiting constructor without initializing field",
    span { loc = "location", message = "exiting constructor without initializing field '~fieldName'" }
)

warning(
    "field-not-default-initialized",
    41021,
    "default initializer will not initialize field",
    span { loc = "location", message = "default initializer for '~typeName' will not initialize field '~fieldName'" }
)

warning(
    "comma-operator-used-in-expression",
    41024,
    "comma operator used in expression",
    span { loc = "expr:Expr", message = "comma operator used in expression (may be unintended)" }
)

warning(
    "switch-fallthrough-restructured",
    41026,
    "switch fall-through will be restructured",
    span { loc = "location", message = "switch fall-through is not supported by this target and will be restructured; this may affect wave/subgroup convergence if the duplicated code contains wave operations" }
)

err(
    "cannot-default-initialize-resource",
    41024,
    "cannot default-initialize resource type",
    span { loc = "location", message = "cannot default-initialize ~resourceName with '{}'. Resource types must be explicitly initialized" }
)

err(
    "cannot-default-initialize-struct-with-uninitialized-resource",
    41024,
    "cannot default-initialize struct with uninitialized resource",
    span { loc = "location", message = "cannot default-initialize struct '~structName' with '{}' because it contains an uninitialized ~resourceName field" }
)

err(
    "cannot-default-initialize-struct-containing-resources",
    41024,
    "cannot default-initialize struct containing resource fields",
    span { loc = "location", message = "cannot default-initialize struct '~structName' with '{}' because it contains resource fields" }
)


-- Load semantic checking diagnostics (part 13) - AnyValue, Autodiff, Static assertions, Atomics, etc.
-- (inlined from slang-diagnostics-semantic-checking-13.lua)

-- 41xxx - Semantic checking (continued)

err(
    "type-does-not-fit-any-value-size",
    41011,
    "type does not fit in size required by interface",
    span { loc = "location", message = "type '~type:IRInst' does not fit in the size required by its conforming interface." }
)

standalone_note(
    "type-and-limit",
    -1,
    "sizeof(~type:IRInst) is ~size:Int, limit is ~limit:Int",
    span { loc = "location" }
)

err(
    "type-cannot-be-packed-into-any-value",
    41014,
    "type cannot be packed for dynamic dispatch",
    span { loc = "location", message = "type '~type:IRInst' contains fields that cannot be packed into ordinary bytes for dynamic dispatch." }
)

err(
    "loss-of-derivative-due-to-call-of-non-differentiable-function",
    41020,
    "derivative cannot be propagated through non-differentiable call",
    span { loc = "location", message = "derivative cannot be propagated through call to non-~diffLevel-differentiable function `~funcName:IRInst`, use 'no_diff' to clarify intention." }
)

err(
    "loss-of-derivative-assigning-to-non-differentiable-location",
    41024,
    "derivative is lost during assignment",
    span { loc = "location", message = "derivative is lost during assignment to non-differentiable location, use 'detach()' to clarify intention." }
)

err(
    "loss-of-derivative-using-non-differentiable-location-as-out-arg",
    41025,
    "derivative is lost passing non-differentiable location",
    span { loc = "location", message = "derivative is lost when passing a non-differentiable location to an `out` or `inout` parameter, consider passing a temporary variable instead." }
)

err(
    "get-string-hash-must-be-on-string-literal",
    41023,
    "getStringHash requires string literal",
    span { loc = "location", message = "getStringHash can only be called when argument is statically resolvable to a string literal" }
)

warning(
    "operator-shift-left-overflow",
    41030,
    "left shift overflow",
    span { loc = "location", message = "left shift amount exceeds the number of bits and the result will be always zero, (`~lhsType:IRInst' << `~shiftAmount:Int`)." }
)

err(
    "unsupported-use-of-l-value-for-auto-diff",
    41901,
    "unsupported L-value for auto differentiation",
    span { loc = "location", message = "unsupported use of L-value for auto differentiation." }
)

err(
    "invalid-use-of-torch-tensor-type-in-device-func",
    42001,
    "TorchTensor not allowed in device functions",
    span { loc = "location", message = "invalid use of TorchTensor type in device/kernel functions. use `TensorView` instead." }
)

warning(
    "potential-issues-with-prefer-recompute-on-side-effect-method",
    42050,
    "[PreferRecompute] on function with side effects",
    span { loc = "location", message = "~funcName has [PreferRecompute] and may have side effects. side effects may execute multiple times. use [PreferRecompute(SideEffectBehavior.Allow)], or mark function with [__NoSideEffect]" }
)

-- 45xxx - Linking

err(
    "unresolved-symbol",
    45001,
    "unresolved external symbol",
    span { loc = "location", message = "unresolved external symbol '~symbol:IRInst'." }
)

-- 41xxx - Semantic checking (continued)

warning(
    "expect-dynamic-uniform-argument",
    41201,
    "argument might not be dynamic uniform",
    span { loc = "location", message = "argument for '~param:IRInst' might not be a dynamic uniform, use `asDynamicUniform()` to silence this warning." }
)

warning(
    "expect-dynamic-uniform-value",
    41201,
    "value must be dynamic uniform",
    span { loc = "location", message = "value stored at this location must be dynamic uniform, use `asDynamicUniform()` to silence this warning." }
)

err(
    "not-equal-bit-cast-size",
    41202,
    "bit_cast size mismatch",
    span { loc = "location", message = "invalid to bit_cast differently sized types: '~fromType:IRInst' with size '~fromSize:Int' casted into '~toType:IRInst' with size '~toSize:Int'" }
)

err(
    "byte-address-buffer-unaligned",
    41300,
    "invalid byte address buffer alignment",
    span { loc = "location", message = "invalid alignment `~alignment:Int` specified for the byte address buffer resource with the element size of `~elementSize:Int`" }
)

err(
    "static-assertion-failure",
    41400,
    "static assertion failed",
    span { loc = "location", message = "static assertion failed, ~message" }
)

err(
    "static-assertion-failure-without-message",
    41401,
    "static assertion failed",
    span { loc = "location", message = "static assertion failed." }
)

err(
    "static-assertion-condition-not-constant",
    41402,
    "static assertion condition not compile-time constant",
    span { loc = "location", message = "condition for static assertion cannot be evaluated at compile time." }
)

err(
    "multi-sampled-texture-does-not-allow-writes",
    41402,
    "cannot write to multisampled texture",
    span { loc = "location", message = "cannot write to a multisampled texture with target '~target:CodeGenTarget'." }
)

err(
    "invalid-atomic-destination-pointer",
    41403,
    "invalid atomic destination",
    span { loc = "location", message = "cannot perform atomic operation because destination is neither groupshared nor from a device buffer." }
)


-- Load semantic checking diagnostics (part 14) - Target code generation
-- (inlined from slang-diagnostics-semantic-checking-14.lua)

--
-- 5xxxx - Target code generation.
--

internal(
    "missing-existential-bindings-for-parameter",
    50010,
    "missing argument for existential parameter slot",
    span { loc = "location" }
)

warning(
    "spirv-version-not-supported",
    50011,
    "SPIR-V version too old",
    span { loc = "location", message = "Slang's SPIR-V backend only supports SPIR-V version 1.3 and later. Use `-emit-spirv-via-glsl` option to produce SPIR-V 1.0 through 1.2." }
)

err(
    "invalid-mesh-stage-output-topology",
    50060,
    "invalid mesh output topology",
    span { loc = "location", message = "Invalid mesh stage output topology '~topology' for target '~target', must be one of: ~validTopologies" }
)

err(
    "no-type-conformances-found-for-interface",
    50100,
    "no type conformances found",
    span { loc = "location", message = "No type conformances are found for interface '~interfaceType'. Code generation for current target requires at least one implementation type present in the linkage." }
)

err(
    "dynamic-dispatch-on-potentially-uninitialized-existential",
    50101,
    "cannot dispatch on uninitialized interface",
    span { loc = "location", message = "Cannot dynamically dispatch on potentially uninitialized interface object '~object'." }
)

standalone_note(
    "dynamic-dispatch-code-generated-here",
    50102,
    "generated dynamic dispatch code for this site. ~count:Int possible types: '~types'",
    span { loc = "location" }
)

standalone_note(
    "specialized-dynamic-dispatch-code-generated-here",
    50103,
    "generated specialized dynamic dispatch code for this site. ~count:Int possible types: '~types'. specialization arguments: '~specArgs'.",
    span { loc = "location" }
)

err(
    "multi-level-break-unsupported",
    52000,
    "multi-level break not supported",
    span { loc = "location", message = "control flow appears to require multi-level `break`, which Slang does not yet support" }
)

warning(
    "dxil-not-found",
    52001,
    "dxil library not found",
    span { loc = "location", message = "dxil shared library not found, so 'dxc' output cannot be signed! Shader code will not be runnable in non-development environments." }
)

err(
    "pass-through-compiler-not-found",
    52002,
    "pass-through compiler not found",
    span { loc = "location", message = "could not find a suitable pass-through compiler for '~compiler'." }
)

err(
    "cannot-disassemble",
    52003,
    "cannot disassemble",
    span { loc = "location", message = "cannot disassemble '~target'." }
)

err(
    "unable-to-write-file",
    52004,
    "unable to write file",
    span { loc = "location", message = "unable to write file '~path'" }
)

err(
    "unable-to-read-file",
    52005,
    "unable to read file",
    span { loc = "location", message = "unable to read file '~path'" }
)

err(
    "compiler-not-defined-for-transition",
    52006,
    "compiler not defined for transition",
    span { loc = "location", message = "compiler not defined for transition '~sourceTarget' to '~destTarget'." }
)

err(
    "dynamic-dispatch-on-specialize-only-interface",
    52008,
    "dynamic dispatch on specialize-only type",
    span { loc = "location", message = "type '~conformanceType:IRInst' is marked for specialization only, but dynamic dispatch is needed for the call." }
)

err(
    "cannot-emit-reflection-without-target",
    52009,
    "cannot emit reflection JSON",
    span { loc = "location", message = "cannot emit reflection JSON; no compilation target available" }
)

err(
    "ref-param-with-interface-type-in-dynamic-dispatch",
    52010,
    "ref parameter incompatible with dynamic dispatch",
    span { loc = "location", message = "'~paramKind' parameter of type '~paramType:IRInst' cannot be used in a dynamic dispatch context." }
)

warning(
    "mesh-output-must-be-out",
    54001,
    "mesh output must be out",
    span { loc = "location", message = "Mesh shader outputs must be declared with 'out'." }
)

err(
    "mesh-output-must-be-array",
    54002,
    "mesh output must be array",
    span { loc = "location", message = "HLSL style mesh shader outputs must be arrays" }
)

err(
    "mesh-output-array-must-have-size",
    54003,
    "mesh output array must have size",
    span { loc = "location", message = "HLSL style mesh shader output arrays must have a length specified" }
)

warning(
    "unnecessary-hlsl-mesh-output-modifier",
    54004,
    "unnecessary mesh output modifier",
    span { loc = "location", message = "Unnecessary HLSL style mesh shader output modifier" }
)

err(
    "invalid-torch-kernel-return-type",
    55101,
    "invalid pytorch kernel return type",
    span { loc = "location", message = "'~type:IRInst' is not a valid return type for a pytorch kernel function." }
)

err(
    "invalid-torch-kernel-param-type",
    55102,
    "invalid pytorch kernel parameter type",
    span { loc = "location", message = "'~type:IRInst' is not a valid parameter type for a pytorch kernel function." }
)

err(
    "unsupported-builtin-type",
    55200,
    "unsupported builtin type",
    span { loc = "location", message = "'~type:IRInst' is not a supported builtin type for the target." }
)

err(
    "unsupported-recursion",
    55201,
    "recursion not allowed",
    span { loc = "location", message = "recursion detected in call to '~callee:IRInst', but the current code generation target does not allow recursion." }
)

err(
    "system-value-attribute-not-supported",
    55202,
    "system value semantic not supported",
    span { loc = "location", message = "system value semantic '~semanticName' is not supported for the current target." }
)

err(
    "system-value-type-incompatible",
    55203,
    "system value type mismatch",
    span { loc = "location", message = "system value semantic '~semanticName' should have type '~requiredType:IRInst' or be convertible to type '~requiredType:IRInst'." }
)

err(
    "unsupported-target-intrinsic",
    55204,
    "unsupported intrinsic operation",
    span { loc = "location", message = "intrinsic operation '~operation' is not supported for the current target." }
)

err(
    "unsupported-specialization-constant-for-num-threads",
    55205,
    "specialization constants not supported for numthreads",
    span { loc = "location", message = "Specialization constants are not supported in the 'numthreads' attribute for the current target." }
)

fatal(
    "generic-specialization-recursion-cycle",
    55206,
    "recursive generic specialization detected",
    span { loc = "location", message = "generic specialization for '~generic:IRInst' recursively re-entered the same specialization key." }
)

fatal(
    "generic-specialization-budget-exceeded",
    55207,
    "generic specialization exceeded maximum depth",
    span { loc = "location", message = "generic specialization for '~generic:IRInst' exceeded the recursive specialization budget (~budget:int)." }
)

err(
    "unable-to-auto-map-cuda-type-to-host-type",
    56001,
    "CUDA type mapping failed",
    span { loc = "location", message = "Could not automatically map '~type:IRInst' to a host type. Automatic binding generation failed for '~func:IRInst'" }
)

err(
    "attempt-to-query-size-of-unsized-array",
    56002,
    "cannot get size of unsized array",
    span { loc = "location", message = "cannot obtain the size of an unsized array." }
)

fatal(
    "use-of-uninitialized-opaque-handle",
    56003,
    "use of uninitialized opaque handle",
    span { loc = "location", message = "use of uninitialized opaque handle '~handleType:IRInst'." }
)


-- Load semantic checking diagnostics (part 15) - Target code generation and platform-specific diagnostics
-- (inlined from slang-diagnostics-semantic-checking-15.lua)

-- Metal (56101-56104)

err(
    "resource-types-in-constant-buffer-in-parameter-block-not-allowed-on-metal",
    56101,
    "ConstantBuffer with resource types in ParameterBlock not supported on Metal",
    span { loc = "location", message = "nesting a 'ConstantBuffer' containing resource types inside a 'ParameterBlock' is not supported on Metal, please use 'ParameterBlock' instead." }
)

err(
    "division-by-matrix-not-supported",
    56102,
    "division by matrix not supported",
    span { loc = "location", message = "division by matrix is not supported for Metal and WGSL targets." }
)

err(
    "int16-not-supported-in-wgsl",
    56103,
    "16-bit integers not supported in WGSL",
    span { loc = "location", message = "16-bit integer type '~typeName' is not supported by the WGSL backend." }
)

err(
    "assign-to-ref-not-supported",
    56104,
    "mesh output must be assigned as whole struct",
    span { loc = "location", message = "whole struct must be assiged to mesh output at once for Metal target." }
)

-- SPIRV (57001-57004)

warning(
    "spirv-opt-failed",
    57001,
    "spirv-opt optimization failed",
    span { loc = "location", message = "spirv-opt failed. ~error" }
)

err(
    "unknown-patch-constant-parameter",
    57002,
    "unknown patch constant parameter",
    span { loc = "location", message = "unknown patch constant parameter '~param:IRInst'." }
)

err(
    "unknown-tess-partitioning",
    57003,
    "unknown tessellation partitioning",
    span { loc = "location", message = "unknown tessellation partitioning '~partitioning'." }
)

err(
    "output-spv-is-empty",
    57004,
    "SPIR-V output contains no exported symbols",
    span { loc = "location", message = "output SPIR-V contains no exported symbols. Please make sure to specify at least one entrypoint." }
)

-- GLSL Compatibility (58001-58003)

err(
    "entry-point-must-return-void-when-global-output-present",
    58001,
    "entry point must return void with global outputs",
    span { loc = "location", message = "entry point must return 'void' when global output variables are present." }
)

err(
    "unhandled-glsl-ssbo-type",
    58002,
    "unhandled GLSL SSBO contents",
    span { loc = "location", message = "Unhandled GLSL Shader Storage Buffer Object contents, unsized arrays as a final parameter must be the only parameter" }
)

err(
    "inconsistent-pointer-address-space",
    58003,
    "inconsistent pointer address space",
    span { loc = "location", message = "'~inst:IRInst': use of pointer with inconsistent address space." }
)

-- Autodiff checkpoint reporting notes (-1)

standalone_note(
    "report-checkpoint-intermediates",
    -1,
    "checkpointing context of ~size:Int bytes associated with function: '~func:IRInst'",
    span { loc = "location" }
)

standalone_note(
    "report-checkpoint-variable",
    -1,
    "~size:Int bytes (~typeName) used to checkpoint the following item:",
    span { loc = "location" }
)

standalone_note(
    "report-checkpoint-counter",
    -1,
    "~size:Int bytes (~typeName) used for a loop counter here:",
    span { loc = "location" }
)

standalone_note(
    "report-checkpoint-none",
    -1,
    "no checkpoint contexts to report"
)

-- 9xxxx - Documentation generation (90001)

warning(
    "ignored-documentation-on-overload-candidate",
    90001,
    "documentation comment on overload candidate ignored",
    span { loc = "location:Decl", message = "documentation comment on overload candidate '~location' is ignored" }
)

-- 8xxxx - Issues specific to a particular library/technology/platform/etc.

-- 811xx - NVAPI (81110-81111)

err(
    "nvapi-macro-mismatch",
    81110,
    "conflicting NVAPI macro definitions",
    span { loc = "location", message = "conflicting definitions for NVAPI macro '~macroName': '~existingValue' and '~newValue'" }
)

err(
    "opaque-reference-must-resolve-to-global",
    81111,
    "cannot determine register/space for NVAPI resource",
    span { loc = "location", message = "could not determine register/space for a resource or sampler used with NVAPI" }
)


-- Load semantic checking diagnostics (part 16) - Internal compiler errors, ray tracing, and cooperative matrix
-- (inlined from slang-diagnostics-semantic-checking-16.lua)

-- 99999 - Internal compiler errors, and not-yet-classified diagnostics.

internal(
    "unimplemented",
    99999,
    "unimplemented feature in Slang compiler: ~feature\\nFor assistance, file an issue on GitHub (https://github.com/shader-slang/slang/issues) or join the Slang Discord (https://khr.io/slangdiscord)",
    span { loc = "location" }
)

internal(
    "unexpected",
    99999,
    "unexpected condition encountered in Slang compiler: ~message\\nFor assistance, file an issue on GitHub (https://github.com/shader-slang/slang/issues) or join the Slang Discord (https://khr.io/slangdiscord)",
    span { loc = "location" }
)

internal(
    "internal-compiler-error",
    99999,
    "Slang internal compiler error\\nFor assistance, file an issue on GitHub (https://github.com/shader-slang/slang/issues) or join the Slang Discord (https://khr.io/slangdiscord)",
    span { loc = "location" }
)

err(
    "compilation-aborted",
    99999,
    "Slang compilation aborted due to internal error\\nFor assistance, file an issue on GitHub (https://github.com/shader-slang/slang/issues) or join the Slang Discord (https://khr.io/slangdiscord)"
)

err(
    "compilation-aborted-due-to-exception",
    99999,
    "Slang compilation aborted due to an exception of ~exceptionType: ~exceptionMessage\\nFor assistance, file an issue on GitHub (https://github.com/shader-slang/slang/issues) or join the Slang Discord (https://khr.io/slangdiscord)"
)

internal(
    "serial-debug-verification-failed",
    99999,
    "Verification of serial debug information failed.",
    span { loc = "location" }
)

internal(
    "spirv-validation-failed",
    99999,
    "Validation of generated SPIR-V failed.",
    span { loc = "location" }
)

internal(
    "no-blocks-or-intrinsic",
    99999,
    "no blocks found for function definition",
    span { loc = "location", message = "no blocks found for function definition, is there a '~target' intrinsic missing?" }
)

-- 40100 - Entry point renaming warning

warning(
    "main-entry-point-renamed",
    40100,
    "entry point '~oldName' has been renamed to '~newName'",
    span { loc = "location" }
)

--
-- Ray tracing (40000-40001)
--

err(
    "ray-payload-field-missing-access-qualifiers",
    40000,
    "ray payload field missing access qualifiers",
    span { loc = "field:Decl", message = "field '~field' in ray payload struct must have either 'read' OR 'write' access qualifiers" }
)

err(
    "ray-payload-invalid-stage-in-access-qualifier",
    40001,
    "invalid stage name in ray payload access qualifier",
    span { loc = "location", message = "invalid stage name '~stageName' in ray payload access qualifier; valid stages are 'anyhit', 'closesthit', 'miss', and 'caller'" }
)

--
-- Cooperative Matrix (50000, 51701)
--

err(
    "cooperative-matrix-unsupported-element-type",
    50000,
    "unsupported element type for cooperative matrix",
    span { loc = "location", message = "Element type '~elementType' is not supported for matrix '~matrixUse'." }
)

err(
    "cooperative-matrix-invalid-shape",
    50000,
    "invalid shape for cooperative matrix",
    span { loc = "location", message = "Invalid shape ['~rowCount', '~colCount'] for cooperative matrix '~matrixUse'." }
)

fatal(
    "cooperative-matrix-unsupported-capture",
    51701,
    "'CoopMat.MapElement' per-element function cannot capture buffers, resources or any opaque type values",
    span { loc = "location", message = "'CoopMat.MapElement' per-element function cannot capture buffers, resources or any opaque type values. Consider pre-loading the content of any referenced buffers into a local variable before calling 'CoopMat.MapElement', or moving any referenced resources to global scope." }
)


-- Load semantic checking diagnostics (part 17) - Standalone notes for cross-referencing
-- (inlined from slang-diagnostics-semantic-checking-17.lua)

--
-- -1 - Notes that decorate another diagnostic.
--
-- These are standalone notes used after various error diagnostics to point
-- to related locations (definitions, declarations, etc.)
--

-- Note: These notes use Decl* which provides both the name (via getName()) and
-- location (via loc). The diagnostic system will extract the name for display.

standalone_note(
    "see-definition-of",
    -1,
    "see definition of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see-definition-of-conversion-function",
    -1,
    "see definition of the conversion function '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see-definition-of-constraint",
    -1,
    "see definition of the unsatisfied constraint '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see-definition-of-struct",
    -1,
    "see definition of struct '~name'",
    span { loc = "location" }
)

standalone_note(
    "see-constant-buffer-definition",
    -1,
    "see constant buffer definition.",
    span { loc = "location" }
)

-- Note: seeUsingOf takes both a decl (for name) and a location (for where it's used)
-- This allows pointing to the call site while showing the name of what's being used
standalone_note(
    "see-using-of",
    -1,
    "see using of '~decl:Decl'",
    span { loc = "location" }
)

standalone_note(
    "see-call-of-func",
    -1,
    "see call to '~name'",
    span { loc = "location" }
)

standalone_note(
    "see-previous-definition",
    -1,
    "see previous definition",
    span { loc = "location" }
)

standalone_note(
    "see-previous-definition-of",
    -1,
    "see previous definition of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see-declaration-of",
    -1,
    "see declaration of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see-declaration-of-interface-requirement",
    -1,
    "see interface requirement declaration of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see-overload-considered",
    -1,
    "see overloads considered: '~decl:Decl'.",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see-previous-declaration-of",
    -1,
    "see previous declaration of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "note-explicit-conversion-possible",
    -1,
    "explicit conversion from '~fromType:Type' to '~toType:Type' is possible",
    span { loc = "location" }
)

-- IR-specific variants for use in IR linking and lowering passes
-- These take IRInst* instead of Decl*

standalone_note(
    "see-declaration-of-ir",
    -1,
    "see declaration of '~inst:IRInst'",
    span { loc = "inst:IRInst" }
)

standalone_note(
    "see-call-of-func-ir",
    -1,
    "see call to '~inst:IRInst'",
    span { loc = "location" }
)

-- Modifier variant for notes
standalone_note(
    "see-declaration-of-modifier",
    -1,
    "see declaration of '~modifier:Modifier'",
    span { loc = "modifier:Modifier" }
)

-- ASTNodeType variant for generic references
standalone_note(
    "see-using-of-node-type",
    -1,
    "see using of '~nodeType'",
    span { loc = "location" }
)


-- Process and validate all diagnostics
processed_diagnostics, validation_errors = helpers.process_diagnostics(helpers.diagnostics)

if #validation_errors > 0 then
    error("Diagnostic validation failed:\n" .. table.concat(validation_errors, "\n"))
end

return processed_diagnostics

