// -emit-spirv-directly is required: a dynamically-indexed bindless resource-heap read does not
// lower to valid GLSL (no GL_EXT_nonuniform_qualifier), so the via-glsl cross-check cannot build it.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly -Xslang -DREPRO
//TEST:REFLECTION(filecheck=REFLECTION):-target spirv-asm -stage compute -entry computeMain
//TEST:REFLECTION(filecheck=REFLECTION_REPRO):-target spirv-asm -stage compute -entry computeMain -DREPRO
//TEST:SIMPLE(filecheck=HEAP): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=HEAP_REPRO): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly -DREPRO

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

// A bound bindless handle whose read keeps the resource heap live, so its lowered descriptor-set
// placement is actually exercised (not just the frontend reservation). A buffer handle is used
// rather than a texture/sampler to avoid a separate suspected Mesa image-descriptor bug.
//TEST_INPUT:ubuffer(data=[10.0 20.0 30.0 40.0], stride=4):name=bindless
uniform RWStructuredBuffer<float>.Handle bindless;

//CHK:11
//CHK-NEXT:22
//CHK-NEXT:33
//CHK-NEXT:44
// The live heap lands after the declared-but-unused parameter blocks: set 2 with p1, set 3 once
// REPRO adds p2. This is the frontend-reservation vs post-DCE placement invariant of #9870 — the
// unused blocks still reserve their sets, so the heap does not collide with a space that DCE freed.
// REFLECTION pins the frontend index and HEAP pins the lowered set, in both configurations, so the
// two are asserted to agree.
//REFLECTION: "bindlessSpaceIndex": 2
//REFLECTION_REPRO: "bindlessSpaceIndex": 3
//HEAP: OpDecorate %__slang_resource_heap{{.*}}DescriptorSet 2
//HEAP_REPRO: OpDecorate %__slang_resource_heap{{.*}}DescriptorSet 3

struct P
{
        Texture2D.Handle                t;
};

ParameterBlock<P> p1;

#if defined(REPRO)
ParameterBlock<P> p2;
#endif

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    gOutput[tid] = gInput[tid] + bindless[tid];
}
