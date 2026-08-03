//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metals-three-lowerbufferelementtypetostoragetype-invocations
//META: doc_section_digest=836e9573185f6aff1abb3c7758ad765b45c7765530128f9d01ed780be2599b0d
//META: purpose=A load-modify-store of a device-buffer matrix goes through the pack/unpack conversion between the native matrix working form and the packed-vector storage form: both `matrix<float,...>` and `packed_float3` appear in the emitted MSL.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the conversion half of the packed-vector lowering: loads
// unpack the storage form into the native matrix working form and stores
// pack it back. The kernel loads a matrix from the buffer, mutates one
// element, and writes it back; the CHECKs assert that the emitted MSL
// contains both the working `matrix<float,...>` type and the
// `packed_float3` storage type, which only holds if the load/store paths
// convert between the two forms.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Cell
{
    float3x3 m;
}

RWStructuredBuffer<Cell> cells;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3x3 t = cells[tid.x].m;
    t[0][0] = t[0][0] + 1.0;
    cells[tid.x].m = t;
}

// CHECK-DAG: packed_float3
// CHECK-DAG: matrix<float,int(3),int(3)>
