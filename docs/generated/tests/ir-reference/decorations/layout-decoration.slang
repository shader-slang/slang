//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#layout-layoutdecoration
//META: doc_section_digest=31fd6c29642b2014d16781ceb9cd5a2070d0dd3d7233c66a65799e59f1cdeb9c
//META: purpose=The layout decoration attaches a Layout opcode reference to a global parameter; the operand is an inst-reference to a Layout instruction.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc: "layout attaches a Layout opcode (documented in
// metadata.md) to a parameter, type, or variable. The Layout itself
// carries the offset / size / register binding information; layout
// is the link." The dump prints [layout(%N)] where %N is an
// unstable inst-reference ID; the test wildcards the reference.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [layout(%{{[0-9]+}})]
