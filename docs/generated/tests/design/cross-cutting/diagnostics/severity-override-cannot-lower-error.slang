//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=a2c5b6b64f38a8b7fd3692290be87fe1f26f4a31a5f12e076913b5c584acc7f3
//META: purpose=A per-id severity override may not lower a diagnostic that is already at `Error`: `-warnings-disable` on an error id is rejected with E31111.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the override step refuses to demote a diagnostic whose declared
// severity is already Error. 30015 is undefined-identifier, an error entry;
// requesting `-warnings-disable 30015` is rejected rather than silently applied.
// CHECK asserts the rejection is reported as E31111 and CHECK-NOT asserts the
// error was not, in fact, silenced into nothing by a partially-applied override.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -warnings-disable 30015

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = notDeclaredAnywhere;
}

// CHECK: E31111
// CHECK: unknown diagnostic
// CHECK-NOT: E41024
