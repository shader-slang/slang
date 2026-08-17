//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Boundary: same-width signed-to-unsigned `int -> uint` lowers to `intCast(%a)`, even though no bit-width change is involved — the `intCast` opcode covers the signedness-flip step.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Same-width but signedness-changing conversion. Verifies that lowering lumps
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
