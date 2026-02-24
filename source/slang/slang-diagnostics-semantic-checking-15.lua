-- Semantic checking diagnostics (part 15) - Target code generation and platform-specific diagnostics
-- Converted from slang-diagnostic-defs.h lines 304-394

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

-- Metal (56101-56104)

err(
    "resource types in constant buffer in parameter block not allowed on metal",
    56101,
    "ConstantBuffer with resource types in ParameterBlock not supported on Metal",
    span { loc = "location", message = "nesting a 'ConstantBuffer' containing resource types inside a 'ParameterBlock' is not supported on Metal, please use 'ParameterBlock' instead." }
)

err(
    "division by matrix not supported",
    56102,
    "division by matrix not supported",
    span { loc = "location", message = "division by matrix is not supported for Metal and WGSL targets." }
)

err(
    "int16 not supported in wgsl",
    56103,
    "16-bit integers not supported in WGSL",
    span { loc = "location", message = "16-bit integer type '~typeName' is not supported by the WGSL backend." }
)

err(
    "assign to ref not supported",
    56104,
    "mesh output must be assigned as whole struct",
    span { loc = "location", message = "whole struct must be assiged to mesh output at once for Metal target." }
)

-- SPIRV (57001-57004)

warning(
    "spirv opt failed",
    57001,
    "spirv-opt optimization failed",
    span { loc = "location", message = "spirv-opt failed. ~error" }
)

err(
    "unknown patch constant parameter",
    57002,
    "unknown patch constant parameter",
    span { loc = "location", message = "unknown patch constant parameter '~param'." }
)

err(
    "unknown tess partitioning",
    57003,
    "unknown tessellation partitioning",
    span { loc = "location", message = "unknown tessellation partitioning '~partitioning'." }
)

err(
    "output spv is empty",
    57004,
    "SPIR-V output contains no exported symbols",
    span { loc = "location", message = "output SPIR-V contains no exported symbols. Please make sure to specify at least one entrypoint." }
)

-- GLSL Compatibility (58001-58003)

err(
    "entry point must return void when global output present",
    58001,
    "entry point must return void with global outputs",
    span { loc = "location", message = "entry point must return 'void' when global output variables are present." }
)

err(
    "unhandled glsl ssbo type",
    58002,
    "unhandled GLSL SSBO contents",
    span { loc = "location", message = "Unhandled GLSL Shader Storage Buffer Object contents, unsized arrays as a final parameter must be the only parameter" }
)

err(
    "inconsistent pointer address space",
    58003,
    "inconsistent pointer address space",
    span { loc = "location", message = "'~inst': use of pointer with inconsistent address space." }
)

-- Autodiff checkpoint reporting notes (-1)

standalone_note(
    "report checkpoint intermediates",
    -1,
    "checkpointing context of ~size:Int bytes associated with function: '~func'",
    span { loc = "location" }
)

standalone_note(
    "report checkpoint variable",
    -1,
    "~size:Int bytes (~typeName) used to checkpoint the following item:",
    span { loc = "location" }
)

standalone_note(
    "report checkpoint counter",
    -1,
    "~size:Int bytes (~typeName) used for a loop counter here:",
    span { loc = "location" }
)

standalone_note(
    "report checkpoint none",
    -1,
    "no checkpoint contexts to report"
)

-- 9xxxx - Documentation generation (90001)

warning(
    "ignored documentation on overload candidate",
    90001,
    "documentation comment on overload candidate ignored",
    span { loc = "location:Decl", message = "documentation comment on overload candidate '~location' is ignored" }
)

-- 8xxxx - Issues specific to a particular library/technology/platform/etc.

-- 811xx - NVAPI (81110-81111)

err(
    "nvapi macro mismatch",
    81110,
    "conflicting NVAPI macro definitions",
    span { loc = "location", message = "conflicting definitions for NVAPI macro '~macroName': '~existingValue' and '~newValue'" }
)

err(
    "opaque reference must resolve to global",
    81111,
    "cannot determine register/space for NVAPI resource",
    span { loc = "location", message = "could not determine register/space for a resource or sampler used with NVAPI" }
)

end
