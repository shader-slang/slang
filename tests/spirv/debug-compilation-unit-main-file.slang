//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly

#include "shader-utils.slang"

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
}

[shader("fragment")]
float4 main(VertexOutput input) : SV_Target
{
    // Create a simple gradient pattern based on texture coordinates
    float2 uv = input.texCoord;

    // Call function from the included header file
    float distanceFromCenter = calculateDistanceFromCenter(uv);

    // Use another function from the header for a radial fade effect
    float radialFade = createRadialFade(uv, 0.7);

    // Create color pattern with both distance effects
    float4 color = float4(
        uv.x * (1.0 - distanceFromCenter * 0.5),  // Red with subtle distance fade
        uv.y * radialFade,                        // Green with smooth radial fade
        uv.x * uv.y,                              // Blue as product of coordinates
        1.0                                       // Full opacity
    );

    return color;
}

// Verify that DebugCompilationUnit references this main shader file
// CHECK: [[MAIN_FILE:%[0-9]+]] = OpString "{{.*}}debug-compilation-unit-main-file.slang"
// CHECK: [[SOURCE_ID:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[MAIN_FILE]] %{{[0-9]+}}
// CHECK: OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_ID]] %uint_{{[0-9]+}}
