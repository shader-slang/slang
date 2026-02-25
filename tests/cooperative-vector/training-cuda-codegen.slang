//TEST:SIMPLE(filecheck=CHECK): -target cuda -capability optix_coopvec -entry computeMain -stage compute

// CHECK: optixCoopVecOuterProductAccumulate({{.*}}, {{.*}}, (CUdeviceptr)(&({{.*}})), int(0), 32U)
// CHECK: optixCoopVecReduceSumAccumulate({{.*}}, (CUdeviceptr)(&({{.*}})), int(0))

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
