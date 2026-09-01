//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=The `intCast` opcode lowers to `OpSConvert` when widening a signed integer and `OpUConvert` when widening an unsigned integer, realizing the doc's "sign/zero extension chosen by types".
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `intCast` as "Integer-to-integer cast (sign/zero
// extension chosen by types)". The unified IR opcode splits in SPIR-V on
// the source signedness when widening: a signed source emits `OpSConvert`
// (sign-extend), an unsigned source emits `OpUConvert` (zero-extend).
// This pins the distinguishing op per signedness row. Operands are
// uniform narrow ints to keep the conversion live.
//
// HLSL, GLSL and Metal state the same split in their own spelling: the
// widening is a named constructor whose name carries the signedness
// (`int(...)` vs `uint(...)`), applied to a narrow source the emitter
// declares as `int8_t`/`uint8_t` (HLSL, GLSL) or `char`/`uchar` (Metal).
// GLSL additionally has to request `GL_EXT_shader_8bit_storage` for the
// narrow uniform to exist at all, which is checked here too.
//
// WGSL has no integer type other than `i32`/`u32`, so integer *widening*
// has no WGSL spelling and the claim cannot be stated there; see the
// bundle README's `## Untested claims` row (`unsupported-on-target`).

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main

uniform int8_t s8;
uniform uint8_t u8;
uniform RWStructuredBuffer<int> ibuf;
uniform RWStructuredBuffer<uint> ubuf;

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = int(s8);
    ubuf[0] = uint(u8);
}

// SPV-DAG: OpSConvert %int %{{[0-9]+}}
// SPV-DAG: OpUConvert %uint %{{[0-9]+}}

// HLSL-DAG: int8_t s8_{{[0-9]+}};
// HLSL-DAG: uint8_t u8_{{[0-9]+}};
// HLSL-DAG: = int({{.*}}s8_{{[0-9]+}});
// HLSL-DAG: = uint({{.*}}u8_{{[0-9]+}});

// GLSL-DAG: #extension GL_EXT_shader_8bit_storage : require
// GLSL-DAG: int8_t s8_{{[0-9]+}};
// GLSL-DAG: uint8_t u8_{{[0-9]+}};
// GLSL-DAG: = int({{.*}}s8_{{[0-9]+}});
// GLSL-DAG: = uint({{.*}}u8_{{[0-9]+}});

// METAL-DAG: char s8_{{[0-9]+}};
// METAL-DAG: uchar u8_{{[0-9]+}};
// METAL-DAG: = int({{.*}}s8_{{[0-9]+}});
// METAL-DAG: = uint({{.*}}u8_{{[0-9]+}});
