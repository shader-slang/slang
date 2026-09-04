//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl-intrinsic/raytracing/glsl-rayAnyhit.slang -stage anyhit -entry main -target glsl -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK_SPV): -lang glsl tests/glsl-intrinsic/raytracing/glsl-rayAnyhit.slang -emit-spirv-directly -stage anyhit -entry main -target spirv-assembly -Wno-118

layout(binding = 0) uniform accelerationStructureEXT as;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    uint data[];
} outputBuffer;

bool testVars() {
    return true
// CHECK_GLSL-DAG: gl_HitTEXT
// CHECK_SPV-DAG: RayTmax{{NV|KHR}}
        && gl_HitTEXT != 99.0f
// CHECK_GLSL-DAG: gl_HitKindEXT
// CHECK_SPV-DAG: HitKind{{NV|KHR}}
        && gl_HitKindEXT != 0xFFFFFFFF
        ;    
}

void main()
{
    outputBuffer.data[0] = true
        && testVars()
        ;
    //BUF: 1
}
