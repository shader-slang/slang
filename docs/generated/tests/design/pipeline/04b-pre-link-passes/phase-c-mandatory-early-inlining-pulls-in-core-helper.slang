//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#performmandatoryearlyinlining-and-the-surrounding-loop
//META: doc_section_digest=3e4ecc0dde41d070a716078d43988f337d7fe402d8628557a72dc1a19f293965
//META: purpose=Phase B's prelinkIR (B1) pulls in `[unsafeForceInlineEarly]` core-module bodies; the mandatory-early-inlining loop in Phase C then inlines them, so neither the core operator name nor a forwarding call survives.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `prelinkIR` imports `[unsafeForceInlineEarly]`
// bodies so the mandatory-early-inlining fixed-point loop can
// inline them. Core arithmetic operators on plain Int are marked
// `[__unsafeForceInlineEarly]`; after Phase C the operator-level
// helper is gone and the LOWER-TO-IR dump shows the operator's
// expanded body (a raw `add` IR opcode) at the call site rather
// than a `call %operator{plus}(...)`.

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
