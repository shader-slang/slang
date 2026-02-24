-- Semantic checking diagnostics (part 16) - Internal compiler errors, ray tracing, and cooperative matrix
-- Converted from slang-diagnostic-defs.h lines 165-260

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning
local internal = helpers.internal
local fatal = helpers.fatal

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

end
