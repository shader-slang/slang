//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=1690bc07bc351f6d57ef0cbfbdf90dd7a3a711d8e434d872c178e166712d1f85
//META: purpose=A bare discarded constructor call is deliberately excluded from the `[NoDiscard]` check, so only the discarded ordinary call in the same body is reported (E30059).
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out in the discarded-result check by putting both shapes
// in one body: a `[NoDiscard]` `__init` invoked as a bare statement and a
// `[NoDiscard]` free function invoked the same way. The directive runs in
// exhaustive mode, so the single annotated E30059 must account for every
// diagnostic the compiler emits — which it can only do if the constructor call
// on the preceding line produced none.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

struct S
{
    int v;
    [NoDiscard]
    __init(int x) { v = x; }
}

[NoDiscard]
int plain(int x)
{
    return x + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<int> buf)
{
    S(3);
    plain(1);
//CHECK: E30059
//CHECK: the result of calling 'plain' is discarded
    buf[0] = 0;
}
