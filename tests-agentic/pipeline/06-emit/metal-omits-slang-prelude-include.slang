//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#preludes
//META: doc_section_digest=e35f51be3beae3d8e169e53667630c421c042fdc0c864ef7e4a6aea3014b6b82
//META: purpose=Negative: Metal emit does NOT include a slang-*-prelude.h header. It includes the platform's metal_stdlib system header but no Slang `prelude/` header.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative-shape probe of the preludes claim: the doc explicitly
// notes that "Metal, WGSL, and SPIR-V do not use a `prelude/` header
// in the same way" as CUDA/C++. Metal does emit
// `#include <metal_stdlib>`, but that is the platform's system
// header, not the Slang `prelude/`. This test pins the absence of
// any Slang prelude include in the Metal emit.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include <metal_stdlib>
// CHECK-NOT: slang-cpp-prelude.h
// CHECK-NOT: slang-cuda-prelude.h
// CHECK-NOT: slang-hlsl-prelude.h
// CHECK-NOT: slang-metal-prelude.h
