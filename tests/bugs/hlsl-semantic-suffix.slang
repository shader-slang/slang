//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry vertexMain -stage vertex

// Previously TEXCOORD1 would be emitted as TEXCOORD10 because the
// semantic index was appended to a name that already contained the index.

struct VSInput {
    float2 data1 : TEXCOORD0;
    float2 data2 : TEXCOORD1;
    float3 normal : NORMAL0;
    float3 tangent : NORMAL1;
};

struct VSOutput {
    float4 pos : SV_Position;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input) {
    VSOutput out;
    out.pos = float4(input.data1, input.data2);

    return out;
}

// Use semicolons for precise matching (no false substring matches).
// CHECK: : TEXCOORD0;
// CHECK: : TEXCOORD1;
// CHECK: : NORMAL0;
// CHECK: : NORMAL1;
