//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes
//META: doc_section_digest=b2bba5b4923f9f3b4fa76f2dbfcef4a05d4c5bbf61e50737eaabe8565887e46f
//META: purpose=Phase C's per-function eliminateDeadCode (C5) sweeps unused intra-function instructions; an unread `int dead = a * 999;` is removed before the LOWER-TO-IR dump.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An intra-function local that is computed but never read is
// stripped by C5's per-function DCE sweep. The multiplication
// constant (999) never appears in the LOWER-TO-IR block. C5
// runs with `keepExportsAlive` / `keepLayoutsAlive` set, so we
// observe the effect inside `main` rather than on module-scope
// helpers.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int dead = a * 999;
    buf[0] = 1;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK-NOT: 999 : Int
