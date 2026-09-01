//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=5fdd0ac6daa767e15b8f1dad12d2d7a2846fe891f83f100e2466f794797eb465
//META: purpose=`introduceExplicitGlobalContext` (reached by the Metal fallthrough) packs loose `uniform` globals into a `GlobalParams` struct passed as a `constant*` kernel parameter.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that Metal falls through the `undoParameterCopy` /
// `transformParamsToConstRef` arm into the shader-object arm, which is
// what runs `introduceExplicitGlobalContext`. Module-scope `uniform`
// scalars have no home in an MSL kernel signature, so they must be
// gathered into one struct and passed by pointer. CHECK asserts a
// `GlobalParams_<N>` struct is declared and that the kernel receives a
// `GlobalParams_<N> constant*` argument.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

uniform int scale;
uniform float bias;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x) * float(scale) + bias;
}

// CHECK: struct GlobalParams_{{[0-9]+}}
// CHECK: GlobalParams_{{[0-9]+}} constant* globalParams_{{[0-9]+}}
