//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=09ecdf153c06b46e85b5bfd65a0136d6c1516c1e42e5e33b4b02ee440f2f963c
//META: purpose=The Slang round-trip emitter has no public `SlangCompileTarget` value, so `-target slang` names nothing selectable and is rejected with `E00013: unknown code generation target`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the Slang round-trip row of the target table really
// has no user-selectable target behind it. The most plausible
// spelling a reader would try, `-target slang`, is passed on the
// command line; the CHECKs pin the unknown-target diagnostic and its
// code, which shows the name is not in the public target set rather
// than being an accepted-but-empty back end.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target slang -entry main -stage compute
//CHECK: unknown code generation target 'slang'
// CHECK: E00013

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}
