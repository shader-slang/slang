//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#dyn-interface-restrictions
//META: doc_section_digest=733bfc84d5a926cb0990d8b19fc9bdb571304a51dd223d8d0eb5820ebe12888a
//META: purpose=The `dyn interface` validation is a no-op unless the module's language version is 2026 or later, so the same source that `-std 2026` rejects compiles under the default `-std`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the language-version gate itself rather than any one restriction:
// this file declares the generic `dyn interface` that the sibling
// `dyn-interface-declaration-restrictions-rejected.slang` has rejected under
// `-std 2026`, and compiles it with no `-std` at all. CHECK asserts the entry
// point is emitted and its buffer store survives, which can only happen if the
// validation never ran — a diagnostic would have stopped codegen before any
// HLSL was written.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

dyn interface IGen<T>
{
    int get();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<int> buf)
{
    buf[0] = 7;
}

//CHECK: void main(
//CHECK: = int(7)
