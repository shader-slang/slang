//TEST:SIMPLE(filecheck=VK): -stage compute -entry computeMain -target spirv-asm
//VK: OpDecorate %_runtimearr_int ArrayStride 4
//VK: %_runtimearr_int = OpTypeRuntimeArray %int
//VK: OpArrayLength %uint %{{.*}} 1

//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST_INPUT:ubuffer(data=[0 1 2 3 4 0x100 0x999], stride=4):out,name outputBuffer

//TEST:SIMPLE(filecheck=GLSL): -target glsl
//GLSL: buffer OutBufTy
//GLSL: outputBuffer_0.ua_0.length()


// RW/StructuredBuffer adds an implicit unsized array, which turns into nested unsized array,
// and breaks VK validation, so use GLSLShaderStorageBuffer<> here.
struct OutBufTy
{
    int pad;
    int ua[];
};
GLSLShaderStorageBuffer<OutBufTy> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.pad = outputBuffer.ua.getCount();
    outputBuffer.ua[dispatchThreadID.x] = dispatchThreadID.x + outputBuffer.ua[4];
}
