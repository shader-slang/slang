//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metals-three-lowerbufferelementtypetostoragetype-invocations
//META: doc_section_digest=836e9573185f6aff1abb3c7758ad765b45c7765530128f9d01ed780be2599b0d
//META: purpose=Boundary: the smallest square matrix (float2x2) in a device buffer is also lowered to packed-vector storage, emitted as MSL `packed_float2`.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the packed-vector storage lowering at the minimum matrix
// dimension: `packed_T<N>` generalizes across column counts, so a
// float2x2 device-buffer field must surface as `packed_float2` in the
// emitted MSL. The kernel reads one matrix element through the unpack
// path so the field cannot be dead-code-eliminated.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Tiny
{
    float2x2 m;
}

RWStructuredBuffer<Tiny> tiles;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 v = mul(tiles[tid.x].m, float2(1.0, -1.0));
    outBuf[tid.x] = v.x + v.y;
}

// CHECK: packed_float2
