//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type -Xslang -DREPRO
//TEST:REFLECTION(filecheck=REFLECTION):-target spirv-asm -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

//CHK:1
//CHK-NEXT:2
//CHK-NEXT:3
//CHK-NEXT:4
//REFLECTION: "bindlessSpaceIndex": 2

struct P
{
        Texture2D.Handle                t;
};

// p1 is intentionally unused. Frontend layout accounts for its parameter space when reserving
// bindlessSpaceIndex before DCE (shader-slang/slang#9870); the reflection check above verifies
// that index. REPRO adds p2, but these compute variants do not exercise a live bindless heap.
ParameterBlock<P> p1;

#if defined(REPRO)
ParameterBlock<P> p2;
#endif

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Nothing is bound to p1.t, so it must not be dereferenced: loading an unbound bindless
    // descriptor is UB under Vulkan (SIGSEGV on lavapipe).
    gOutput[tid] = gInput[tid];
}
