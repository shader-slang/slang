//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#imagegatheroffset
//META: doc_section_digest=0e987db459d6585b402818e888f7feb042e67c1424ead62468638b9bd3c00973
//META: purpose=`Texture2D::GatherRed(sampler, uv, offset)` reaches the IR as `imageGatherOffset(sampledImage, location, component, offset)` inside the core-module gather body pulled into the dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the four-operand shape of the newest opcode in the image
// family. The gather is called with a whole-texel `int2` offset, and the
// CHECK matches the opcode globally rather than under `func %main`
// because the `__intrinsic_op` sits in the library `GatherRed` body that
// the user call specializes; the operand names in that body (`location`,
// `component`, `offset`) are what the CHECK pins.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture2D<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = float2(tid.xy) * 0.01;
    rwbuf[0] = tex.GatherRed(samp, uv, int2(1, 1));
}

// IR: imageGatherOffset(%{{[A-Za-z0-9_]+}}, %location, %component, %offset)
