//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type -Xslang -DREPRO
//TEST:REFLECTION(filecheck=REFLECTION):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=IR):-target spirv-asm -stage compute -entry computeMain -dump-ir-after collectMetadata

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

//CHK:1
//CHK-NEXT:2
//CHK-NEXT:3
//CHK-NEXT:4
//REFLECTION: "bindlessSpaceIndex": 2
//IR-LABEL: ### AFTER collectMetadata:
//IR-NOT: LoadResourceDescriptorFromHeap
//IR-NOT: SPIRVLoadDescriptorFromHeap
//IR: [nameHint("__slang_resource_heap")]
//IR: = global_param
//IR-NOT: LoadResourceDescriptorFromHeap
//IR-NOT: SPIRVLoadDescriptorFromHeap
//IR: getElement(%x5Fx5Fslangx5Fresourcex5Fheap
//IR-NOT: LoadResourceDescriptorFromHeap
//IR-NOT: SPIRVLoadDescriptorFromHeap

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
    gOutput[tid] = gInput[tid] + p1.t.Load(int3(0,0,0)).x * gOutput[tid];
}
