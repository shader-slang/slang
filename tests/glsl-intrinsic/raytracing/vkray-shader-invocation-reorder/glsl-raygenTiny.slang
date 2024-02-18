//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -stage raygeneration -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV): -allow-glsl -emit-spirv-directly -stage raygeneration -entry main -target spirv-assembly
layout(binding = 0) uniform accelerationStructureEXT as;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    uint data[];
} outputBuffer;

// CHECK_GLSL-DAG: rayPayloadEXT
// CHECK_SPV-DAG: RayPayloadNV
layout(location = 2) rayPayloadEXT vec4 payload;
// CHECK_GLSL-DAG: hitObjectAttributeNV
// CHECK_SPV-DAG: HitObjectAttributeNV
layout(location = 2) hitObjectAttributeNV vec4 attrMain;

bool testTraceRayMotion() {
// CHECK_GLSL-DAG: traceRayMotionNV
// CHECK_SPV-DAG: OpTraceRayMotionNV
    traceRayMotionNV(as, gl_RayFlagsNoneEXT, 0xff, 0, 0, 0, vec3(0.1, 0.1, 0.0), 0.01f, vec3(0, 0, 1), 1e4f, 0.0f, 2);
    attrMain = vec4(1); //ensure traceRayMotionNV does not get optimized out 
    return true &&
        attrMain == vec4(1);
        
}

bool testPayloadReadWrite() {
    payload = vec4(4);

    vec4 read = payload;

    return true
        && read != vec4(0)
        ;
}

bool testAttributeReadWrite() {
    attrMain = vec4(2);

    vec4 read = attrMain;

    return true
        && read != vec4(0)
        ;
}

// CHECK_GLSL-DAG: main
// CHECK_SPV-DAG: OpEntryPoint
void main()
{
    //only checking compile'ing currently, therefore just ensure the code will not optimize out
    outputBuffer.data[0] = true
        && testTraceRayMotion()
        && testPayloadReadWrite()
        && testAttributeReadWrite()
        ;

    //BUF: 1
}
