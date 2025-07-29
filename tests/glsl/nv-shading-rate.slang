//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage fragment

// Test NV_FragmentSize and NV_InvocationsPerPixel GLSL builtin mapping

struct FragmentInput
{
    int2 fragmentSize : NV_FragmentSize;
    int invocationsPerPixel : NV_InvocationsPerPixel;
}

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    // CHECK: #extension GL_NV_shading_rate_image : require
    // CHECK: flat in ivec2 gl_FragmentSizeNV
    // CHECK: flat in int gl_InvocationsPerPixelNV

    return float4(input.fragmentSize.x, input.fragmentSize.y, input.invocationsPerPixel, 1.0);
}