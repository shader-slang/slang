// Test for issue #8912: GLSL emission of TextureCubeArray should require the
// GL_ARB_texture_cube_map_array extension.

//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main -stage fragment

TextureCubeArray texCubeArr;
SamplerState samp;

float4 main(float4 pos : SV_Position, float3 coord : TEXCOORD) : SV_Target
{
    return texCubeArr.Sample(samp, float4(coord, 0));
}

// CHECK: GL_ARB_texture_cube_map_array
