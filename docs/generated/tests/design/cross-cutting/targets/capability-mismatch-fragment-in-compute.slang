//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#capability-system
//META: doc_section_digest=42bb2b8e4a42e0ceb11d49a4ca220fc9b2fff51da3e1dfe9c94764da839027ca
//META: purpose=The capability system rejects use of a feature whose stage atom is incompatible with the active entry point's stage; `discard` (fragment-only) inside a compute entry point on HLSL is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `discard` requires the `fragment` stage atom. Calling a function
// that uses `discard` from a `compute` entry point on HLSL violates
// the stage keyhole: the two stage atoms (`fragment` for the
// transitive requirement, `compute` for the entry point) populate
// the same `stage` keyhole and are incompatible. The compiler
// emits a "uses features that are not available in 'compute' stage"
// diagnostic at the entry-point signature.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -profile cs_6_1
void helper()
{
    discard;
/*CHECK:
    ^^^^^^^ see using of 'discard'
*/
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
/*CHECK:
     ^^^^ unavailable features in entry point
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
// CHECK: E36107
{
    helper();
}
