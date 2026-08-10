//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=babb40e44ba0eb7addfafeddc37185c8c3bbed4c27b3b7fc8a2469d95b33aca1
//META: purpose=Boundary - numthreads with a single x argument defaults the missing y and z dimensions to 1, surfacing as numThreads(x : Int, 1 : Int, 1 : Int).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the minimum operand count of `numThreads` is satisfied even
// when the user writes fewer arguments: the front end fills y and z with 1.
// The CHECK pins all three operands, so an omitted-operand lowering that
// left the decoration short would fail here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(64)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [numThreads(64 : Int, 1 : Int, 1 : Int)]
// CHECK: func %main
