//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout-family
//META: doc_section_digest=c1ef039f2fd0378a7cac58b8bb8282b937d992edac625ada2b1761464ceb475b
//META: purpose=Layout records are hoistable, so two distinct layouts that need the same attribute value share one attribute instruction.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the deduplication the whole family relies on. The constant buffer's
// array field and matrix field both want byte-alignment 16, and the array and
// the matrix both occupy 64 bytes. The CHECKs capture the size and alignment
// operand ids off the `arrayTypeLayout` and then require the *same* two ids to
// appear as operands of the `matrixTypeLayout`, which can only happen if the
// attributes were deduplicated into single shared instructions.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

cbuffer CB
{
    float4x4 m;
    float arr[4];
}

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = m[0][0] + arr[tid.x % 4];
}

// CHECK: = arrayTypeLayout(%{{[0-9]+}}, %[[SZ:[0-9]+]], %[[AL:[0-9]+]])
// CHECK: = matrixTypeLayout({{[0-9]+}} : Int, %[[SZ]], %[[AL]])
// CHECK: %[[AL]]{{.*}}= TypeAlignment(16 : Int)
// CHECK: %[[SZ]]{{.*}}= size(8 : Int, 64 : Int)
