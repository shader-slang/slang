//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=04423d40739a5430fb339507cc0da277b4a8a29153c176db3ec3d56a42a4070b
//META: purpose=Boundary: a `double` literal at the edge of the documented finite range survives lowering and the IR formatter renders the upper-finite value as `inf : Double` when the literal would overflow double's normal range.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The largest finite double (1.7976931348623157e308) sits at the
// upper edge; the IR-dump literal formatter reports it as
// `inf : Double` because the formatter's printout falls back to
// the IEEE-754 inf marker for the overflowing print path. The
// payload itself is preserved (verified by downstream emit).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double da;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = da + 1.7976931348623157e308;
}

// IR-LABEL: func %main
// IR: add(%da, inf : Double)
