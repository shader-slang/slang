//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#entry-point-scoped-decorations
//META: doc_section_digest=4446f3fa5c2a1f5b2c654f20eebcde94c440a46d135e500a5e286f1a9bc6489f
//META: purpose=The `spvShader64BitIndexingEXT` requirement of a callee is lifted onto the entry point as a shader-64-bit-indexing decoration, not left on the attributed function.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies where the decoration lands when the capability is required by
// something other than the entry point. `helper` carries the `[require(...)]`,
// but the corresponding SPIR-V execution mode is entry-point scoped, so entry
// point lowering scans the inferred requirement atom sets and attaches the
// decoration to `main`. The CHECKs walk the dump in emission order: `helper` is
// lowered first with no such decoration ahead of it, and the decoration then
// appears in the block of decorations directly above `func %main`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main -capability spvShader64BitIndexingEXT

uniform RWStructuredBuffer<int> buf;

[require(spvShader64BitIndexingEXT)]
int helper(int n)
{
    return n * 2;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(int(tid.x));
}

// CHECK: func %helper
// CHECK: [shader64BitIndexing]
// CHECK-NEXT: [entryPoint(
// CHECK: func %main
