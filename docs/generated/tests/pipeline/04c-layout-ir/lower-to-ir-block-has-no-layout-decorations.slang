//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#what-this-module-is-not
//META: doc_section_digest=c5f9bc15bef4a61077dd5dc0d9ab9b6fd48a5c67823464969cab691a0a05a86c
//META: purpose=The pre-link executable IR (### LOWER-TO-IR:) carries no [layout(...)] decorations; layout lives in a sibling module built by createIRModuleForLayout.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "What this module is not" section states that the
// executable IR module produced by generateIRForTranslationUnit
// is kept layout-free. A [layout(...)] decoration would only ever
// appear once the layout-IR module is merged in, which happens
// after the pre-link region and so does not appear in the
// `### LOWER-TO-IR:` block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK: func %main
// CHECK-NOT: [layout(
// CHECK-NOT: EntryPointLayout(
// CHECK-NOT: structTypeLayout(
// CHECK-LABEL: ###
