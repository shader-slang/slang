-- Parsing diagnostics (2xxxx series) and snippet parsing diagnostics (29xxx series)
-- This module is included from slang-diagnostics.lua

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 2xxxx - Parsing
--

err(
    "unexpected token",
    20003,
    "unexpected token",
    span { loc = "location", message = "unexpected ~token_type" }
)

err(
    "unexpected token expected token type",
    20001,
    "unexpected token",
    span { loc = "location", message = "unexpected ~actual_token, expected ~expected_token" }
)

err(
    "unexpected token expected token name",
    20001,
    "unexpected token",
    span { loc = "location", message = "unexpected ~actual_token, expected '~expected_token_name'" }
)

err(
    "token name expected but eof",
    0,
    "unexpected end of file",
    span { loc = "location", message = "\"~token_name\" expected but end of file encountered." }
)

err(
    "token type expected but eof",
    0,
    "unexpected end of file",
    span { loc = "location", message = "~token_type expected but end of file encountered." }
)

err(
    "token name expected",
    20001,
    "expected token",
    span { loc = "location", message = "\"~token_name\" expected" }
)

err(
    "token name expected but eof2",
    20001,
    "unexpected end of file",
    span { loc = "location", message = "\"~token_name\" expected but end of file encountered." }
)

err(
    "token type expected",
    20001,
    "expected token",
    span { loc = "location", message = "~token_type expected" }
)

err(
    "token type expected but eof2",
    20001,
    "unexpected end of file",
    span { loc = "location", message = "~token_type expected but end of file encountered." }
)

err(
    "type name expected but",
    20001,
    "expected type name",
    span { loc = "location", message = "unexpected ~token, expected type name" }
)

err(
    "type name expected but eof",
    20001,
    "expected type name",
    span { loc = "location", message = "type name expected but end of file encountered." }
)

err(
    "unexpected eof",
    20001,
    "unexpected end of file",
    span { loc = "location", message = "Unexpected end of file." }
)

err(
    "syntax error",
    20002,
    "syntax error",
    span { loc = "location", message = "syntax error." }
)

err(
    "invalid empty parenthesis expr",
    20005,
    "invalid empty parentheses expression",
    span { loc = "location", message = "empty parenthesis '()' is not a valid expression." }
)

err(
    "invalid operator",
    20008,
    "invalid operator",
    span { loc = "location", message = "invalid operator '~op'." }
)

err(
    "invalid spirv version",
    20012,
    "invalid SPIR-V version",
    span { loc = "location", message = "Expecting SPIR-V version as either 'major.minor', or quoted if has patch (eg for SPIR-V 1.2, '1.2' or \"1.2\"')" }
)

err(
    "invalid cuda sm version",
    20013,
    "invalid CUDA SM version",
    span { loc = "location", message = "Expecting CUDA SM version as either 'major.minor', or quoted if has patch (eg for '7.0' or \"7.0\"')" }
)

err(
    "missing layout binding modifier",
    20016,
    "missing 'binding' modifier",
    span { loc = "location", message = "Expecting 'binding' modifier in the layout qualifier here" }
)

err(
    "const not allowed on c style ptr decl",
    20017,
    "'const' not allowed on C-style pointer declaration",
    span { loc = "location", message = "'const' not allowed on pointer typed declarations using the C style '*' operator. If the intent is to restrict the pointed-to value to read-only, use 'Ptr<T, Access.Read>'; if the intent is to make the pointer itself immutable, use 'let' or 'const Ptr<...>'." }
)

err(
    "const not allowed on type",
    20018,
    "invalid 'const' usage",
    span { loc = "location", message = "cannot use 'const' as a type modifier" }
)

warning(
    "unintended empty statement",
    20101,
    "potentially unintended empty statement",
    span { loc = "location", message = "potentially unintended empty statement at this location; use {} instead." }
)

err(
    "unexpected body after semicolon",
    20102,
    "unexpected function body after semicolon",
    span { loc = "location", message = "unexpected function body after signature declaration, is this ';' a typo?" }
)

err(
    "decl not allowed",
    30102,
    "declaration not allowed here",
    span { loc = "location", message = "~decl_type is not allowed here." }
)

-- 29xxx - Snippet parsing and inline asm

err(
    "snippet parsing failed",
    29000,
    "snippet parsing failed",
    span { loc = "location", message = "unable to parse target intrinsic snippet: ~snippet" }
)

err(
    "unrecognized spirv opcode",
    29100,
    "unrecognized SPIR-V opcode",
    span { loc = "location", message = "unrecognized spirv opcode: ~opcode" }
)

err(
    "misplaced result id marker",
    29101,
    "misplaced result-id marker",
    span { loc = "location", message = "the result-id marker must only be used in the last instruction of a spriv_asm expression" }
)

standalone_note(
    "consider op copy object",
    29102,
    "consider adding an OpCopyObject instruction to the end of the spirv_asm expression"
)

standalone_note(
    "no such address",
    29103,
    "unable to take the address of this address-of asm operand",
    span { loc = "location" }
)

err(
    "spirv instruction without result id",
    29104,
    "instruction has no result-id operand",
    span { loc = "location", message = "cannot use this 'x = ~opcode ...' syntax because ~opcode does not have a <result-id> operand" }
)

err(
    "spirv instruction without result type id",
    29105,
    "instruction has no result-type-id operand",
    span { loc = "location", message = "cannot use this 'x : <type> = ~opcode ...' syntax because ~opcode does not have a <result-type-id> operand" }
)

warning(
    "spirv instruction with too many operands",
    29106,
    "too many operands",
    span { loc = "location", message = "too many operands for ~opcode (expected max ~max_operands:Int), did you forget a semicolon?" }
)

err(
    "spirv unable to resolve name",
    29107,
    "unknown SPIR-V identifier",
    span { loc = "location", message = "unknown SPIR-V identifier ~name, it's not a known enumerator or opcode" }
)

err(
    "spirv non constant bitwise or",
    29108,
    "invalid bitwise or operand",
    span { loc = "location", message = "only integer literals and enum names can appear in a bitwise or expression" }
)

err(
    "spirv operand range",
    29109,
    "literal integer out of range",
    span { loc = "location", message = "Literal ints must be in the range 0 to 0xffffffff" }
)

err(
    "unknown target name",
    29110,
    "unknown target name",
    span { loc = "location", message = "unknown target name '~name'" }
)

err(
    "spirv invalid truncate",
    29111,
    "invalid truncate operation",
    span { loc = "location", message = "__truncate has been given a source smaller than its target" }
)

err(
    "spirv instruction with not enough operands",
    29112,
    "not enough operands",
    span { loc = "location", message = "not enough operands for ~opcode" }
)

err(
    "spirv id redefinition",
    29113,
    "SPIR-V id redefinition",
    span { loc = "location", message = "SPIRV id '%~id' is already defined in the current assembly block" }
)

err(
    "spirv undefined id",
    29114,
    "undefined SPIR-V id",
    span { loc = "location", message = "SPIRV id '%~id' is not defined in the current assembly block location" }
)

err(
    "target switch case cannot be a stage",
    29115,
    "cannot use stage name in __target_switch",
    span { loc = "location", message = "cannot use a stage name in '__target_switch', use '__stage_switch' for stage-specific code." }
)

end -- return function(helpers)
