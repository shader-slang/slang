//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=3e3c1fa508130c56014c50d7c74ee56f2e557c277dfbc86c712dbb9373bf794a
//META: purpose=The bit-field accessor pass fills in the getter / setter bodies with shift-and-mask arithmetic over the backing integer, so a bit-field read emits shifts and masks rather than a field access.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the emitted shape a bit-field turns into. The struct declares
// two named bit-fields; if the accessors were left as plain field access
// the emitted struct would still have two members and the read would name
// one of them. The CHECKs assert instead that a single backing integer
// member carries both, and that reading a field emits shift operators over
// it — the arithmetic the accessor bodies were filled in with.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

struct Flags
{
    uint a : 3;
    uint b : 5;
}

RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    Flags f;
    f.a = tid.x;
    f.b = tid.y;
    outBuf[0] = f.b;
}

// Both named bit-fields collapse into one backing integer member ...
// HLSL: uint {{[A-Za-z0-9_]*}}bit_field_backing_{{[0-9_]+}};
// ... and the read of `b` is shift arithmetic over it, not a field access.
// HLSL: outBuf_{{[0-9]+}}[int(0)] = (({{.*}}bit_field_backing{{.*}}) << int(24)) >> int(27);
// HLSL-NOT: f_{{[0-9]+}}.b_
