//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=f6737af21fc0ce62bf35ca601aa6d8bc483fabcafaedad874ca80c1323a37207
//META: purpose=Boundary: documented `uint` MAX (4294967295) survives lowering as `4294967295 : UInt` on the `add` operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Upper edge of the unsigned-32 range. The literal payload is
// expected to surface verbatim. Note that `a + 0u` is constant-
// folded away (additive identity) so only the MAX-edge form is
// asserted.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + 4294967295u;
}

// IR-LABEL: func %main
// IR: add(%a, 4294967295 : UInt)
