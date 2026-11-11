//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=6399b7c2a0c9dbf90340a8eb7cb86baa3cd9c07289fb6d93436a49b19618df81
//META: purpose=A matrix-typed value uses the `Mat` IR type with element-type, row-count, column-count and layout operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `Mat`/`MatrixType` row with operands
// `(elementType, rowCount, columnCount, layout)`. A `float4x4` value
// read from a buffer appears as a `Mat(Float, 4 : Int, 4 : Int, ...)`
// type in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform StructuredBuffer<float4x4> in_buf;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4x4 m = in_buf[0];
    buf[0] = m[0][0];
}

// IR-LABEL: func %main
// IR: Mat(Float, 4 : Int, 4 : Int
