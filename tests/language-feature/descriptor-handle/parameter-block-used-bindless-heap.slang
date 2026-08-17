//TEST:SIMPLE(filecheck=IR):-target spirv-asm -stage compute -entry computeMain -dump-ir-after collectMetadata

RWStructuredBuffer<float> gInput;
RWStructuredBuffer<float> gOutput;

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

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    gOutput[tid] = gInput[tid] + p1.t.Load(int3(0,0,0)).x * gOutput[tid];
}
