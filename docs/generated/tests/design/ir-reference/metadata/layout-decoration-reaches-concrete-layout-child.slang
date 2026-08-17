//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout
//META: doc_section_digest=8611701da03ed9b5dca8322f8af4809da1b1c134d53509c9de4451b865f1b242
//META: purpose=A laid-out instruction is connected to its layout by a layout decoration whose operand is a concrete Layout child.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented navigation step from a laid-out inst to its layout
// data. The CHECK captures the id referenced by a `[layout(...)]` decoration
// and then requires that id's defining instruction to be one of the concrete
// layout opcodes -- which is what makes "follow the decoration to reach the
// offset/size data" a valid rule for a reader walking the IR.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [layout(%[[L:[0-9]+]])]
// CHECK: %[[L]]{{[^0-9].*}}= {{(varLayout|EntryPointLayout|typeLayout|structTypeLayout)}}
