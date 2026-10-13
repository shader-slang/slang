//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#warning-groups
//META: doc_section_digest=fe2a3c2e311ba8162e26f4c17f4a54e2d2b52665ed3d562b5445ade975ac7e04
//META: purpose=`Default` is the implicit group of every untagged warning and is always emitted, so enabling another group does not gate it out.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that turning on a named group leaves untagged warnings alone. 41024
// carries no group sentinel, so it belongs to Default; compiling with `-Wall`
// selects a different group entirely and CHECK asserts 41024 still fires, which
// would not hold if the mask gated Default warnings too.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -Wall

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = (1, 2);
    buf[0] = x;
}

// CHECK: E41024
// CHECK: comma operator used in expression
