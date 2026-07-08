//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:06:56+00:00
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=01592114ee53e6fb4d64d35bfd46e14fb1203ef5ea4792bcbf05944b2edfe91c
//META: purpose=The Targets table maps `SLANG_HOST_CPP_SOURCE` alone to host-side C++ source produced by the C++ emit backend; `-target host-cpp` emits C++ text containing the module's extern function.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the restructured C++ host table row: `SLANG_HOST_CPP_SOURCE` is
// the one host target that still flows through the C++ emit backend as
// text (the binary/host-callable siblings moved to a separate row and are
// downstream-compiled). Compiling a module with an `__extern_cpp` function
// under `-target host-cpp` must therefore yield C++ source; the CHECK
// asserts the function's unmangled name appears in the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target host-cpp

__extern_cpp int hostAccumulate(int a, int b)
{
    return a + b;
}

// CHECK: hostAccumulate
