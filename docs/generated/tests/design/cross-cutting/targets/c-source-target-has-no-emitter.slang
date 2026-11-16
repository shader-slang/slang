//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:12+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=69b107f4782f9ecf7830ee47173687274ec18b773210afb33c78ee54075e5efa
//META: purpose=`SLANG_C_SOURCE` (`-target c`) is declared but has no working emitter, so compiling to it diagnoses `UnableToGenerateCodeForTarget` (E00028) instead of producing C text.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Targets table lists C source as "declared only": the dispatcher
// has no `C` case, so the target is accepted on the command line but
// code generation reports "unable to generate code for target". The
// kernel touches no buffer so the capability check does not fire first
// and the diagnostic under test is the only error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target c -entry main -stage compute
//CHECK: unable to generate code for target 'c'
// CHECK: E00028

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
}
