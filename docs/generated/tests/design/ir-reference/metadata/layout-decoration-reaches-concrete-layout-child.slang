//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:09:48Z
//META: source_commit=f39188078fe010048714735213c24a7cb15532b9
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout
//META: doc_section_digest=603eb96e45583a4aaf05d861ab8c093cb01e3cf0104ce403f82a3847e5c5a8bd
//META: purpose=Following a [layout(...)] decoration's operand reaches a concrete Layout child definition (varLayout / typeLayout / EntryPointLayout) that holds the offset/size/binding data.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the layout decoration is the link from a laid-out inst to
// its layout record: the id inside a `[layout(%N)]` decoration must be
// defined by one of the concrete Layout children, never by the abstract
// parent. The test captures the referenced id from a decoration in the
// dump and asserts its definition is a varLayout / EntryPointLayout /
// typeLayout inst.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [layout(%[[L:[0-9]+]])]
// CHECK: %[[L]]{{[^0-9].*}}= {{(varLayout|EntryPointLayout|typeLayout)}}(
