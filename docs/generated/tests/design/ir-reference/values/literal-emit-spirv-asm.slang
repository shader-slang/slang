//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=f6737af21fc0ce62bf35ca601aa6d8bc483fabcafaedad874ca80c1323a37207
//META: purpose=An integer literal `42` and a float literal `3.5` emit as a recognizable numeric token on every text-emit target.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Closes the SPIR-V back-end of the literal-payload claim that the
// sibling literal-emit-multi-target.slang covers for HLSL/GLSL/Metal/WGSL/CUDA/CPP.
// The integer literal `42` and the float literal `3.5` survive lowering
// into SPIR-V as named OpConstant declarations; CHECK pins the real
// emitted `OpConstant %int 42` and `OpConstant %float 3.5` tokens.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + 42;
    fbuf[0] = 3.5;
}

// SPV-DAG: OpConstant %int 42
// SPV-DAG: OpConstant %float 3.5
