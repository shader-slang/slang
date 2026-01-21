//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage vertex -g3 -O0 -emit-spirv-directly

// Test that DebugScope instructions are followed by DebugLine instructions
// This ensures that SPIR-V blocks defined by DebugScope have valid debug line info

struct VSInput {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VSOutput {
    float3 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR;
};

uniform float3 cameraPosition;

// Helper function that returns bool - this tends to create DebugScope blocks
bool IsVertexVisible(float3 worldPos, float3 cameraPos) {
    float distance = length(worldPos - cameraPos);
    return distance < 1000.0 && distance > 0.1;
}

// Another helper function returning bool2
bool2 CheckVertexBounds(float2 texCoord) {
    return bool2(texCoord.x >= 0.0 && texCoord.x <= 1.0,
                 texCoord.y >= 0.0 && texCoord.y <= 1.0);
}

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = float3(0);
    output.texCoord = float2(0);
    output.color = float4(0);

    if (!IsVertexVisible(input.position, cameraPosition)) {
        return output;
    }

    if (!all(CheckVertexBounds(input.texCoord))) {
        return output;
    }

    if (input.position.x < 0.0f) {
        input.position.x *= -1.0f;
    }

    output.color = float4(1);
    output.texCoord = input.texCoord;
    output.position = input.position;
    return output;
}

// CHECK: DebugScope
// CHECK: DebugLine
// CHECK: DebugScope
// CHECK: DebugLine
