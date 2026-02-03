// WGSL location indices should be generated for struct fields used as entry point IO,
// and should not be deduplicated if the struct is used as both vertex output and fragment input.
// The bug only occurs when doing whole-program compilation.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -whole-program

struct VOut
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

// The VOut struct used as vertex output must have @location attributes
// CHECK: struct VOut_0
// CHECK: @location({{[0-9]+}}) normal_0
// CHECK: @location({{[0-9]+}}) uv_0

[shader("vertex")]
VOut vertexMain(uint vertexID : SV_VertexID)
{
    VOut output;
    output.position = float4(0, 0, 0, 1);
    output.normal = float3(0, 1, 0);
    output.uv = float2(0, 0);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input) : SV_Target
{
    return float4(input.normal, 1.0);
}
