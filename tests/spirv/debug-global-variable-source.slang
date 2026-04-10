//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly

// Test that DebugGlobalVariable and DebugCompilationUnit reference the correct source file
// (this main file) rather than an included header file.

#include "shader-utils.slang"

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
}

[shader("fragment")]
float4 main(VertexOutput input) : SV_Target
{
    float2 uv = input.texCoord;
    float distanceFromCenter = calculateDistanceFromCenter(uv);
    float radialFade = createRadialFade(uv, 0.7);
    float4 color = float4(
        uv.x * (1.0 - distanceFromCenter * 0.5),
        uv.y * radialFade,
        uv.x * uv.y,
        1.0
    );
    return color;
}

// Verify that DebugSource for the main file is defined
// CHECK-DAG: [[MAIN_FILE:%[0-9]+]] = OpString "{{.*}}debug-global-variable-source.slang"
// CHECK-DAG: [[SOURCE_MAIN:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[MAIN_FILE]] %{{[0-9]+}}

// Verify that DebugCompilationUnit references the main file source, not the included header
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_MAIN]] %uint_{{[0-9]+}}

// Verify that DebugGlobalVariable for entry point parameters references the main file source
// The key fix: DebugGlobalVariable should reference the main file source, not the included file source
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugGlobalVariable {{%[0-9]+}} {{%[0-9]+}} [[SOURCE_MAIN]] %uint_0 %uint_0
