//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#layout--layoutdecoration
//META: doc_section_digest=9aea9e5f0576048132cb69783d0a9675d51a7a43153f52ed20527c7dae4045f3
//META: purpose=The layout decoration attaches a Layout opcode reference to a global parameter; the operand is an inst-reference to a Layout instruction.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `layout` is a *link* rather than a container: its operand
// is an inst reference to a separate `Layout` opcode that holds the
// offset/size/register data. The CHECK matches `[layout(%N)]` with the
// reference id wildcarded, because inst numbering is assigned by the dump
// printer and is not stable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [layout(%{{[0-9]+}})]
