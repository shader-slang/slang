// -emit-spirv-directly is required: a dynamically-indexed bindless resource-heap read does not
// lower to valid GLSL (no GL_EXT_nonuniform_qualifier), so the via-glsl cross-check cannot build it.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly
//TEST:REFLECTION(filecheck=REFLECTION):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HEAP): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

// A bindless handle bound to a real buffer. Reading it keeps the bindless resource heap live
// (this is the bindless use the test exercises) while staying valid — reading an *unbound*
// bindless descriptor is UB under Vulkan (shader-slang/slang#12797). A buffer handle is used
// rather than a texture/sampler to avoid the lavapipe image-descriptor bug.
//TEST_INPUT:ubuffer(data=[10.0 20.0 30.0 40.0], stride=4):name=bindless
uniform RWStructuredBuffer<float>.Handle bindless;

//CHK:11.0
//CHK-NEXT:22.0
// REFLECTION pins the frontend reservation; HEAP pins the lowered heap descriptor set. With no
// parameter blocks the heap lands right after gInput/gOutput (space 0), so both are set 1.
//REFLECTION: "bindlessSpaceIndex": 1
//HEAP: OpDecorate %__slang_resource_heap{{.*}}DescriptorSet 1

[Shader("compute")]
[NumThreads(2, 2, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    gOutput[tid] = gInput[tid] + bindless[tid];
}
