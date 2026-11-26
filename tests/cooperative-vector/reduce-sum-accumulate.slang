//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-render-feature cooperative-vector -output-using-type

// CPU target doesn't support float16_t

// HLSL doesn't support the training operations

// CHECK: type: half
// CHECK-NEXT: 112.000000
// CHECK-NEXT: 2.00
// CHECK-NEXT: 3.00
// CHECK-NEXT: 4.0


//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=2):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4),name=inputA
ByteAddressBuffer inputA;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4),name=output
RWByteAddressBuffer output;

[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<half, 4> vecA;
    for(int i = 0; i < vecA.getCount(); ++i)
        vecA[i] = half(i+1);

    output.Store(0, half(111));

    coopVecReduceSumAccumulate(
        vecA,
        output,
        0,
    );

    for(int i = 0; i < vecA.getCount(); ++i)
        outputBuffer[i] = output.Load<half>(i * 2);
}

