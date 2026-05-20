//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#preludes
//META: doc_section_digest=aa6dfad5c535c2a139def7eece16399b9abb61a5dab99ce3129b1ab73720d4de
//META: purpose=Emitted C++ output references the C++ prelude — observe `SLANG_PRELUDE_EXPORT` (defined in slang-cpp-prelude.h) on the entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `prelude/slang-cpp-prelude.h` defines `SLANG_PRELUDE_EXPORT`. The
// `-target cpp` emit references the prelude and marks entry points
// with `SLANG_PRELUDE_EXPORT`. Observing the marker in the emitted
// text demonstrates the prelude is referenced from the output.

//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CPP: SLANG_PRELUDE_EXPORT
