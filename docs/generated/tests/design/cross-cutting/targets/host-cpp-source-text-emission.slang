//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=09ecdf153c06b46e85b5bfd65a0136d6c1516c1e42e5e33b4b02ee440f2f963c
//META: purpose=The Targets table maps `SLANG_HOST_CPP_SOURCE` alone to host-side C++ source produced by the C++ emit backend; `-target host-cpp` emits C++ text containing the module's extern function.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the restructured C++ host table row: `SLANG_HOST_CPP_SOURCE` is
// the one host target that still flows through the C++ emit backend as
// text (the binary/host-callable siblings moved to a separate row and are
// downstream-compiled). Compiling under `-target host-cpp` must therefore
// yield C++ source text; an entry point referencing an `__extern_cpp`
// function gives codegen a live program to emit, and the CHECK asserts
// the function's unmangled name appears (with a C++ parameter list) in
// the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target host-cpp

__extern_cpp int hostAccumulate(int a, int b)
{
    return a + b;
}

RWStructuredBuffer<int> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    buf[0] = hostAccumulate(1, 2);
}

// CHECK: hostAccumulate(int32_t
// CHECK: hostAccumulate(int(1), int(2))
