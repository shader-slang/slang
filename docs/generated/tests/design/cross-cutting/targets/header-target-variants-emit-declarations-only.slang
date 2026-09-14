//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_section_digest=09ecdf153c06b46e85b5bfd65a0136d6c1516c1e42e5e33b4b02ee440f2f963c
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: purpose=The header enumerators of the C++ and CUDA target groups (`-target hpp`, `-target cuh`) route through the same emit backend as their source variants but write a declarations-only file with no kernel definition.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a target group's header variant is a format variation
// of the same backend rather than a separate one: it emits the same
// language's prelude and declarations but stops short of the kernel.
// The same compute kernel is emitted twice, once per header target;
// each CHECK pins the include-guard plus that language's prelude, and
// each CHECK-NOT pins the definition marker the source variant of the
// same group would have emitted (`SLANG_PRELUDE_EXPORT` for C++,
// `__global__` for CUDA), which is what makes the file
// declarations-only.

//TEST:SIMPLE(filecheck=HPP):-target hpp -entry main -stage compute
//TEST:SIMPLE(filecheck=CUH):-target cuh -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// HPP: #pragma once
// HPP: slang-cpp-prelude.h
// HPP-NOT: SLANG_PRELUDE_EXPORT

// CUH: #pragma once
// CUH: slang-cuda-prelude.h
// CUH-NOT: __global__
