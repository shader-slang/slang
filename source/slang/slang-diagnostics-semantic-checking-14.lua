-- Semantic checking diagnostics (part 14) - Target code generation diagnostics
-- Converted from slang-diagnostic-defs.h lines 155-303

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 5xxxx - Target code generation.
--

err(
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

err(
    "use-of-uninitialized-opaque-handle",
    56003,
    "use of uninitialized opaque handle",
    span { loc = "location", message = "use of uninitialized opaque handle '~handleType:IRInst'." }
)

end
