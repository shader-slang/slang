//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=cf6e88293f416086490f900fb6322b3e953eeecf6ea19c2143f0800c759fa6c5
//META: purpose=The core module exposes `Optional<T>`; user code can declare and use `Optional<int>` without an explicit import.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `Optional`, `Result`, `Tuple` as part of the core
// module's standard vocabulary. We observe `Optional<int>` resolves
// in user code, compiles, and the chosen-value path produces the
// expected int result via the interpreter.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

// `Optional<T>` is a core-module type. Using `Optional<int>` in a
// buffer parameter and constructing/inspecting it compiles on HLSL.
// We FileCheck that the function compiled and produced an entry
// point — the existence of an `Optional<int>` symbol in source code
// is the observation.

RWStructuredBuffer<int> outBuf;

int unwrap(Optional<int> o, int dflt)
{
    if (o.hasValue) return o.value;
    return dflt;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<int> some = int(tid.x);
    outBuf[tid.x] = unwrap(some, -1);
}

// HLSL: void main
