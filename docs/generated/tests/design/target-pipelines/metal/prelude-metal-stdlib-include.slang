//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=Every Metal compile begins with the `<metal_stdlib>` / `<metal_math>` / `<metal_texture>` includes followed by `using namespace metal;`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fixed Metal front matter the prelude emitter writes before
// any user code. The unqualified type names Slang emits (`float4`,
// `texture2d`, ...) only resolve because the namespace is opened here, so
// the block is load-bearing rather than cosmetic. CHECK asserts the three
// includes and the `using namespace` line in emission order.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include <metal_stdlib>
// CHECK: #include <metal_math>
// CHECK: #include <metal_texture>
// CHECK: using namespace metal;
