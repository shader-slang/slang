//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#resource-type-spellings
//META: doc_section_digest=09c255c9e2a2d27e0f91041ce7e96a567874c012eee1581f14bdc2a113ac3c2e
//META: purpose=A `Buffer<T>` is a texture-kind resource on Metal: it emits as `texture_buffer<T, access::read>` and takes a `[[texture(N)]]` slot rather than a buffer slot.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the row of the resource table most likely to surprise a reader:
// despite its name a `Buffer<T>` does not become a device pointer, it becomes
// a texture-buffer object in the texture slot space. Declaring it alongside a
// structured buffer makes the two slot spaces visible at once -- the output
// buffer takes buffer(0) while the texture buffer takes texture(0).

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Buffer<float4> tbuf;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tbuf[tid.x];
}

// CHECK: texture_buffer<float, access::read> tbuf_
// CHECK: texture_buffer<float, access::read> tbuf_{{[0-9]+}} {{\[\[}}texture(0){{\]\]}}
