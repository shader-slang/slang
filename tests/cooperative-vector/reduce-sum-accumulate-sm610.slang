// SM6.10 behavioral test for cooperative-vector reduce-sum accumulate.
// TODO: enable when GPU CI runners gain cs_6_10 / DXC SM6.10 support
//       (requires DXC >= v1.9.2602 and dx::linalg::VectorAccumulate in dx/linalg.h).
// Multi-threaded: 8 lanes each accumulate [1,1,1,1] into the same byte offset.
// Atomic reduce-sum yields output[i]=8; a non-atomic load/add/store loop would
// produce a smaller value due to lost updates under thread contention.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_10 -Xdxc -Vd

// CHECK: type: half
// CHECK-NEXT: 8.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 8.0

//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=2):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=2),name=output
RWByteAddressBuffer output;

[numthreads(8, 1, 1)]
void computeMain(uint groupIndex : SV_GroupIndex)
{
    CoopVec<half, 4> vecA;
    for(int i = 0; i < vecA.getCount(); ++i)
        vecA[i] = half(1);

    coopVecReduceSumAccumulate(
        vecA,
        output,
        0,
    );

    AllMemoryBarrierWithGroupSync();

    if (groupIndex == 0)
    {
        for(int i = 0; i < vecA.getCount(); ++i)
            outputBuffer[i] = output.Load<half>(i * 2);
    }
}
