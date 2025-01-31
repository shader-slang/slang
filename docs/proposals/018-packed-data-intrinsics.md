SP #018: Data pack/unpack intrinsics
====================================

Adds intrinsics for converting numeric vector data to and from packed unsigned integer values.

## Status

Status: Design Review.

Implementation: N/A

Author: Darren Wihandi and Slang community.

Reviewer: Yong He.

## Background

Data packing/unpacking intrinsics provide great utility. Slang's core module, which derives from HLSL, already defines integer pack/unpack intrinsics which were
introduced in SM 6.6. Floating-point variants however are undefined. Floating-point pack/unpacking intrinsics exist as built-in intrinsics on GLSL, SPIRV, Metal
and WGSL but not on HLSL and Slang, and there is no way to access the intrinsics provided by the other shader language targets.

## Proposed Approach

We propose to add new packed-data intrinsics to cover both floating-points and integers. Although the HLSL integer intrinsics are already implemented, integer variants are
also added to obtain independence from the HLSL specs and syntax. For floating-point processing, different variants  exist that handles conversion between unorm, snorm, and
unormalized/standard IEEE 754 floats and vectors of 16-bit and 32-bit floats.

A set of unpack intrinsics are added to decompose a 32-bit integer of packed 8-bit or 16-bit float chunks and reinterprets them
as a vector of unorm, snorm or standard floats and halfs. Each 8-bit or 16-bit chunk is converted to either a normalized float
through a conversion rule or to a standard IEEE-754 floating-point.
```
float4 unpackUnorm4x8ToFloat(uint packedVal);
half4 unpackUnorm4x8ToHalf(uint packedVal);

float4 unpackSnorm4x8ToFloat(uint packedVal);
half4 unpackSnorm4x8ToHalf(uint packedVal);

float2 unpackUnorm2x16ToFloat(uint packedVal);
half2 unpackUnorm2x16ToHalf(uint packedVal);

float2 unpackSnorm2x16ToFloat(uint packedVal);
half2 unpackSnorm2x16ToHalf(uint packedVal);

float2 unpackHalf2x16ToFloat(uint packedVal);
half2 unpackHalf2x16ToHalf(uint packedVal);
```
A set of pack intrinsics are added to pack a vector of unorm, snorm or standard floats and halfs to a 32-bit integer of packed 8-bit or 16-bit float chunks.
Each vector element is converted to an 8-bit or 16-bit integer chunk through conversion rules, then packed into one 32-bit integer value.
```
uint packUnorm4x8(float4 unpackedVal);
uint packUnorm4x8(half4 unpackedVal);

uint packSnorm4x8(float4 unpackedVal);
uint packSnorm4x8(half4 unpackedVal);

uint packUnorm2x16(float2 unpackedVal);
uint packUnorm2x16(half2 unpackedVal);

uint packSnorm2x16(float2 unpackedVal);
uint packSnorm2x16(half2 unpackedVal);

uint packHalf2x16(float2 unpackedVal);
uint packHalf2x16(half2 unpackedVal);
```

A set of unpack intrinsics are added to decompose a 32-bit integer containing four packed 8-bit signed or unsigned integer values and reinterpret them as vectors of 16-bit or 32-bit integers.
These intrinsics support sign extension for signed integers and zero extension for unsigned integers.
```
uint32_t4 unpackUint4x8ToUint32(uint packedVal);
uint16_t4 unpackUint4x8ToUint16(uint packedVal);

int32_t4 unpackInt4x8ToInt32(uint packedVal);
int16_t4 unpackInt4x8ToInt16(uint packedVal);
```

A set of pack intrinsics are added to convert a vector of 16-bit or 32-bit signed or unsigned integers into a 32-bit packed representation, 
storing only the lower 8 bits of each value. Clamped variants clamp each 8-bit value to `[0, 255]` for unsigned values and `[-128, 127]` for signed values.
```
uint packUint4x8(uint32_t4 unpackedVal);
uint packUint4x8(uint16_t4 unpackedVal);

uint packInt4x8(int32_t4 unpackedVal);
uint packInt4x8(int16_t4 unpackedVal);

uint packUint4x8Clamp(int32_t4 unpackedVal);
uint packUint4x8Clamp(int16_t4 unpackedVal);

uint packInt4x8Clamp(int32_t4 unpackedVal);
uint packInt4x8Clamp(int16_t4 unpackedVal);
```

### Normalized float conversion rules
Normalized float conversion rules are standard across GLSL/SPIRV, Metal and WGSL. Slang follows these standards. Details of the conversion rules for each target can be found in:
- Section 8.4, `Floating-Point Pack and Unpack Functions`, of the GLSL language specification, which is also used by the SPIR-V extended instrucsion for GLSL.
- Section 7.7.1, `Conversion Rules for Normalized Integer Pixel Data Types`, of the Metal Shading language specification.
- Sections [16.9  Data Packing Built-in Functions](https://www.w3.org/TR/WGSL/#pack-builtin-functions) and [16.10 Data Unpacking Built-in Functions](https://www.w3.org/TR/WGSL/#unpack-builtin-functions) of the WebGPU Shading language specification.

### Built-in packed Datatypes
Unlike HLSL's implementation with introduces new packed datatypes, `uint8_t4_packed` and `int8_t4_packed`, unsigned 32-bit integers are used directly 
and no new pakced datatypes are introduced.

### Targets without built-in intrinsics
For targets without built-in intrinsics, the implementation is done manually through code with a combination of arithmetic and bitwise operations.
