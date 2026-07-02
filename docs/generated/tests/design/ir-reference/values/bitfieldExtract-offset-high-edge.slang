//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
//META: purpose=Boundary: `bitfieldExtract(a, 31, 1)` (top-bit extract on a 32-bit `uint`) lowers to `bitfieldExtract(%a, 31 : UInt, 1 : UInt)` — the high-edge of the offset axis.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The high-edge of the offset axis: offset = width - 1, count = 1.
// This is the canonical "extract the sign bit" pattern on a
// 32-bit unsigned value.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bitfieldExtract(a, 31, 1);
}

// IR-LABEL: func %main
// IR: bitfieldExtract(%a, 31 : UInt, 1 : UInt)
