-- Semantic checking diagnostics (part 11) - Type layout and parameter binding
-- Converted from slang-diagnostic-defs.h lines 315-440

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

-- 39xxx - Type layout and parameter binding.

err(
    "conflicting explicit bindings for parameter",
    39000,
    "conflicting explicit bindings",
    span { loc = "decl:Decl", message = "conflicting explicit bindings for parameter '~param_name'" }
)

warning(
    "parameter bindings overlap",
    39001,
    "explicit binding overlap",
    span { loc = "param_a:Decl", message = "explicit binding for parameter '~param_a' overlaps with parameter '~param_b'" },
    note { message = "see declaration of '~param_b'", span { loc = "param_b:Decl" } }
)

err(
    "unknown register class",
    39007,
    "unknown register class",
    span { loc = "location", message = "unknown register class: '~class_name'" }
)

err(
    "expected a register index",
    39008,
    "expected a register index",
    span { loc = "location", message = "expected a register index after '~class_name'" }
)

err(
    "expected space",
    39009,
    "expected 'space'",
    span { loc = "location", message = "expected 'space', got '~got'" }
)

err(
    "expected space index",
    39010,
    "expected a register space index after 'space'",
    span { loc = "location", message = "expected a register space index after 'space'" }
)

err(
    "invalid component mask",
    39011,
    "invalid register component mask",
    span { loc = "location", message = "invalid register component mask '~mask'." }
)

warning(
    "requested bindless space index unavailable",
    39012,
    "bindless space index unavailable",
    span { loc = "location", message = "requested bindless space index '~requested:Int' is unavailable, using the next available index '~available:Int'." }
)

warning(
    "register modifier but no vulkan layout",
    39013,
    "D3D register without Vulkan binding",
    span { loc = "location", message = "shader parameter '~param_name' has a 'register' specified for D3D, but no '[[vk::binding(...)]]` specified for Vulkan" }
)

err(
    "unexpected specifier after space",
    39014,
    "unexpected specifier after register space",
    span { loc = "location", message = "unexpected specifier after register space: '~specifier'" }
)

err(
    "whole space parameter requires zero binding",
    39015,
    "whole descriptor set requires binding 0",
    span { loc = "location", message = "shader parameter '~param_name' consumes whole descriptor sets, so the binding must be in the form '[[vk::binding(0, ...)]]'; the non-zero binding '~binding:Int' is not allowed" }
)

err(
    "dont expect out parameters for stage",
    39017,
    "stage does not support out/inout parameters",
    span { loc = "location", message = "the '~stage' stage does not support `out` or `inout` entry point parameters" }
)

err(
    "dont expect in parameters for stage",
    39018,
    "stage does not support in parameters",
    span { loc = "location", message = "the '~stage' stage does not support `in` entry point parameters" }
)

warning(
    "global uniform not expected",
    39019,
    "implicit global shader parameter",
    span { loc = "decl:Decl", message = "'~decl' is implicitly a global shader parameter, not a global variable. If a global variable is intended, add the 'static' modifier. If a uniform shader parameter is intended, add the 'uniform' modifier to silence this warning." }
)

err(
    "too many shader record constant buffers",
    39020,
    "too many shader record constant buffers",
    span { loc = "location", message = "can have at most one 'shader record' attributed constant buffer; found ~count:Int." }
)

warning(
    "vk index without vk location",
    39022,
    "vk::index without vk::location",
    span { loc = "location", message = "ignoring '[[vk::index(...)]]` attribute without a corresponding '[[vk::location(...)]]' attribute" }
)

err(
    "mixing implicit and explicit binding for varying params",
    39023,
    "mixing implicit and explicit varying bindings",
    span { loc = "location", message = "mixing explicit and implicit bindings for varying parameters is not supported (see '~implicit_name:Name' and '~explicit_name:Name')" }
)

err(
    "conflicting vulkan inferred binding for parameter",
    39025,
    "conflicting Vulkan inferred binding",
    span { loc = "decl:Decl", message = "conflicting vulkan inferred binding for parameter '~param_name' overlap is ~overlap1 and ~overlap2" }
)

err(
    "matrix layout modifier on non matrix type",
    39026,
    "matrix layout modifier on non-matrix type",
    span { loc = "location", message = "matrix layout modifier cannot be used on non-matrix type '~type:Type'." }
)

err(
    "get attribute at vertex must refer to per vertex input",
    39027,
    "'GetAttributeAtVertex' must reference vertex input",
    span { loc = "location", message = "'GetAttributeAtVertex' must reference a vertex input directly, and the vertex input must be decorated with 'pervertex' or 'nointerpolation'." }
)

err(
    "not valid varying parameter",
    39028,
    "not a valid varying parameter",
    span { loc = "decl:Decl", message = "parameter '~param_name' is not a valid varying parameter." }
)

err(
    "target does not support descriptor handle",
    39029,
    "target does not support 'DescriptorHandle' types",
    span { loc = "location", message = "the current compilation target does not support 'DescriptorHandle' types." }
)

warning(
    "register modifier but no vk binding nor shift",
    39029,
    "D3D register without Vulkan binding or shift",
    span { loc = "location", message = "shader parameter '~param_name' has a 'register' specified for D3D, but no '[[vk::binding(...)]]` specified for Vulkan, nor is `-fvk-~class_name-shift` used." }
)

warning(
    "binding attribute ignored on uniform",
    39071,
    "binding attribute ignored",
    span { loc = "decl:Decl", message = "binding attribute on uniform '~decl' will be ignored since it will be packed into the default constant buffer at descriptor set 0 binding 0. To use explicit bindings, declare the uniform inside a constant buffer." }
)

end
