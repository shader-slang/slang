//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-global-parameter-steps
//META: doc_section_digest=985e0bab98cb696d50c51a3509833524406afcf6cfdcb16668a11fd5c51bd2d9
//META: purpose=The per-global-parameter loop runs for each varLayout in globalStructLayout.fields; with two globals, the IRStructTypeLayout records two structFieldLayout entries and both stubs carry [layout(...)] decorations.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "Record in the global type-layout builder via
// globalStructTypeLayoutBuilder.addField(irVar, irLayout) ...
// feeds the module-level IRStructTypeLayout built right after
// the loop via _lowerTypeLayoutCommon." Two distinct globals
// produce two distinct structFieldLayout(...) entries.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> bufA;
uniform RWStructuredBuffer<float> bufB;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bufA[0] = int(tid.x);
    bufB[0] = float(tid.y);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: structFieldLayout(%buf{{[A-Z]}}
// CHECK: structFieldLayout(%buf{{[A-Z]}}
