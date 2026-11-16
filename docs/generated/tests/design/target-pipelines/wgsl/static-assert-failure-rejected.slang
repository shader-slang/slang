//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:39:19+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=9170fd752e08e7e2778f564ecb1cabc328722ba47d9d53a46dfb89d7feaa308a
//META: purpose=A failed static_assert with a message is reported as E41400 on the WGSL target
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target wgsl -entry main -stage compute

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    static_assert(false, "boom");
}

// CHECK: E41400
