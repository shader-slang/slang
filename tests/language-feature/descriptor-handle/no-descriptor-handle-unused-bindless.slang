//TEST:SIMPLE(filecheck=IR):-target spirv-asm -stage compute -entry computeMain -dump-ir-after collectMetadata

RWStructuredBuffer<float> gInput;
RWStructuredBuffer<float> gOutput;

//IR-LABEL: ### AFTER collectMetadata:
//IR-NOT: [nameHint("__slang_resource_heap")]
//IR-NOT: LoadResourceDescriptorFromHeap
//IR-NOT: SPIRVLoadDescriptorFromHeap
//IR-NOT: getElement(%x5Fx5Fslangx5Fresourcex5Fheap

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    gOutput[tid] = gInput[tid] + 1.0f;
}
