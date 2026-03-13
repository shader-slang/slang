//TEST:SIMPLE(filecheck=CHECK): -target spirv

struct VertexOutput
{
    float color;
}

[shader("fragment")]
float4 fsmain(VertexOutput vout) : SV_Target
{
    // CHECK: error{{.*}}39027
    return GetAttributeAtVertex(vout.color, 0); // error: color must be decorated with `nointerpolation`.
}
