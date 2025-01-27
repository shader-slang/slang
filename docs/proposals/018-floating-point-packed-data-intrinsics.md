SP #018: Add floating-point pack/unpack intrinsics
=================

Adds intrinsics for converting vector floating-point data to and from packed unsigned integer values.

## Status

Status: Design Review.

Implementation: N/A

Author: Darren Wihandi and Slang community

Reviewer:

## Background

Floating-point pack and unpack functions provide great utility and exist as built-in intrinsics on GLSL, SPIRV, Metal and WGSL but not on HLSL.
Since Slang's core mudle is derived from HLSL, floating-point pack/unpack intrinsics are not defined and there is no way to access the intrinsics
provided by the other shader language targets.

## Proposed Approach

Slang's core module, which derives from HLSL, already defines integer pack/unpack intrinsics which were introduced in SM 6.6. Some examples are
```
uint32_t4 unpack_u8u32(uint8_t4_packed packedVal);
uint8_t4_packed pack_u8(uint32_t4 unpackedVal);
```

We propose to add more intrinsics, with similar syntax, to cover floating-points. Different variants exist to cover conversion between unorm, snorm, and
unormalized/standard IEEE 754 floats and vectors of 16-bit and 32-bit floats.

A set of unpack intrinsics are added to decompose a 32-bit integer of packed 8-bit or 16-bit float chunks and reinterprets them
as a vector of unorm, snorm or standard floats and halfs. Each 8-bit or 16-bit chunk is converted to either a normalized float
through a conversion rule or to a standard IEEE-754 floating-point.
```
float4 unpack_unorm_f8f32(uint packedVal);
half4 unpack_unorm_f8f16(uint packedVal);

float4 unpack_snorm_f8f32(uint packedVal);
half4 unpack_snorm_f8f16(uint packedVal);

float2 unpack_unorm_f16f32(uint packedVal);
half2 unpack_unorm_f16f16(uint packedVal);

float2 unpack_snorm_f16f32(uint packedVal);
half2 unpack_snorm_f16f16(uint packedVal);

float2 unpack_half_f16f32(uint packedVal);
half4 unpack_half_f16f16(uint packedVal);
```

A set of pack intrinsics are added to pack a vector of unorm, snorm or standard floats and halfs to a 32-bit integer of packed 8-bit or 16-bit float chunks.
Each vector element is converted to an 8-bit or 16-bit integer chunk through conversion rules, then packed into one 32-bit integer value.
```
uint pack_unorm_f8(float4 unpackedVal);
uint pack_unorm_f8(half4 unpackedVal);

uint pack_snorm_f8(float4 unpackedVal);
uint pack_snorm_f8(half4 unpackedVal);

uint pack_unorm_f16(float2 unpackedVal);
uint pack_unorm_f16(half2 unpackedVal);

uint pack_snorm_f16(float2 unpackedVal);
uint pack_snorm_f16(half2 unpackedVal);

uint pack_half_f16(float2 unpackedVal);
uint pack_half_f16(half2 unpackedVal);
```

### Normalized float conversion rules
Normalized float conversion rules are standard across GLSL/SPIRV, Metal and WGSL. Slang follows these standards. Details of the conversion rules for each target can be found in:
- Section 8.4, `Floating-Point Pack and Unpack Functions`, of the GLSL language specification, which is also used by the SPIR-V extended instrucsion for GLSL.
- Section 7.7.1, `Conversion Rules for Normalized Integer Pixel Data Types` of the Metal Shading language specification.
- Sections [16.9, Data Packing Built-in Functions](https://www.w3.org/TR/WGSL/#pack-builtin-functions) and [16.10, Data Unpacking Built-in Functions](https://www.w3.org/TR/WGSL/#unpack-builtin-functions) of the WebGPU Shading language specification.

### Built-in packed Datatypes
Unlike HLSL's implementation with introduces new packed datatypes, `uint8_t4_packed` and `int8_t4_packed`, unsigned 32-bit integers are used directly 
and no new pakced datatypes are introduced.

### Targets without built-in intrinsics
For targets without built-in intrinsics, the implementation is done manually through code with a combination of arithmetic and bitwise operations.
