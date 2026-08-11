//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=The `irem` opcode lowers to `OpSRem` for signed and `OpUMod` for unsigned remainder, and `frem` lowers to `OpFRem` — three distinct SPIR-V ops keyed by operand type.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the two `%` opcodes — `irem` for integers and `frem`
// for floats — fan out further on the way to SPIR-V. The SPIR-V legalizer
// splits the integer form by signedness (signed `OpSRem` vs unsigned
// `OpUMod`) and emits `OpFRem` for the float form. This pins the
// distinguishing token per operand-type row. Operands are uniforms to
// defeat constant folding.
//
// The four shader-source targets each split the family differently, and
// the float row is where they diverge most, so each gets its own CHECKs:
// HLSL and Metal have a native `fmod`; WGSL applies `%` to floats
// directly; GLSL has no float `%` at all and the emitter synthesizes the
// C-style truncating remainder out of `mod` and `abs` with an explicit
// sign correction for a negative dividend. The two integer rows collapse
// onto one `%` operator on every one of them, with signedness carried by
// the operand type the emitter declares.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform int sa;
uniform int sb;
uniform uint ua;
uniform uint ub;
uniform float fa;
uniform float fb;
uniform RWStructuredBuffer<int> ibuf;
uniform RWStructuredBuffer<uint> ubuf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = sa % sb;
    ubuf[0] = ua % ub;
    fbuf[0] = fmod(fa, fb);
}

// SPV-DAG: OpSRem %int %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpUMod %uint %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpFRem %float %{{[0-9]+}} %{{[0-9]+}}

// HLSL-DAG: int {{[A-Za-z_0-9]+}} = {{.*}}sa_{{[0-9]+}} % {{.*}}sb_{{[0-9]+}};
// HLSL-DAG: uint {{[A-Za-z_0-9]+}} = {{.*}}ua_{{[0-9]+}} % {{.*}}ub_{{[0-9]+}};
// HLSL-DAG: fmod({{.*}}fa_{{[0-9]+}}, {{.*}}fb_{{[0-9]+}});

// GLSL-DAG: int {{[A-Za-z_0-9]+}} = {{.*}}sa_{{[0-9]+}} % {{.*}}sb_{{[0-9]+}};
// GLSL-DAG: uint {{[A-Za-z_0-9]+}} = {{.*}}ua_{{[0-9]+}} % {{.*}}ub_{{[0-9]+}};
// GLSL-DAG: ? -mod(-({{[A-Za-z_0-9]+}}),abs(({{[A-Za-z_0-9]+}}))) : mod(({{[A-Za-z_0-9]+}}),abs(({{[A-Za-z_0-9]+}})))

// METAL-DAG: int {{[A-Za-z_0-9]+}} = {{.*}}sa_{{[0-9]+}} % {{.*}}sb_{{[0-9]+}};
// METAL-DAG: uint {{[A-Za-z_0-9]+}} = {{.*}}ua_{{[0-9]+}} % {{.*}}ub_{{[0-9]+}};
// METAL-DAG: fmod({{.*}}fa_{{[0-9]+}}, {{.*}}fb_{{[0-9]+}});

// WGSL-DAG: var {{[A-Za-z_0-9]+}} : i32 = {{.*}}sa_{{[0-9]+}} % {{.*}}sb_{{[0-9]+}};
// WGSL-DAG: var {{[A-Za-z_0-9]+}} : u32 = {{.*}}ua_{{[0-9]+}} % {{.*}}ub_{{[0-9]+}};
// WGSL-DAG: (({{.*}}fa_{{[0-9]+}})) % (({{.*}}fb_{{[0-9]+}}))
