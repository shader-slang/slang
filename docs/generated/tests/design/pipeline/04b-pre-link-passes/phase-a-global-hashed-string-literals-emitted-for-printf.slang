//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission
//META: doc_section_digest=e1b449b7fcdd81d8efc3a0ebc0902561859fb0966734d156746b5be25c2fee00
//META: purpose=Phase A row A8 emits the kIROp_GlobalHashedStringLiterals aggregate that collects every hashed string literal observed during lowering.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `printf("hello!\n")` routes its format string through the
// hashed-string-literal table. Phase A row A8 emits a single
// aggregate inst that holds every observed hashed string; the
// LOWER-TO-IR dump shows it as `global_hashed_string_literals(...)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("hello!\n");
    buf[0] = 1;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK: global_hashed_string_literals("hello!\n")
