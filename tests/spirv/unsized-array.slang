//TEST:SIMPLE(filecheck=VK): -stage compute -entry computeMain -target spirv-asm
//VK: OpDecorate %_runtimearr_int ArrayStride 4
//VK: %_runtimearr_int = OpTypeRuntimeArray %int
//VK: OpArrayLength %uint %{{.*}} 3

//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST_INPUT:ubuffer(data=[9 9 9 1 2 3 4 0x100 0x999], stride=4):out,name outputBuffer
//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name ssbo2

//TEST:SIMPLE(filecheck=GLSL): -target glsl
//GLSL: buffer OutBufTy
//GLSL: outputBuffer_0.ua_0.length()

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain
//HLSL: error 36107: entrypoint 'computeMain' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
//HLSL: note: see using of 'getTrailingElementCount'

// RW/StructuredBuffer adds an implicit unsized array, which turns into nested unsized array,
// and breaks VK validation, so use GLSLShaderStorageBuffer<> here.
struct OutBufTy
{
    int a;
    int b;
    int c;
    int ua[];
};
GLSLShaderStorageBuffer<OutBufTy> outputBuffer;

struct SSBOTy2
{
    int fa[4];
}
GLSLShaderStorageBuffer<SSBOTy2> ssbo2;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.a = outputBuffer.getTrailingElementCount();
    outputBuffer.b = ssbo2.getTrailingElementCount();
    outputBuffer.c = ssbo2.fa.getCount();
    outputBuffer.ua[dispatchThreadID.x] = dispatchThreadID.x + outputBuffer.ua[4];
}
