//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#resource-type-spellings
//META: doc_section_digest=09c255c9e2a2d27e0f91041ce7e96a567874c012eee1581f14bdc2a113ac3c2e
//META: purpose=A write-only texture access mode gives `access::write` rather than the `access::read_write` a read-write texture gets, and its store lowers to `.write(...)`.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the access-mode branch of the texture spelling rule at the mode
// the read-write tests do not reach. A `WTexture2D` carries the write-only
// access mode all the way into the MSL type argument, and the subscript
// store still legalizes into Metal's write method, so the CHECK pins both
// the type spelling and the access form that depends on it.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

WTexture2D<float4> wtex;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    wtex.Store(int2(tid.xy), float4(1, 0, 0, 1));
}

// CHECK: texture2d<float, access::write> wtex_
// CHECK: wtex_{{[0-9]+}}.write(
