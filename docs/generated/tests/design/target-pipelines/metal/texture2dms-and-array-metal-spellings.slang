//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#resource-type-spellings
//META: doc_section_digest=7b7f20ff0637d9f1a655a69590f780c00b292d49ad138092b4ca9bd4ebed1ccb
//META: purpose=A multisampled texture takes the `_ms` stem suffix and a multisampled array takes `_ms_array` in that order, both emitted with `access::read`.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the suffix-composition rule for Metal texture spellings, which is
// the one row of the resource table whose emitted name is assembled rather
// than looked up. Declaring both a multisampled texture and a multisampled
// array in one kernel makes the ordering observable: the array suffix has to
// follow the multisample suffix, so a test that only checked for the two
// substrings separately would not distinguish the two orders.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture2DMS<float4> ms;
Texture2DMSArray<float4> msArray;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = ms.Load(int2(tid.xy), 0) + msArray.Load(int3(tid), 0);
}

// CHECK-DAG: texture2d_ms<float, access::read> ms_
// CHECK-DAG: texture2d_ms_array<float, access::read> msArray_
