//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#makediffpair
//META: doc_section_digest=9c6b7a39271eb0f667fd936efc160c0082114c1a92422a72493cec6e8b0fc6cb
//META: purpose=processPairTypes rewrites MakeDiffPair into a makeStruct over an auto-generated pair struct, so the pair reaches target text as a two-field struct.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies what happens to a differential pair *after* the opcodes are gone.
// The sibling test `pair-opcodes-produce-no-target-code.slang` shows that no
// opcode name survives; this one shows what replaces them. The pair here is
// passed across a function boundary — it is the argument and the result of the
// forward-mode derivative of `myfunc` — so the auto-generated struct cannot be
// scalarized away and has to be declared in the emitted text. Each CHECK
// requires a declaration of `DiffPair_float_<n>` followed by its two fields;
// the fields are matched with `CHECK-DAG` because WGSL spells a member as
// `name : type` while the C-family targets spell it `type name`, so only the
// member names are common to all six.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float myfunc(float x)
{
    return x * x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(myfunc)(DifferentialPair<float>(a, d)).d;
}

//HLSL: struct DiffPair_float_{{[0-9]+}}
//HLSL-DAG: primal_{{[0-9]+}}
//HLSL-DAG: differential_{{[0-9]+}}

//GLSL: struct DiffPair_float_{{[0-9]+}}
//GLSL-DAG: primal_{{[0-9]+}}
//GLSL-DAG: differential_{{[0-9]+}}

//METAL: struct DiffPair_float_{{[0-9]+}}
//METAL-DAG: primal_{{[0-9]+}}
//METAL-DAG: differential_{{[0-9]+}}

//WGSL: struct DiffPair_float_{{[0-9]+}}
//WGSL-DAG: primal_{{[0-9]+}}
//WGSL-DAG: differential_{{[0-9]+}}

//CUDA: struct DiffPair_float_{{[0-9]+}}
//CUDA-DAG: primal_{{[0-9]+}}
//CUDA-DAG: differential_{{[0-9]+}}

//CPP: struct DiffPair_float_{{[0-9]+}}
//CPP-DAG: primal_{{[0-9]+}}
//CPP-DAG: differential_{{[0-9]+}}
