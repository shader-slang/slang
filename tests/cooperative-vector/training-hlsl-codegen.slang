//TEST:SIMPLE(filecheck=CHECK609): -target hlsl -entry computeMain -profile cs_6_9
//TEST:SIMPLE(filecheck=CHECK610): -target hlsl -entry computeMain -profile cs_6_10

// CHECK609-DAG: RWByteAddressBuffer outerProductOutput_0 : register
// CHECK609-DAG: RWByteAddressBuffer reduceSumOutput_0 : register
// CHECK609-DAG: __builtin_OuterProductAccumulate(
// CHECK609-DAG: __slang_linalg_OuterProductAccumulate<
// CHECK609-DAG: __builtin_VectorAccumulate(
// CHECK609-DAG: __slang_linalg_VectorAccumulate<

// CHECK610-DAG: dx::linalg::OuterProduct<
// CHECK610-DAG: InterlockedAccumulate(
// CHECK610-DAG: __slang_linalg_OuterProductAccumulate<
// CHECK610-DAG: __slang_linalg_VectorAccumulate<

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
