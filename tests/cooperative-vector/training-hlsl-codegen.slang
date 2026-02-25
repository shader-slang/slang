//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -profile cs_6_9

// CHECK: RWByteAddressBuffer outerProductOutput_0 : register
// CHECK: RWByteAddressBuffer reduceSumOutput_0 : register
// CHECK-DAG: __builtin_OuterProductAccumulate((_S{{[0-9]+}}), (_S{{[0-9]+}}), (outerProductOutput_0), (int(0)), (8U), (3U), (32U));
// CHECK-DAG: __builtin_VectorAccumulate((vecA_0), (reduceSumOutput_0), (int(0)));

RWByteAddressBuffer outerProductOutput;
RWByteAddressBuffer reduceSumOutput;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<half, 4> vecA;
    CoopVec<half, 8> vecB;

    for (int i = 0; i < vecA.getCount(); ++i)
        vecA[i] = half(i + 1);

    for (int i = 0; i < vecB.getCount(); ++i)
        vecB[i] = half(i + 1);

    coopVecOuterProductAccumulate(
        vecA,
        vecB,
        outerProductOutput,
        0,
        32,
        CoopVecMatrixLayout::TrainingOptimal,
        CoopVecComponentType::Float16,
    );

    coopVecReduceSumAccumulate(
        vecA,
        reduceSumOutput,
        0,
    );
}
