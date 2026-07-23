// Non-scalar `fwidth` must emit for a Metal fragment shader. See issue #12165.
// METAL checks the emitted MSL text; METALLIB confirms the Metal compiler accepts it.
//TEST:SIMPLE(filecheck=METAL): -stage fragment -target metal -entry fragMain
//TEST:SIMPLE(filecheck=METALLIB): -stage fragment -target metallib -entry fragMain

struct FSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

// METALLIB: define {{.*}} @fragMain
float4 fragMain(FSInput input) : SV_Target
{
    // METAL: fwidth(
    float2 vw = fwidth(input.uv);

    float2x2 m = float2x2(input.uv.x, input.uv.y, input.uv.y, input.uv.x);
    // METAL: fwidth(
    float2x2 mw = fwidth(m);

    return float4(vw, mw[0][0], mw[1][1]);
}
