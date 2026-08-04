//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=Boundary: same-width signed-to-unsigned `int -> uint` lowers to `intCast(%a)`, even though no bit-width change is involved — the `intCast` opcode covers the signedness-flip step.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Same-width but signedness-changing conversion. The doc lumps
// signedness flips under `intCast`; this probe verifies that even
// the zero-bit-width-change case still surfaces the opcode (the
// IR uses the result type to encode the new signedness).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(a);
}

// IR-LABEL: func %main
// IR: intCast(%a)
