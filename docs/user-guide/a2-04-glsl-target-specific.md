---
layout: user-guide
permalink: /user-guide/glsl-target-specific
---

# GLSL-Specific Functionalities

This page documents features and behaviors unique to the GLSL target in Slang. For any features or translation rules that are identical to the SPIR-V target, see the [SPIR-V Target Specific](./a2-01-spirv-target-specific.md) page.

> **Note:** The GLSL target in Slang is currently less mature than the SPIR-V target and has several known limitations. While basic functionality works, some advanced features may not be fully supported or may behave differently than expected. Due to fundamental limitations of GLSL, the GLSL target is not expected to achieve feature parity with other backends. For cross-platform use cases, we recommend using the SPIR-V target for more complete and reliable shader compilation. This document is a work in progress and will be updated as the GLSL target matures and more limitations are documented.

## Combined Texture Sampler

Combined texture samplers (e.g., `Sampler2D`) are mapped directly to GLSL sampler types. See SPIR-V page for details on explicit bindings and emulation on other targets.

## System-Value Semantics

System-value semantics are mapped to the corresponding GLSL built-in variables (e.g., `gl_Position`, `gl_FragCoord`, etc.). For a full mapping table, refer to the [SPIR-V Target Specific](./a2-01-spirv-target-specific.md) page.

## `discard` Statement

The `discard` statement in Slang maps directly to GLSL's `discard` keyword, which exits the current fragment shader invocation. No special handling is required.

## HLSL Features Supported in GLSL

Slang supports many HLSL features when targeting GLSL, including:
- Geometry shaders
- Tessellation shaders
- Compute shaders
- Atomics (see below for type support)
- Wave intrinsics (where supported by GLSL extensions)

## Atomic Types

GLSL supports atomic operations on:
- 32-bit integer types (GLSL 4.3+)
- 64-bit integer types (GLSL 4.4+ with `GL_EXT_shader_atomic_int64`)
- 32-bit float types (GLSL 4.6+ with `GLSL_EXT_shader_atomic_float`)
- 16-bit float types (GLSL 4.6+ with `GLSL_EXT_shader_atomic_float2`)
See the SPIR-V page for a comparative table.

## Buffer Types

- `ConstantBuffer` members are emitted as `uniform` parameters in a uniform block.
- `StructuredBuffer` and `ByteAddressBuffer` are translated to shader storage buffers (`buffer` in GLSL 4.3+).
- Layouts can be controlled with `std140`, `std430`, or `scalar` layouts (see options below).

## Matrix Layout

GLSL uses column-major layout by default. Slang will handle row-major to column-major translation as needed. For more on matrix layout and memory layout, see the [SPIR-V Target Specific](./a2-01-spirv-target-specific.md) and [Matrix Layout](./a1-01-matrix-layout.md) pages.

## Entry Points

GLSL requires the entry point to be named `main`. Only one entry point per shader is supported. For multiple entry points or advanced scenarios, see the SPIR-V page.

## Specialization Constants

Slang supports specialization constants in GLSL using the `layout(constant_id = N)` syntax or the `[SpecializationConstant]` attribute. See the SPIR-V page for details.

## Attributes and Layout Qualifiers

Slang attributes such as `[[vk::location]]`, `[[vk::binding]]`, etc., are mapped to GLSL `layout` qualifiers where possible.

## GLSL-Specific Compiler Options

Relevant options for GLSL output:

### -profile glsl_<version>
Select the GLSL version to target (e.g., `-profile glsl_450`).

### -force-glsl-scalar-layout
Use scalar layout for buffer types.

> **Note:** Scalar layout is generally only supported by Vulkan consumers of GLSL, and is not expected to be usable for OpenGL.

### -fvk-use-dx-layout
Use D3D buffer layout rules.

### -fvk-use-gl-layout
Use std430 layout for raw buffer load/stores.

### -line-directive-mode glsl
Emit GLSL-style `#line` directives.

### -default-downstream-compiler glsl <compiler>
Set the downstream GLSL compiler (e.g., glslang).

> **Note:** The GLSL target has a known limitation with constant buffer packing for 3-element vectors, where it cannot always reproduce the same exact buffer layout. For example, when a 3-element vector follows a scalar in a constant buffer, the alignment differs from a 4-element vector, causing incorrect packing.

For all other behaviors, translation rules, and advanced features, refer to the [SPIR-V Target Specific](./a2-01-spirv-target-specific.md) page.
