//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=a41eec816a55db301ccafcfa867bab048ae6ed92cc99c5f791c2e8474302d15d
//META: purpose=The `cmpEQ` opcode lowers to `OpIEqual` for integer operands (signed and unsigned collapse to the same op) and `OpFOrdEqual` for floating-point operands.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Equality differs from the ordered relationals: the signed and unsigned
// integer rows collapse to a single signedness-agnostic `OpIEqual`
// (verified by compiling both and diffing — `int` and `uint` emit the
// identical op), while the float row uses `OpFOrdEqual`. This pins the
// two surviving distinct cells and documents the int/uint collapse.
// Operands are uniforms; `select` keeps the bool result live.
//
// HLSL, GLSL, Metal and WGSL collapse further still: all three rows are
// the same `==` operator, and it is the operand type the emitter hands the
// target that separates the integer equality from the ordered float
// equality. Each shader target therefore pins its declared operand types
// alongside the comparison that reads them.

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

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = select(sa == sb, 1, 0);
    ibuf[1] = select(ua == ub, 1, 0);
    ibuf[2] = select(fa == fb, 1, 0);
}

// SPV-DAG: OpIEqual %bool %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpFOrdEqual %bool %{{[0-9]+}} %{{[0-9]+}}

// HLSL-DAG: int sa_{{[0-9]+}};
// HLSL-DAG: uint ua_{{[0-9]+}};
// HLSL-DAG: float fa_{{[0-9]+}};
// HLSL-DAG: ({{.*}}sa_{{[0-9]+}}) == ({{.*}}sb_{{[0-9]+}})
// HLSL-DAG: ({{.*}}ua_{{[0-9]+}}) == ({{.*}}ub_{{[0-9]+}})
// HLSL-DAG: ({{.*}}fa_{{[0-9]+}}) == ({{.*}}fb_{{[0-9]+}})

// GLSL-DAG: int sa_{{[0-9]+}};
// GLSL-DAG: uint ua_{{[0-9]+}};
// GLSL-DAG: float fa_{{[0-9]+}};
// GLSL-DAG: ({{.*}}sa_{{[0-9]+}}) == ({{.*}}sb_{{[0-9]+}})
// GLSL-DAG: ({{.*}}ua_{{[0-9]+}}) == ({{.*}}ub_{{[0-9]+}})
// GLSL-DAG: ({{.*}}fa_{{[0-9]+}}) == ({{.*}}fb_{{[0-9]+}})

// METAL-DAG: int sa_{{[0-9]+}};
// METAL-DAG: uint ua_{{[0-9]+}};
// METAL-DAG: float fa_{{[0-9]+}};
// METAL-DAG: ({{.*}}sa_{{[0-9]+}}) == ({{.*}}sb_{{[0-9]+}})
// METAL-DAG: ({{.*}}ua_{{[0-9]+}}) == ({{.*}}ub_{{[0-9]+}})
// METAL-DAG: ({{.*}}fa_{{[0-9]+}}) == ({{.*}}fb_{{[0-9]+}})

// WGSL-DAG: sa_{{[0-9]+}} : i32,
// WGSL-DAG: ua_{{[0-9]+}} : u32,
// WGSL-DAG: fa_{{[0-9]+}} : f32,
// WGSL-DAG: ({{.*}}sa_{{[0-9]+}}) == ({{.*}}sb_{{[0-9]+}})
// WGSL-DAG: ({{.*}}ua_{{[0-9]+}}) == ({{.*}}ub_{{[0-9]+}})
// WGSL-DAG: ({{.*}}fa_{{[0-9]+}}) == ({{.*}}fb_{{[0-9]+}})
