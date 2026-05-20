//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#preludes
//META: doc_section_digest=e35f51be3beae3d8e169e53667630c421c042fdc0c864ef7e4a6aea3014b6b82
//META: purpose=Targets that ship a `prelude/` header (HLSL, CUDA, C++) emit text that references it. GLSL, Metal, and WGSL do not include a `prelude/` header (Metal pulls in metal_stdlib, which is the platform's system header, not the Slang `prelude/`).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Preludes section lists per-target prelude headers under
// `prelude/`. HLSL has an NVAPI macro path
// (`SLANG_HLSL_ENABLE_NVAPI`) but does not unconditionally `#include`
// its prelude; the table still groups it with the prelude-having
// targets because the header is shipped under `prelude/`. CUDA and
// C++ shader emit each unconditionally include their matching
// prelude header. GLSL/Metal/WGSL do not use a `prelude/` header.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: SLANG_HLSL_ENABLE_NVAPI

// CUDA: #include {{.*}}slang-cuda-prelude.h

// CPP: #include {{.*}}slang-cpp-prelude.h
// CPP: SLANG_PRELUDE_NAMESPACE

// GLSL emits no `prelude/` header include; verify only that no
// `slang-{cpp,cuda}-prelude.h` reference leaks in.
// GLSL-NOT: slang-cpp-prelude.h
// GLSL-NOT: slang-cuda-prelude.h
// GLSL-NOT: slang-hlsl-prelude.h

// WGSL similarly has no Slang prelude include.
// WGSL-NOT: slang-cpp-prelude.h
// WGSL-NOT: slang-cuda-prelude.h
// WGSL-NOT: slang-hlsl-prelude.h
