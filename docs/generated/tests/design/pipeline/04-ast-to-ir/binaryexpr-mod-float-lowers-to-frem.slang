//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=84a14c8c06fb62977462b222065d70ef81479ad93846db3d8c9e968027e956d2
//META: purpose=A builtin `a % b` on `float` operands picks the floating-point remainder opcode `frem` — the element-type selection for `%` (IRFRem for floating-point operands).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one operator where the fast path consults the element
// type: `%` on floating-point operands must emit `frem`, not `irem`.
// Sibling of the integer test that pins `irem` for the "otherwise"
// side of the same selection.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform float b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a % b;
}

// CHECK-LABEL: func %main
// CHECK: frem(%{{.*}}, %{{.*}})
// CHECK-NOT: irem(
