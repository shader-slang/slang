//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#c
//META: doc_section_digest=98bae1408ef948818e6d18a391c8a3a89d2f13645057f93200d977f0f093501d
//META: purpose=The C++ backend emits C++ source that includes slang-cpp-prelude.h and exports the entry point with SLANG_PRELUDE_EXPORT.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The C++ backend (slang-emit-cpp.{h,cpp}) pairs the emitted source
// with the C++ shader prelude (prelude/slang-cpp-prelude.h) and
// wraps the compute entry point with SLANG_PRELUDE_EXPORT-tagged
// host shim functions. The runtime support these shims call into
// lives in source/slang-rt/.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include {{.*}}slang-cpp-prelude.h
// CHECK: SLANG_PRELUDE_EXPORT
// CHECK: void main_{{[0-9]+}}(ComputeVaryingInput
