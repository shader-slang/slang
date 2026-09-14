//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=The `shr` opcode lowers to `OpShiftRightArithmetic` for a signed operand and `OpShiftRightLogical` for an unsigned operand, realizing the doc's "arithmetic / logical chosen by operand signedness".
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `shr` is "Right-shift (arithmetic / logical chosen by
// operand signedness)". The unified IR `shr` opcode (sibling
// arith-shifts.slang pins the IR form) splits in SPIR-V: a signed value
// emits `OpShiftRightArithmetic`, an unsigned value emits
// `OpShiftRightLogical`. Operands are uniforms to defeat folding.
//
// HLSL, GLSL, Metal and WGSL all write one `>>` token for both rows and
// take the arithmetic-vs-logical decision from the shifted value's own
// type, so on those targets the claim shows up as the operand type the
// emitter declares. Metal and WGSL add a second visible per-target step:
// both require the shift *amount* to be unsigned, so the emitter wraps the
// `int amt` in `uint(...)` / `u32(...)` while leaving the shifted value's
// signedness untouched — exactly the distinction this claim is about.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform int sa;
uniform uint ua;
uniform int amt;
uniform RWStructuredBuffer<int> ibuf;
uniform RWStructuredBuffer<uint> ubuf;

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = sa >> amt;
    ubuf[0] = ua >> amt;
}

// SPV-DAG: OpShiftRightArithmetic %int %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpShiftRightLogical %uint %{{[0-9]+}} %{{[0-9]+}}

// HLSL-DAG: int sa_{{[0-9]+}};
// HLSL-DAG: uint ua_{{[0-9]+}};
// HLSL-DAG: ({{.*}}sa_{{[0-9]+}}) >> ({{.*}}amt_{{[0-9]+}});
// HLSL-DAG: ({{.*}}ua_{{[0-9]+}}) >> ({{.*}}amt_{{[0-9]+}});

// GLSL-DAG: int sa_{{[0-9]+}};
// GLSL-DAG: uint ua_{{[0-9]+}};
// GLSL-DAG: ({{.*}}sa_{{[0-9]+}}) >> ({{.*}}amt_{{[0-9]+}});
// GLSL-DAG: ({{.*}}ua_{{[0-9]+}}) >> ({{.*}}amt_{{[0-9]+}});

// METAL-DAG: int sa_{{[0-9]+}};
// METAL-DAG: uint ua_{{[0-9]+}};
// METAL-DAG: ({{.*}}sa_{{[0-9]+}}) >> uint({{.*}}amt_{{[0-9]+}});
// METAL-DAG: ({{.*}}ua_{{[0-9]+}}) >> uint({{.*}}amt_{{[0-9]+}});

// WGSL-DAG: sa_{{[0-9]+}} : i32,
// WGSL-DAG: ua_{{[0-9]+}} : u32,
// WGSL-DAG: ({{.*}}sa_{{[0-9]+}}) >> (u32({{.*}}amt_{{[0-9]+}}))
// WGSL-DAG: ({{.*}}ua_{{[0-9]+}}) >> (u32({{.*}}amt_{{[0-9]+}}))
