//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage geometry -entry geometryMain
//CHECK-NOT: error

#define MAX_POINT_LIGHTS 4

static const int instances = 3;

struct GSOutput {
    float4 position : SV_Position;
    uint   layer    : SV_RenderTargetArrayIndex;
};

struct GSInput { float4 position : SV_Position; };

[shader("geometry")]
[maxvertexcount((MAX_POINT_LIGHTS * 6 + 1) * 3)]
[instance(instances)]
void geometryMain(triangle GSInput input[3], inout TriangleStream<GSOutput> outStream) {}