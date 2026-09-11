//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=3e3c1fa508130c56014c50d7c74ee56f2e557c277dfbc86c712dbb9373bf794a
//META: purpose=Matrix and vector parameters of a [CudaKernel] function are flattened into plain storage structs, a float2x2 becoming _MatrixStorage_float2x2 (_ColMajor suffixed under column-major layout) and a float3 becoming _VectorStorage_float3_.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the storage-struct names the CUDA builtin-type lowering
// produces, and the layout branch the row calls out. The kernel takes one
// matrix and one vector parameter; both must reach the emitted signature
// as storage structs rather than as native CUDA vector / matrix types.
// The two directives differ only in matrix layout, which is what the
// `_ColMajor` suffix is keyed to, so together they pin both spellings the
// row names. The generated unpack functions are checked too, since the row
// says they are called at the kernel boundary.

//TEST:SIMPLE(filecheck=COL):-target cuda -matrix-layout-column-major
//TEST:SIMPLE(filecheck=ROW):-target cuda -matrix-layout-row-major

[CudaKernel]
void kern(float2x2 m, float3 v, RWStructuredBuffer<float> outBuf)
{
    outBuf[0] = m[0][0] + v.x;
}

// COL-DAG: struct _MatrixStorage_float2x2_ColMajor_{{[0-9]+}}
// COL-DAG: struct _VectorStorage_float3_{{[0-9]+}}
// COL-DAG: unpackStorage_{{[0-9]+}}(_MatrixStorage_float2x2_ColMajor_{{[0-9]+}}
// COL-DAG: unpackVector_{{[0-9]+}}(_VectorStorage_float3_{{[0-9]+}}

// Row-major layout drops the suffix from the matrix storage struct.
// ROW-DAG: struct _MatrixStorage_float2x2_{{[0-9]+}}
// ROW-DAG: struct _VectorStorage_float3_{{[0-9]+}}
// ROW-NOT: _ColMajor
