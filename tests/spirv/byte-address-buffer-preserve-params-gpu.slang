//TEST:COMPARE_COMPUTE(targets=all-dx11-wgpu, filecheck-buffer=CHECK):-output-using-type -Xslang -preserve-params
//TEST:COMPARE_COMPUTE(targets=all-dx11-wgpu, filecheck-buffer=CHECK):-output-using-type -Xslang -preserve-params -Xslang -O2
//TEST:COMPARE_COMPUTE(targets=all-dx11-wgpu, filecheck-buffer=CHECK):-output-using-type

// With -preserve-params, a shader that indexes an array of byte-address buffers and also
// declares an unused one still runs on Vulkan and reads the right words (#14039).

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[7 8 9 10], stride=4):name=unusedBuffer
ByteAddressBuffer unusedBuffer;

//TEST_INPUT:ubuffer(data=[11 12 13 14], stride=4):name=usedArray[0]
//TEST_INPUT:ubuffer(data=[21 22 23 24], stride=4):name=usedArray[1]
ByteAddressBuffer usedArray[2];

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):name=rw
RWByteAddressBuffer rw;

[numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = usedArray[tid.x].Load(4) + usedArray[1].Load<uint>(8);
    if (tid.x == 0)
    {
        rw.Store(0, 100u);
        outputBuffer[2] = rw.Load<uint>(4) + usedArray[0].Load<uint2>(8).y;
    }
}
// CHECK: 35
// CHECK-NEXT: 45
// CHECK-NEXT: 16
