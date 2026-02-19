-- Semantic checking diagnostics (part 9) - Operators, Literals, Entry Points, Specialization
-- Converted from slang-diagnostic-defs.h lines 229-333

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 39999: operators and other errors
--

-- Note: expectedPrefixOperator (39999) already exists in slang-diagnostics.lua

err(
    "expected postfix operator",
    39999,
    "function called as postfix operator was not declared `__postfix`",
    span { loc = "call_loc", message = "function called as postfix operator was not declared `__postfix`" },
    note { message = "see definition of '~decl'", span { loc = "decl:Decl" } }
)

err(
    "not enough arguments",
    39999,
    "not enough arguments to call",
    span { loc = "location", message = "not enough arguments to call (got ~got:Int, expected ~expected:Int)" }
)

err(
    "too many arguments",
    39999,
    "too many arguments to call",
    span { loc = "location", message = "too many arguments to call (got ~got:Int, expected ~expected:Int)" }
)

err(
    "invalid integer literal suffix",
    39999,
    "invalid suffix on integer literal",
    span { loc = "location", message = "invalid suffix '~suffix' on integer literal" }
)

err(
    "invalid floating point literal suffix",
    39999,
    "invalid suffix on floating-point literal",
    span { loc = "location", message = "invalid suffix '~suffix' on floating-point literal" }
)

warning(
    "integer literal too large",
    39999,
    "integer literal is too large to be represented in a signed integer type, interpreting as unsigned",
    span { loc = "location", message = "integer literal is too large to be represented in a signed integer type, interpreting as unsigned" }
)

warning(
    "integer literal truncated",
    39999,
    "integer literal truncated",
    span { loc = "location", message = "integer literal '~literal' too large for type '~type' truncated to '~truncated_value'" }
)

warning(
    "float literal unrepresentable",
    39999,
    "floating-point literal unrepresentable",
    span { loc = "location", message = "~type literal '~literal' unrepresentable, converted to '~converted_value'" }
)

warning(
    "float literal too small",
    39999,
    "floating-point literal too small",
    span { loc = "location", message = "'~literal' is smaller than the smallest representable value for type ~type, converted to '~converted_value'" }
)

err(
    "matrix column or row count is one",
    39999,
    "matrices with 1 column or row are not supported by the current code generation target",
    span { loc = "location", message = "matrices with 1 column or row are not supported by the current code generation target" }
)

--
-- 38xxx: entry points and specialization
--

err(
    "entry point function not found",
    38000,
    "no function found matching entry point name '~name'",
    span { loc = "location", message = "no function found matching entry point name '~name'" }
)

err(
    "expected type for specialization arg",
    38005,
    "expected a type as argument for specialization parameter",
    span { loc = "location", message = "expected a type as argument for specialization parameter '~param'" }
)

warning(
    "specified stage doesnt match attribute",
    38006,
    "entry point stage mismatch",
    span { loc = "location", message = "entry point '~entry_point' being compiled for the '~compiled_stage' stage has a '[shader(...)]' attribute that specifies the '~attribute_stage' stage" }
)

err(
    "entry point has no stage",
    38007,
    "no stage specified for entry point",
    span { loc = "location", message = "no stage specified for entry point '~entry_point'; use either a '[shader(\"name\")]' function attribute or the '-stage <name>' command-line option to specify a stage" }
)

err(
    "specialization parameter of name not specialized",
    38008,
    "no specialization argument was provided for specialization parameter",
    span { loc = "location", message = "no specialization argument was provided for specialization parameter '~param'" }
)

err(
    "specialization parameter not specialized",
    38008,
    "no specialization argument was provided for specialization parameter",
    span { loc = "location", message = "no specialization argument was provided for specialization parameter" }
)

err(
    "expected value of type for specialization arg",
    38009,
    "expected a constant value for specialization parameter",
    span { loc = "location", message = "expected a constant value of type '~type:Type' as argument for specialization parameter '~param'" }
)

warning(
    "unhandled mod on entry point parameter",
    38010,
    "modifier on entry point parameter is unsupported",
    span { loc = "location", message = "~modifier on parameter '~param:Name' is unsupported on entry point parameters and will be ignored" }
)

err(
    "entry point cannot return resource type",
    38011,
    "entry point cannot return type that contains resource types",
    span { loc = "location", message = "entry point '~entry_point:Name' cannot return type '~return_type:Type' that contains resource types" }
)

end
