//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#performmandatoryearlyinlining-and-the-surrounding-loop
//META: doc_section_digest=334e18bcc405858cc1b138ad3c309f182a118316cbcb940b8ace0438053c2b33
//META: purpose=The prelinkIR import of cross-module [unsafeForceInlineEarly] bodies plus the Phase C mandatory-early-inlining loop leaves the core arithmetic operator inlined rather than called.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the cooperation between the pre-link import step and the
// force-inline loop, which is the only user-visible consequence of either.
// `a + b` on plain ints resolves to a core-module operator that is marked
// force-inline-early; because its body was imported and then inlined, the
// snapshot shows the raw `add` opcode at the use site instead of a call to
// the operator function.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + b;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK: add(%a, %b)
