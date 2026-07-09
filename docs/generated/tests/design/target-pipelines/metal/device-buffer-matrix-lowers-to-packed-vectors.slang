//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metals-three-lowerbufferelementtypetostoragetype-invocations
//META: doc_section_digest=836e9573185f6aff1abb3c7758ad765b45c7765530128f9d01ed780be2599b0d
//META: purpose=The Phase-C `Metal`-policy lowerBufferElementTypeToStorageType lowers a float3x3 stored in a device buffer into an array of packed vectors, emitted as MSL `packed_float3`.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a matrix field inside a structured-buffer element struct
// is stored in Metal's natural scalar-aligned layout: the storage form is
// an array of packed row vectors. The kernel reads the matrix (so the
// unpack path is live) and the CHECK asserts the `packed_float3` storage
// type appears in the emitted MSL, which only the Metal buffer-element
// lowering policy produces.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Xform
{
    float3x3 m;
    float bias;
}

RWStructuredBuffer<Xform> xforms;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 v = mul(xforms[tid.x].m, float3(1.0, 2.0, 3.0));
    outBuf[tid.x] = v.x + xforms[tid.x].bias;
}

// CHECK: packed_float3
