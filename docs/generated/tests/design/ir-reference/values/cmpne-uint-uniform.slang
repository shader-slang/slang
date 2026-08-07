//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=04ce765c6615b6d4afcb57668ffc53ea9c74c835ea05c4c4d58b93fe0cbf4541
//META: purpose=Expansion: comparing two `uint` (unsigned) values via `!=` lowers to `cmpNE(%a, %b)` — same opcode as signed-int and float operands.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Unsigned integer inequality. The doc admits one `cmpNE`
// opcode across signedness; signedness is encoded by operand type
// (which the result type reflects only as `Bool`).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform uint b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(a != b, 1, 0);
}

// IR-LABEL: func %main
// IR: cmpNE(%a, %b)
