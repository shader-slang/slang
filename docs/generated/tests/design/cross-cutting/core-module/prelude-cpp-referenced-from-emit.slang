//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#preludes
//META: doc_section_digest=a7e8e82107afcf2792c8394b7f5dff917197cf8708f197b2045955821e3cb62b
//META: purpose=The C++ prelude is referenced from -target cpp output: the emitted text pulls in slang-cpp-prelude.h and uses its SLANG_PRELUDE_EXPORT macro on the entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies preludes are an output-side artefact: nothing in the shader
// mentions them, yet the emitted C++ names the prelude header and uses
// macros the header defines. The CHECKs pin both the reference to
// slang-cpp-prelude.h and the SLANG_PRELUDE_EXPORT marker on a
// generated entry-point wrapper, so a build that stopped emitting the
// prelude reference would fail here rather than at downstream
// compile time.

//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CPP: slang-cpp-prelude.h
// CPP: SLANG_PRELUDE_EXPORT
