//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage vertex -entry main

[Flags]
public enum VertexLayout : uint {
    position,
    color,
    uv
}

struct VertexInput {
    float3 position : POSITION_ATTR;
    uint color      : COLOR_ATTR;
    float2 uv       : UV_ATTR;
};

public struct VertexOutput {
    float4 position     : SV_Position;
    VertexLayout layout : TEXCOORD0;
}

[shader("vertex")]
VertexOutput main(in VertexInput input) {
    VertexOutput output;
    output.position = float4(0);
    output.layout = VertexLayout.position;
    return output;
}

//CHECK: layout_0 : TEXCOORD0