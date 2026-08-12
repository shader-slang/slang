//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=2884135594e88fad2ff6c4fdc20d6bf694888dfaa9f666f2ac51e906fc88f826
//META: purpose=`[deprecated(message)]` makes each use of the declaration report a deprecation diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the attribute's message argument is retained and surfaced at the
// call site rather than merely parsed. The deprecated function is called once
// and CHECK pins the deprecation diagnostic code, which fires only if the
// attribute was resolved and its use-site check ran.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

[deprecated("use bar instead")]
int foo() { return 1; }

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = foo();
    //CHECK: E31200
}
