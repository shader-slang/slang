//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-global-parameter-steps
//META: doc_section_digest=ceb5a8d78703ef1bd82fb8e6b05d1b3d966794012fcf0910b1c8110ed0f27829
//META: purpose=Each per-global-parameter step in createIRModuleForLayout attaches an IRLayoutDecoration to the stub IRGlobalVar; observable as a [layout(%N)] on the global_param.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's per-global-parameter table row 3 says:
// "Attach IRLayoutDecoration via builder->addLayoutDecoration(irVar, irLayout)."
// After the layout-IR module is merged into the post-link IR,
// the stub global_param for `buf` carries a [layout(%N)] decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: [layout(%{{[0-9]+}})]
// CHECK: global_param
