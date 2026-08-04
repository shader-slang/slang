//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
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
