//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=CHECK): -O0 -target dxil -profile ps_6_0 -entry main -validate-ir
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target metal -entry main -stage fragment -validate-ir

// Resource-global replacement creates mutable local storage when control flow selects between two
// samplers. Neither DXIL nor Metal can represent that storage, so the late target check must
// diagnose the surviving local instead of passing it to either emitter.

Texture2D<float4> texture;
SamplerState firstSampler;
SamplerState secondSampler;
uniform bool selectSecondSampler;
static SamplerState selectedSampler = firstSampler;
//CHECK:            ^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//CHECK:            ^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'SamplerState'

[shader("fragment")]
float4 main() : SV_Target
{
    if (selectSecondSampler)
        selectedSampler = secondSampler;
    return texture.SampleLevel(selectedSampler, float2(0.0), 0.0);
}
