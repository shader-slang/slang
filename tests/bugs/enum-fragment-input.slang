//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage fragment -entry main

public enum VertexLayout : uint {
    position = 0,
    color = 1,
    uv = 2
}

struct FragmentInput {
    float4 position : SV_Position;
    VertexLayout layout : TEXCOORD0;
};

struct FragmentOutput {
    float4 color : SV_Target;
};

[shader("fragment")]
FragmentOutput main(in FragmentInput input) {
    FragmentOutput output;
    output.color = float4(float(input.layout), 0, 0, 1);
    return output;
}

//CHECK: main