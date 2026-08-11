//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=The unified `div` IR opcode lowers to three distinct SPIR-V ops keyed by operand type: `OpSDiv` for signed, `OpUDiv` for unsigned, `OpFDiv` for floating-point.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `div` is "Division (signed / unsigned / floating-point
// keyed by operand types)". The IR opcode is one (`div`); the SPIR-V
// legalizer branches it into three distinct ops. This pins the
// distinguishing emitted token per operand-type row of that branch.
// Operands come from uniforms so the constant folder cannot collapse the
// division.
//
// The four shader-source back-ends have no signedness-keyed division
// opcode to pick: HLSL, GLSL, Metal and WGSL all spell every row as the
// same `/` operator, and the signed / unsigned / floating-point choice is
// carried by the *types* the emitter gives the operands and the result. So
// each of those targets is checked on both halves — the declared type of
// the operand it reads, and the typed result of the division that consumes
// it. That is the per-target form of the same claim, and it is what would
// regress if a back-end dropped or widened the operand's signedness.

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
    ibuf[0] = sa / sb;
    ubuf[0] = ua / ub;
    fbuf[0] = fa / fb;
}

// SPV-DAG: OpSDiv %int %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpUDiv %uint %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpFDiv %float %{{[0-9]+}} %{{[0-9]+}}

// HLSL-DAG: int sa_{{[0-9]+}};
// HLSL-DAG: uint ua_{{[0-9]+}};
// HLSL-DAG: float fa_{{[0-9]+}};
// HLSL-DAG: int {{[A-Za-z_0-9]+}} = {{.*}}sa_{{[0-9]+}} / {{.*}}sb_{{[0-9]+}};
// HLSL-DAG: uint {{[A-Za-z_0-9]+}} = {{.*}}ua_{{[0-9]+}} / {{.*}}ub_{{[0-9]+}};
// HLSL-DAG: {{.*}}fa_{{[0-9]+}} / {{.*}}fb_{{[0-9]+}};

// GLSL-DAG: int sa_{{[0-9]+}};
// GLSL-DAG: uint ua_{{[0-9]+}};
// GLSL-DAG: float fa_{{[0-9]+}};
// GLSL-DAG: int {{[A-Za-z_0-9]+}} = {{.*}}sa_{{[0-9]+}} / {{.*}}sb_{{[0-9]+}};
// GLSL-DAG: uint {{[A-Za-z_0-9]+}} = {{.*}}ua_{{[0-9]+}} / {{.*}}ub_{{[0-9]+}};
// GLSL-DAG: {{.*}}fa_{{[0-9]+}} / {{.*}}fb_{{[0-9]+}};

// METAL-DAG: int sa_{{[0-9]+}};
// METAL-DAG: uint ua_{{[0-9]+}};
// METAL-DAG: float fa_{{[0-9]+}};
// METAL-DAG: int {{[A-Za-z_0-9]+}} = {{.*}}sa_{{[0-9]+}} / {{.*}}sb_{{[0-9]+}};
// METAL-DAG: uint {{[A-Za-z_0-9]+}} = {{.*}}ua_{{[0-9]+}} / {{.*}}ub_{{[0-9]+}};
// METAL-DAG: {{.*}}fa_{{[0-9]+}} / {{.*}}fb_{{[0-9]+}};

// WGSL-DAG: sa_{{[0-9]+}} : i32,
// WGSL-DAG: ua_{{[0-9]+}} : u32,
// WGSL-DAG: fa_{{[0-9]+}} : f32,
// WGSL-DAG: var {{[A-Za-z_0-9]+}} : i32 = {{.*}}sa_{{[0-9]+}} / {{.*}}sb_{{[0-9]+}};
// WGSL-DAG: var {{[A-Za-z_0-9]+}} : u32 = {{.*}}ua_{{[0-9]+}} / {{.*}}ub_{{[0-9]+}};
// WGSL-DAG: {{.*}}fa_{{[0-9]+}} / {{.*}}fb_{{[0-9]+}};
