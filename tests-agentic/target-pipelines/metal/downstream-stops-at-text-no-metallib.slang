//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#downstream-apple-metal-compiler
//META: doc_section_digest=a18030895beb62382852be4a9918954cf153c6a439ce82644504b2f07adb8d72
//META: purpose=Bare `-target metal` stops at Metal text; the Apple `metal` downstream compiler is invoked only for MetalLib/MetalLibAssembly, so the emitted output is plain MSL source (no .metallib bytes).
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-target metal` produces only the Metal source artifact. We
// confirm the output is human-readable MSL (begins with the
// standard `#include` block) and lacks a `void main(` HLSL-style
// signature — Metal renames `main` to `main_0`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include <metal_stdlib>
// CHECK: void main_0(
// CHECK-NOT: void main(
