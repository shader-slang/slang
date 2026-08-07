//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=bea4097f3cb99e6205f4e10d4bf9ab240dde03162a3388a24e9a78e55d5c0e65
//META: purpose=createIRModuleForLayout calls addLayoutDecoration on the module instance with irGlobalScopeVarLayout; the merged IR shows a top-level `varLayout(...)` instruction (not bound to `let`).
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Global-scope type layout" section says:
//   builder->addLayoutDecoration(irModule->getModuleInst(),
//                                irGlobalScopeVarLayout);
// That decoration appears in the merged IR as a standalone
// `varLayout(%N)` line at the top of the layout block (a top-level
// instruction, not the right-hand side of a `let`).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// The module-instance varLayout is a top-level statement, no `let` binding.
// CHECK: {{^}}varLayout(
