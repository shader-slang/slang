//TEST:SIMPLE(filecheck=METAL): -target metal -stage fragment -entry main
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage fragment -entry main

// Test basic SubpassInput with three float4 inputs at different attachment indices.

// METAL-DAG: color(0)
// METAL-DAG: color(1)
// METAL-DAG: color(5)
// METAL-NOT: SubpassInput
// METALLIB: framebuffer_fetch

[[vk::input_attachment_index(0)]] SubpassInput<float4> inputColor;
[[vk::input_attachment_index(1)]] SubpassInput<float4> inputDepth;
[[vk::input_attachment_index(5)]] SubpassInput<float4> inputHigh;

[shader("fragment")]
float4 main() : SV_Target
{
    float4 color = inputColor.SubpassLoad();
    float4 depth = inputDepth.SubpassLoad();
    float4 high = inputHigh.SubpassLoad();
    return color + depth + high;
}
