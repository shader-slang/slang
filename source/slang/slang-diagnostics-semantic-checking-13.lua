-- Semantic checking diagnostics (part 13) - AnyValue, Autodiff, Static assertions, Atomics, etc.
-- Converted from slang-diagnostic-defs.h lines 264-373

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

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

end
