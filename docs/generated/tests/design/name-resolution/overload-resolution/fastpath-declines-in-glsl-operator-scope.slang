//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=1c3af19385b1b45e53fc2911de4a5495ab14067ddf0d2925bb4569db9c7887aa
//META: purpose=In GLSL operator scope the fast path declines matrix operands, so mat * mat resolves through the glsl module's overload to an algebraic matrix product.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GLSL-operator-scope carve-out: with the `glsl` module in scope
// the fast path refuses matrix operands and the general path selects the
// module's `operator*`, which is the algebraic product. Multiplying {{1,2},{3,4}}
// by the identity leaves the off-diagonal entries intact; CHECK asserts 2 and 3
// in the buffer, whereas a component-wise fast-path multiply would give 0.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -allow-glsl -output-using-type

import glsl;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = outputBuffer[0] + 1.0;
    float2x2 m = float2x2(a, 2.0 * a, 3.0 * a, 4.0 * a);
    float2x2 identity = float2x2(a, 0.0, 0.0, a);
    float2x2 prod = m * identity;
    outputBuffer[0] = prod[0][1];
    outputBuffer[1] = prod[1][0];
    // CHECK: 2.0
    // CHECK: 3.0
}
