// subpass-input-reflection.slang

// Test that SubpassInput types are properly reflected in JSON output.
// This is a regression test for GitHub issue #10045 where SubpassInput
// caused a crash during reflection JSON emission on Linux.

//TEST:REFLECTION:-target spirv -no-codegen

[[vk::input_attachment_index(0)]] SubpassInput<float4> subpassinput_0;

[shader("fragment")]
float4 main() : SV_Target
{
    float4 color = subpassinput_0.SubpassLoad();
    return float4(color.rgb, 1);
}
