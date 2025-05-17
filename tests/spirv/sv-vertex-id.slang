//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry vertMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target spirv  -emit-spirv-via-glsl

// CHECK-DAG: %[[BASE_VERTEX:[0-9]+|[a-zA-Z_][a-zA-Z0-9_]*]] = OpVariable {{.*}} BuiltIn BaseVertex
// CHECK-DAG: %[[REG1:[0-9a-zA-Z_]+]] = OpLoad %int %[[BASE_VERTEX]]
// CHECK-DAG: %[[REG2:[0-9a-zA-Z_]+]] = OpLoad %int %gl_VertexIndex
// CHECK-DAG: OpISub %int %[[REG2]] %[[REG1]]

// GLSL: gl_VertexIndex - gl_BaseVertex

[shader("vertex")]
float4 vertMain(int i : SV_VertexID) : SV_Position
{
    return i;
}
