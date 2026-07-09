//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=bea4097f3cb99e6205f4e10d4bf9ab240dde03162a3388a24e9a78e55d5c0e65
//META: purpose=After the per-global-parameter loop, _lowerTypeLayoutCommon builds an IRStructTypeLayout for the global scope; with no cbuffer wrapping, the merged IR shows a top-level structTypeLayout(...) for the global struct (no parameterGroupTypeLayout).
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says:
//   auto irGlobalStructTypeLayout =
//       _lowerTypeLayoutCommon(&globalStructTypeLayoutBuilder,
//                              globalStructLayout);
// When the global scope is NOT wrapped in a parameter group, the
// module's global-scope type layout is a plain
// structTypeLayout(...) and no parameterGroupTypeLayout appears.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: structTypeLayout(
// CHECK-LABEL: func %main
