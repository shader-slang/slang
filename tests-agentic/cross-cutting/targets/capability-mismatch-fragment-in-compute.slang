//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#capability-system
//META: doc_section_digest=a0d40ede85e7cb31a316466bcfe695e496506c5ca59e269adafbe2b4671c8e81
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
{
    helper();
}
