-- Semantic checking diagnostics (part 5) - COM Interface, DerivativeMember, Extern Decl, Custom Derivative
-- Converted from slang-diagnostic-defs.h lines 200-300

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 311xx - COM Interface
--

err(
    "struct cannot implement com interface",
    31124,
    "struct types cannot implement COM interfaces",
    span { loc = "decl:Decl", message = "a struct type cannot implement a [COM] interface" }
)

err(
    "interface inheriting com must be com",
    31124,
    "non-COM interface inheriting from COM interface",
    span { loc = "decl:Decl", message = "an interface type that inherits from a [COM] interface must itself be a [COM] interface" }
)

--
-- 3113x - DerivativeMember Attribute
--

err(
    "derivative member attribute must name a member in expected differential type",
    31130,
    "invalid DerivativeMember target",
    span { loc = "attr", message = "[DerivativeMember] must reference to a member in the associated differential type '~diff_type:Type'." }
)

err(
    "invalid use of derivative member attribute parent type is not differentiable",
    31131,
    "DerivativeMember on non-differentiable type",
    span { loc = "attr", message = "invalid use of [DerivativeMember], parent type is not differentiable." }
)

err(
    "derivative member attribute can only be used on members",
    31132,
    "DerivativeMember on non-member",
    span { loc = "attr", message = "[DerivativeMember] is allowed on members only." }
)

--
-- 3114x - Extern Decl
--

err(
    "type of extern decl mismatches original definition",
    31140,
    "extern decl type mismatch",
    span { loc = "decl:Decl", message = "type of `extern` decl '~decl' differs from its original definition. expected '~expected_type:Type'." }
)

err(
    "definition of extern decl mismatches original definition",
    31141,
    "extern decl definition mismatch",
    span { loc = "decl:Decl", message = "`extern` decl '~decl' is not consistent with its original definition." }
)

err(
    "ambiguous original defintion of extern decl",
    31142,
    "ambiguous extern decl target",
    span { loc = "decl:Decl", message = "`extern` decl '~decl' has ambiguous original definitions." }
)

err(
    "missing original defintion of extern decl",
    31143,
    "no original definition for extern decl",
    span { loc = "decl:Decl", message = "no original definition found for `extern` decl '~decl'." }
)

--
-- 3114x - Attribute
--

err(
    "decl already has attribute",
    31146,
    "duplicate attribute",
    span { loc = "attr", message = "'~decl:Decl' already has attribute '[~attr_name]'." }
)

--
-- 3114x-3116x - Custom Derivative
--

err(
    "cannot resolve original function for derivative",
    31147,
    "cannot resolve original function for derivative",
    span { loc = "attr", message = "cannot resolve the original function for the the custom derivative." }
)

err(
    "cannot resolve derivative function",
    31148,
    "cannot resolve derivative function",
    span { loc = "attr", message = "cannot resolve the custom derivative function" }
)

err(
    "custom derivative signature mismatch at position",
    31149,
    "custom derivative parameter type mismatch",
    span { loc = "attr", message = "invalid custom derivative. parameter type mismatch at position ~position:int. expected '~expected_type', got '~actual_type'" }
)

err(
    "custom derivative signature mismatch",
    31150,
    "custom derivative signature mismatch",
    span { loc = "attr", message = "invalid custom derivative. could not resolve function with expected signature '~expected_signature'" }
)

err(
    "cannot resolve generic argument for derivative function",
    31151,
    "cannot deduce generic arguments for derivative",
    span { loc = "attr", message = "The generic arguments to the derivative function cannot be deduced from the parameter list of the original function. Consider using [ForwardDerivative], [BackwardDerivative] or [PrimalSubstitute] attributes on the primal function with explicit generic arguments to associate it with a generic derivative function. Note that [ForwardDerivativeOf], [BackwardDerivativeOf], and [PrimalSubstituteOf] attributes are not supported when the generic arguments to the derivatives cannot be automatically deduced." }
)

err(
    "cannot associate interface requirement with derivative",
    31152,
    "interface requirement cannot have derivative",
    span { loc = "attr", message = "cannot associate an interface requirement with a derivative." }
)

err(
    "cannot use interface requirement as derivative",
    31153,
    "interface requirement cannot be used as derivative",
    span { loc = "attr", message = "cannot use an interface requirement as a derivative." }
)

err(
    "custom derivative signature this param mismatch",
    31154,
    "custom derivative 'this' type mismatch",
    span { loc = "attr", message = "custom derivative does not match expected signature on `this`. Both original and derivative function must have the same `this` type." }
)

err(
    "custom derivative expected static",
    31156,
    "expected static custom derivative",
    span { loc = "attr", message = "expected a static definition for the custom derivative." }
)

err(
    "overloaded func used with derivative of attributes",
    31157,
    "overloaded function in derivative-of attribute",
    span { loc = "attr", message = "cannot resolve overloaded functions for derivative-of attributes." }
)

end
