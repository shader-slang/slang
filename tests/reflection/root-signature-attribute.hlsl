//TEST:REFLECTION:-profile ps_6_6 -target hlsl -no-codegen

// Tests that the [RootSignature] attribute on an HLSL-dialect entry point is
// recognized and reflected as a user attribute with its string argument.

[RootSignature("DescriptorTable(SRV(t0))")]
float4 main(): SV_Target
{
    return 0.0.xxxx;
}
