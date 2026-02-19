-- Semantic checking diagnostics (part 4) - Attributes diagnostics
-- Converted from slang-diagnostic-defs.h lines 121-207

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 310xx - Attributes
--

warning(
    "unknown attribute name",
    31000,
    "unknown attribute",
    span { loc = "attr:Modifier", message = "unknown attribute '~attr_name:Name'" }
)

err(
    "attribute argument count mismatch",
    31001,
    "wrong number of attribute arguments",
    span { loc = "attr:Modifier", message = "attribute '~attr_name:Name' expects ~expected arguments (~provided:Int provided)" }
)

err(
    "attribute not applicable",
    31002,
    "invalid attribute placement",
    span { loc = "attr:Modifier", message = "attribute '~attr_name:Name' is not valid here" }
)

err(
    "badly defined patch constant func",
    31003,
    "invalid 'patchconstantfunc' attribute",
    span { loc = "location:Modifier", message = "hull shader '~entry_point_name:Name' has badly defined 'patchconstantfunc' attribute." }
)

err(
    "expected single int arg",
    31004,
    "expected single int argument",
    span { loc = "attr:Modifier", message = "attribute '~attr_name:Name' expects a single int argument" }
)

err(
    "expected single string arg",
    31005,
    "expected single string argument",
    span { loc = "attr:Modifier", message = "attribute '~attr_name:Name' expects a single string argument" }
)

err(
    "attribute function not found",
    31006,
    "function not found for attribute",
    span { loc = "location:Expr", message = "Could not find function '~func_name:Name' for attribute '~attr_name'" }
)

err(
    "attribute expected int arg",
    31007,
    "expected int argument",
    span { loc = "attr:Modifier", message = "attribute '~attr_name:Name' expects argument ~arg_index:Int to be int" }
)

err(
    "attribute expected string arg",
    31008,
    "expected string argument",
    span { loc = "attr:Modifier", message = "attribute '~attr_name:Name' expects argument ~arg_index:Int to be string" }
)

err(
    "expected single float arg",
    31009,
    "expected single float argument",
    span { loc = "attr:Modifier", message = "attribute '~attr_name:Name' expects a single floating point argument" }
)

--
-- 311xx - Attributes (continued)
--

err(
    "unknown stage name",
    31100,
    "unknown stage name",
    span { loc = "location", message = "unknown stage name '~stage_name'" }
)

err(
    "unknown image format name",
    31101,
    "unknown image format",
    span { loc = "location:Expr", message = "unknown image format '~format_name'" }
)

err(
    "unknown diagnostic name",
    31101,
    "unknown diagnostic",
    span { loc = "location", message = "unknown diagnostic '~diagnostic_name'" }
)

err(
    "non positive num threads",
    31102,
    "invalid 'numthreads' value",
    span { loc = "attr:Modifier", message = "expected a positive integer in 'numthreads' attribute, got '~value:Int'" }
)

err(
    "invalid wave size",
    31103,
    "invalid 'WaveSize' value",
    span { loc = "attr:Modifier", message = "expected a power of 2 between 4 and 128, inclusive, in 'WaveSize' attribute, got '~value:Int'" }
)

warning(
    "explicit uniform location",
    31104,
    "explicit binding of uniform discouraged",
    span { loc = "var:Decl", message = "Explicit binding of uniform locations is discouraged. Prefer 'ConstantBuffer<~type:Type>' over 'uniform ~type:Type'" }
)

warning(
    "image format unsupported by backend",
    31105,
    "Image format '~format' is not explicitly supported by the ~backend backend, using supported format '~replacement' instead.",
    span { loc = "location" }  -- No span message: this diagnostic has no meaningful source location
)

err(
    "invalid attribute target",
    31120,
    "invalid syntax target for user defined attribute",
    span { loc = "attr:Modifier", message = "invalid syntax target for user defined attribute" }
)

err(
    "attribute usage attribute must be on non generic struct",
    31125,
    "[__AttributeUsage] requires non-generic struct",
    span { loc = "attr:Modifier", message = "[__AttributeUsage] can only be applied to non-generic struct definitions" }
)

err(
    "any value size exceeds limit",
    31121,
    "'anyValueSize' exceeds limit",
    span { loc = "location", message = "'anyValueSize' cannot exceed ~max_size:Int" }
)

err(
    "associated type not allowed in com interface",
    31122,
    "associatedtype not allowed in [COM] interface",
    span { loc = "decl:Decl", message = "associatedtype not allowed in a [COM] interface" }
)

err(
    "invalid guid",
    31123,
    "invalid GUID",
    span { loc = "attr:Modifier", message = "'~guid' is not a valid GUID" }
)

err(
    "struct cannot implement com interface",
    31124,
    "struct cannot implement [COM] interface",
    span { loc = "decl:Decl", message = "a struct type cannot implement a [COM] interface" }
)

err(
    "interface inheriting com must be com",
    31124,
    "interface inheriting [COM] must be [COM]",
    span { loc = "decl:Decl", message = "an interface type that inherits from a [COM] interface must itself be a [COM] interface" }
)

end
