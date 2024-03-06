//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -stage intersection -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV): -allow-glsl -emit-spirv-directly -stage intersection -entry main -target spirv-assembly

layout(binding = 0) uniform accelerationStructureEXT as;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    uint data[];
} outputBuffer;

//COM: CHECK_GLSL-DAG: hitAttributeEXT
//COM: CHECK_SPV-DAG: HitAttributeNV
hitAttributeEXT vec4 hitAttr;
// CHECK_GLSL-DAG: rayPayloadInEXT
// CHECK_SPV-DAG: IncomingRayPayloadNV
layout(location = 2) rayPayloadInEXT vec4 payload;
// CHECK_GLSL-DAG: hitObjectAttributeNV
// CHECK_SPV-DAG: HitObjectAttributeNV
layout(location = 2) hitObjectAttributeNV vec4 attrMain;
// CHECK_GLSL-DAG: callableDataInEXT
// CHECK_SPV-DAG: IncomingCallableDataNV
layout(location = 1) callableDataInEXT vec2 incall;

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
bool equals(matrix<T, N, M> lhs, matrix<T, N, M> rhs)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            if (
                lhs[i][j] 
                !=
                rhs[i][j]
                )
            {
                return false;
            }
        }
    }
    return true;
}

bool testVars() {
    hitAttr = vec4(1);
    payload = vec4(1);
    attrMain = vec4(1);
    incall = vec2(1);
    
    return true
// CHECK_GLSL-DAG: gl_PrimitiveID
// CHECK_SPV-DAG: PrimitiveId
        && gl_PrimitiveID != 10000
// CHECK_GLSL-DAG: gl_InstanceID
// CHECK_SPV-DAG: InstanceId
        && gl_InstanceID != 10000
// CHECK_GLSL-DAG: gl_InstanceCustomIndexEXT
// CHECK_SPV-DAG: InstanceCustomIndexNV
        && gl_InstanceCustomIndexEXT != 10000
// CHECK_GLSL-DAG: gl_GeometryIndexEXT
// CHECK_SPV-DAG: RayGeometryIndexKHR
        && gl_GeometryIndexEXT != 10000
// CHECK_GLSL-DAG: gl_ObjectRayOriginEXT
// CHECK_SPV-DAG: ObjectRayOriginNV
        && gl_ObjectRayOriginEXT != vec3(99)
// CHECK_GLSL-DAG: gl_ObjectRayDirectionEXT
// CHECK_SPV-DAG: ObjectRayDirectionNV
        && gl_ObjectRayDirectionEXT != vec3(99)
// CHECK_GLSL-DAG: gl_ObjectToWorldEXT
//COM: CHECK_SPV-DAG: ObjectToWorldNV
        && !equals(gl_ObjectToWorldEXT, mat4x3(0.0f))
// CHECK_GLSL-DAG: gl_ObjectToWorld3x4EXT
//COM: CHECK_SPV-DAG: ObjectToWorldNV
        && !equals(gl_ObjectToWorld3x4EXT, mat3x4(0.0f))
// CHECK_GLSL-DAG: gl_WorldToObjectEXT
// CHECK_SPV-DAG-N: WorldToObjectNV 
        && !equals(gl_WorldToObjectEXT, mat4x3(0.0f))
// CHECK_GLSL-DAG: gl_WorldToObject3x4EXT
// spirv checked with CHECK_SPV-DAG-N
        && !equals(gl_WorldToObject3x4EXT, mat3x4(0.0f))
        ;
}

bool testFunctions() {
    return true
// CHECK_GLSL-DAG: reportIntersectionEXT
// CHECK_SPV-DAG: OpReportIntersectionKHR
        && reportIntersectionEXT(99, 0);
}

void main()
{
    outputBuffer.data[0] = true
        && testVars()
        && testFunctions();
        ;
    //BUF: 1
}
