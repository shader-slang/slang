//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=01592114ee53e6fb4d64d35bfd46e14fb1203ef5ea4792bcbf05944b2edfe91c
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
