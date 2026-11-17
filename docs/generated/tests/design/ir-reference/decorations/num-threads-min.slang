//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=dba27f1411ac31618a8d7cd73ed506a70bd92102a42f2b59e56aa750058110fa
//META: purpose=Boundary - the smallest legal numthreads(1,1,1) lowers to numThreads(1 : Int, 1 : Int, 1 : Int) with all three operands at 1.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the workgroup size. All three operands are the
// minimum legal value at once, which is the shape most likely to be
// special-cased or elided; the CHECK asserts the decoration is emitted with
// three explicit `1` operands.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [numThreads(1 : Int, 1 : Int, 1 : Int)]
// CHECK: func %main
