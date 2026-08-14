//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=babb40e44ba0eb7addfafeddc37185c8c3bbed4c27b3b7fc8a2469d95b33aca1
//META: purpose=A numthreads(x,y,z) attribute lowers to a numThreads decoration with three IRIntLit operands carrying the workgroup dimensions in source order.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand count and ordering of `numThreads`. Three distinct
// values (8, 4, 2) are used so a transposed or reordered lowering cannot
// accidentally match; the CHECK pins the exact triple, then `func %main` to
// show it decorates the entry point.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [numThreads(8 : Int, 4 : Int, 2 : Int)]
// CHECK: func %main
