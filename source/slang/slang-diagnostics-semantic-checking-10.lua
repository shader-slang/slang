-- Semantic checking diagnostics (part 10) - Interface Requirements, Global Generics, Differentiation, Modules
-- Converted from slang-diagnostic-defs.h lines 147-313

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 381xx: interface requirements
--

err(
    "type doesnt implement interface requirement",
    38100,
    "missing interface member",
    span { loc = "location", message = "type '~type:Type' does not provide required interface member '~member'" }
)

err(
    "member does not match requirement signature",
    38105,
    "interface requirement mismatch",
    span { loc = "member:Decl", message = "member '~member' does not match interface requirement." }
)

err(
    "member return type mismatch",
    38106,
    "return type mismatch",
    span { loc = "member:Decl", message = "member '~member' return type '~actual_type:Type' does not match interface requirement return type '~expected_type:Type'." }
)

err(
    "generic signature does not match requirement",
    38107,
    "generic signature mismatch",
    span { loc = "location", message = "generic signature of '~member:Name' does not match interface requirement." }
)

err(
    "parameter direction does not match requirement",
    38108,
    "parameter direction mismatch",
    span { loc = "param:Decl", message = "parameter '~param' direction '~actual_direction' does not match interface requirement '~expected_direction'." }
)

--
-- 381xx: this/init/return_val
--

err(
    "this expression outside of type decl",
    38101,
    "'this' used outside aggregate type",
    span { loc = "expr:Expr", message = "'this' expression can only be used in members of an aggregate type" }
)

err(
    "initializer not inside type",
    38102,
    "'init' used outside type",
    span { loc = "decl:Decl", message = "an 'init' declaration is only allowed inside a type or 'extension' declaration" }
)

err(
    "this type outside of type decl",
    38103,
    "'This' used outside aggregate type",
    span { loc = "expr:Expr", message = "'This' type can only be used inside of an aggregate type" }
)

err(
    "return val not available",
    38104,
    "'__return_val' not available",
    span { loc = "expr:Expr", message = "cannot use '__return_val' here. '__return_val' is defined only in functions that return a non-copyable value." }
)

--
-- 380xx: generics and type arguments
--

err(
    "type argument for generic parameter does not conform to interface",
    38021,
    "type argument doesn't conform to interface",
    span { loc = "location", message = "type argument `~type_arg:Type` for generic parameter `~param:Name` does not conform to interface `~interface:Type`." }
)

err(
    "cannot specialize global generic to itself",
    38022,
    "cannot specialize global type parameter to itself",
    span { loc = "location", message = "the global type parameter '~param:Name' cannot be specialized to itself" }
)

err(
    "cannot specialize global generic to another generic param",
    38023,
    "cannot specialize using another global type parameter",
    span { loc = "location", message = "the global type parameter '~param:Name' cannot be specialized using another global type parameter ('~other_param:Name')" }
)

err(
    "invalid dispatch thread id type",
    38024,
    "invalid SV_DispatchThreadID type",
    span { loc = "location", message = "parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int but is ~type" }
)

err(
    "mismatch specialization arguments",
    38025,
    "wrong number of specialization arguments",
    span { loc = "location", message = "expected ~expected:Int specialization arguments (~provided:Int provided)" }
)

err(
    "invalid form of specialization arg",
    38028,
    "invalid specialization argument form",
    span { loc = "location", message = "global specialization argument ~index:Int has an invalid form." }
)

err(
    "type argument does not conform to interface",
    38029,
    "type argument doesn't conform to interface",
    span { loc = "location", message = "type argument '~type_arg:Type' does not conform to the required interface '~interface:Type'" }
)

--
-- 380xx: differentiation modifiers
--

err(
    "invalid use of no diff",
    38031,
    "invalid 'no_diff' usage",
    span { loc = "expr:Expr", message = "'no_diff' can only be used to decorate a call or a subscript operation" }
)

err(
    "use of no diff on differentiable func",
    38032,
    "'no_diff' on differentiable function has no meaning",
    span { loc = "expr:Expr", message = "use 'no_diff' on a call to a differentiable function has no meaning." }
)

err(
    "cannot use no diff in non differentiable func",
    38033,
    "'no_diff' in non-differentiable function",
    span { loc = "expr:Expr", message = "cannot use 'no_diff' in a non-differentiable function." }
)

err(
    "cannot use borrow in on differentiable parameter",
    38034,
    "'borrow in' on differentiable parameter",
    span { loc = "modifier:Modifier", message = "cannot use 'borrow in' on a differentiable parameter." }
)

err(
    "cannot use constref on differentiable member method",
    38034,
    "'[constref]' on differentiable member method",
    span { loc = "attr:Modifier", message = "cannot use '[constref]' on a differentiable member method of a differentiable type." }
)

--
-- 380xx: entry point parameters
--

warning(
    "non uniform entry point parameter treated as uniform",
    38040,
    "entry point parameter treated as uniform",
    span { loc = "param:Decl", message = "parameter '~param' is treated as 'uniform' because it does not have a system-value semantic." }
)

err(
    "int val from non int spec const encountered",
    38041,
    "cannot cast non-integer specialization constant to integer",
    span { loc = "location", message = "cannot cast non-integer specialization constant to compile-time integer" }
)

--
-- 382xx: module imports
--

err(
    "recursive module import",
    38200,
    "recursive module import",
    span { loc = "location", message = "module `~module:Name` recursively imports itself" }
)

err(
    "error in imported module",
    39999,
    "import failed due to compilation error",
    span { loc = "location", message = "import of module '~module' failed because of a compilation error" }
)

err(
    "glsl module not available",
    38201,
    "'glsl' module not available",
    span { loc = "location", message = "'glsl' module is not available from the current global session. To enable GLSL compatibility mode, specify 'SlangGlobalSessionDesc::enableGLSL' when creating the global session." }
)

-- Note: compilationCeased is a fatal diagnostic that is locationless
err(
    "compilation ceased",
    39999,
    "compilation ceased",
    span { loc = "location" }
)

--
-- 382xx: vector and buffer types
--

err(
    "vector with disallowed element type encountered",
    38203,
    "disallowed vector element type",
    span { loc = "location", message = "vector with disallowed element type '~type' encountered" }
)

err(
    "vector with invalid element count encountered",
    38203,
    "invalid vector element count",
    span { loc = "location", message = "vector has invalid element count '~count', valid values are between '~min' and '~max' inclusive" }
)

err(
    "cannot use resource type in structured buffer",
    38204,
    "resource type in StructuredBuffer",
    span { loc = "location", message = "StructuredBuffer element type '~type' cannot contain resource or opaque handle types" }
)

err(
    "recursive types found in structured buffer",
    38205,
    "recursive type in structured buffer",
    span { loc = "location", message = "structured buffer element type '~type:Type' contains recursive type references" }
)

end
