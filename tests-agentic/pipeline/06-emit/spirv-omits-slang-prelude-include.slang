//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#preludes
//META: doc_section_digest=e35f51be3beae3d8e169e53667630c421c042fdc0c864ef7e4a6aea3014b6b82
//META: purpose=Negative: SPIR-V emit does NOT include any slang-*-prelude.h header (SPIR-V is not in the preludes table in the source doc).
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative-shape probe of the preludes claim: the doc's preludes
// table lists HLSL, CUDA, C++ shader, C++ host, Torch, and LLVM. The
// SPIR-V backend is NOT listed and emits no `slang-*-prelude.h`
// reference. SPIR-V also does not contain any C-style `#include`
// because the SPIR-V assembly grammar has none.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: OpCapability Shader
// CHECK-NOT: slang-cpp-prelude.h
// CHECK-NOT: slang-cuda-prelude.h
// CHECK-NOT: slang-hlsl-prelude.h
// CHECK-NOT: #include
