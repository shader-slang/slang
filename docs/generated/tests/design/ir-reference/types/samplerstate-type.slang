//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=A uniform SamplerState declaration lowers to a global_param of IR type SamplerState.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A uniform SamplerState declaration lowers to a global_param of IR
// type SamplerState. A `uniform SamplerState samp;` surfaces
// `SamplerState` as the IR type on the `global_param`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform SamplerState samp;
uniform Texture2D<float4> tex;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// IR: let {{.*}}%samp{{.*}}: SamplerState{{.*}}= global_param
