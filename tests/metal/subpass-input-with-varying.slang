//TEST:SIMPLE(filecheck=METAL): -target metal -stage fragment -entry main
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage fragment -entry main

// Verify SubpassInput coexists with regular varying inputs after both
// legalizeSubpassInputsForMetal and legalizeEntryPointVaryingParamsForMetal run.

// METAL-DAG: stage_in
// METAL-DAG: color(0)
// METALLIB: framebuffer_fetch

struct FragInput { float4 pos : SV_Position; float2 uv : TEXCOORD0; };
[[vk::input_attachment_index(0)]] SubpassInput<float4> fbInput;

[shader("fragment")]
float4 main(FragInput input) : SV_Target
{
    return fbInput.SubpassLoad() * input.uv.x;
}
