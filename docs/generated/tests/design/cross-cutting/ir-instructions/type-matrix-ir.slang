//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=90ad3422172b6f924f66f6a3985bb274c504d3a3d2e99937b3e44e0b93ad4822
//META: purpose=A matrix-typed value uses the `Mat` IR type with element-type, row-count, column-count and layout operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `Mat`/`MatrixType` row with operands
// `(elementType, rowCount, columnCount, layout)`. A `float4x4` value
// read from a buffer appears as a `Mat(Float, 4 : Int, 4 : Int, ...)`
// type in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

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
