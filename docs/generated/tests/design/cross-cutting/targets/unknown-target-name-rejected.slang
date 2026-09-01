//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=09ecdf153c06b46e85b5bfd65a0136d6c1516c1e42e5e33b4b02ee440f2f963c
//META: purpose=A `-target` value that does not appear in the documented target table is rejected with `E00013: unknown code generation target`; target parsing is closed-set against `SlangCompileTarget`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe of the Targets section's table: the public set of
// targets is closed (`SlangCompileTarget`). An arbitrary name not in
// the table is rejected at command-line parse time with E00013, the
// dual of the unknown-profile negative.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target zzfooz -entry main -stage compute
//CHECK: unknown code generation target 'zzfooz'
// CHECK: E00013

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}
