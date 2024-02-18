//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -stage raygeneration -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV): -allow-glsl -emit-spirv-directly -stage raygeneration -entry main -target spirv-assembly

// note about test: there is currently no framework to test multiple shaders compiling together
// as such some of the shader logic calls functions without actually useful data checking 
// (since the data & result would be based off of the auxillary shaders like ray-miss)

// this would be a 'ray miss' shader (gl_CurrentRayTimeNV is ray miss only)

layout(binding = 0) uniform accelerationStructureEXT as;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    uint data[];
} outputBuffer;


layout(location = 2) rayPayloadInEXT vec4 payload;
layout(location = 2) hitObjectAttributeNV vec4 attrMain;

void main()
{
    outputBuffer.data[0] = true
        && gl_CurrentRayTimeNV != 0.0f
        ;
    // CHECK_GLSL: main(
    // CHECK_SPV: OpEntryPoint
    //BUF: 1
}
