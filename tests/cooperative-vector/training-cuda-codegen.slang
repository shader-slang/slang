// `-validate-with-downstream-compiler` makes slangc run NVRTC on the emitted CUDA source, so this
// test covers the downstream compile of the OptiX cooperative-vector calls, not just the emitted
// text. slang-test removes the option on machines that lack the downstream tool.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -capability optix_coopvec -entry computeMain -stage compute -validate-with-downstream-compiler

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
