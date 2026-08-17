//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=e5426f9afa17becb9e7d33c23301014e082fad12bbf8aff1c4a169d90575556e
//META: purpose=A catalog entry is not a promise that some input reaches it: getCount on a non-array reports no-member-of-name-in-type E30027, never the expected-array-expression entry E30020.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The point of the claim is that an entry's name does not identify the
// construct that triggers it. `expected-array-expression` reads like the
// diagnostic for asking a non-array for its length, but the node it
// inspects has no parser production, so no source text provokes it. The
// user-facing spelling is the ordinary `getCount` method, and on a
// non-array the failure is an ordinary member-lookup failure instead.
// This test calls `getCount` on a struct and asserts E30027 fires while
// E30020 does not; the CHECK-NOT is the load-bearing half, since without
// it the test would pass on either code.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct NotAnArray
{
    int a;
};

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    NotAnArray s;
    s.a = int(tid.x);
    outBuf[tid.x] = s.getCount();
}

// CHECK-NOT: E30020
// CHECK: E30027
// CHECK-NOT: E30020
