//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -dump-intermediates -dump-intermediate-prefix tests/spirv/get-vertex-attribute-

// CHECK: OpDecorate %vout_vertexID{{.*}} PerVertexKHR

struct VertexOutput
{
    nointerpolation int vertexID;
}

[shader("fragment")]
float4 fsmain(VertexOutput vout) : SV_Target
{
    return GetAttributeAtVertex(vout.vertexID, 0);
}
