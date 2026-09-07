//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metals-three-lowerbufferelementtypetostoragetype-invocations
//META: doc_section_digest=41531393ced08c0803ee8c63b01f36a7cafa65309b49df24964c1309a93f4b8e
//META: purpose=Boundary: the smallest square matrix (`float2x2`) in a device buffer also lowers to packed-vector storage, emitted as MSL `packed_float2`.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the packed-vector policy at the minimum column count to confirm
// `packed_T<N>` generalizes rather than being special-cased for
// three-component rows. One matrix element is read through the unpack path
// so the field cannot be dead-code-eliminated; CHECK asserts the storage
// type is `packed_float2`.

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
