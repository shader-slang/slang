//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -skip-spirv-validation

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=out
RWStructuredBuffer<uint> out;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=out1
RWByteAddressBuffer out1;

groupshared uint4 sharedData[1];

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    // sharedData[tid] = uint4(tid.x, tid.x + 1, tid.x + 2, tid.x + 3);
    //
    // Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared> ptr = __getAddress(sharedData[tid]);
    //
    // if (tid == 1)
    // {
    //     let res =  ptr[0] + ptr[1] + ptr[2] + ptr[3];
    //     out[0] = res.x;
    // }

    // uint[]* ptr = __getStructuredBufferPtr(out1);
    // uint4* ptr2 = Ptr<uint4, Access::ReadWrite>(ptr);
    //
    // // uint4* ptr2 = bit_cast<uint4*>(&out1[0]);
    //
    // *ptr2 = uint4(1);
    // out1[0] = uint(address >> 32);
    //

    out1.Store4(0, uint4(1, 2, 3, 4));

    out1.Store2(16, uint2(1, 2));
    // out1[0].store
}
