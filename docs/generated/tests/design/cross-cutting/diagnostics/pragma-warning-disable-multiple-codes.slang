//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=83632b726fecd52003cb9a007e201049bb33c66f077ab6265b07278009caca02
//META: purpose=Boundary: `#pragma warning(disable: N M)` with a whitespace-separated id list adjusts the warning state for every listed id at once.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The single-id form of `#pragma warning(disable: N)` is exercised
// by pragma-warning-disable-suppresses. Probe the boundary where
// the disable list carries multiple ids: only 41024 is reliably
// triggerable by short source, so we list 41024 alongside an
// unrelated id (30056) and assert the comma-operator warning is
// silenced.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

#pragma warning(disable: 41024 30056)

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = (1, 2);
    buf[0] = x;
}

// CHECK-NOT: 41024
// CHECK-NOT: comma operator
// CHECK: void main
