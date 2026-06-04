//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Metal emit begins with the standard <metal_stdlib>/<metal_math>/<metal_texture> includes and a `using namespace metal;` line.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The MetalSourceEmitter prelude is the first non-blank text in
// every Metal compile. The block establishes the Metal namespace
// before any user-emitted code.

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
