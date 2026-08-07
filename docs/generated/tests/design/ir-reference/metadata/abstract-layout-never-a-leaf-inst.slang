//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:09:48Z
//META: source_commit=f39188078fe010048714735213c24a7cb15532b9
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout
//META: doc_section_digest=603eb96e45583a4aaf05d861ab8c093cb01e3cf0104ce403f82a3847e5c5a8bd
//META: purpose=The abstract Layout parent never appears as a leaf instruction in the IR dump; only its concrete children are emitted.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the abstract parent of the layout family is never emitted as
// an instruction of its own. The module produces real layout records
// (the positive CHECK pins a concrete `varLayout` definition), while the
// CHECK-NOTs on both sides of it assert no line in the whole dump defines
// a bare `Layout` inst.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-NOT: = Layout(
// CHECK: = varLayout(
// CHECK-NOT: = Layout(
