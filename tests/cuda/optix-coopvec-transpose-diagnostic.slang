//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -capability optix_coopvec -entry computeMain -stage compute

using namespace linalg;

ByteAddressBuffer input;
ByteAddressBuffer matrix;
ByteAddressBuffer bias;
RWStructuredBuffer<half> output;

[numthreads(1,1,1)]
void computeMain()
{
    let vec = coopVecLoad<4, half>(input);

    let resultMatMul = coopVecMatMul<half, 4, 4>(
        vec,
        CoopVecComponentType::Float16,
        matrix,
        0,
        CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor,
        true,
        8);

    let resultMatMulAdd = coopVecMatMulAdd<half, 4, 4>(
        vec,
        CoopVecComponentType::Float16,
        matrix,
        0,
        CoopVecComponentType::Float16,
        bias,
        0,
        CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor,
        true,
        8);

    output[0] = resultMatMul[0];
    output[1] = resultMatMulAdd[0];
}

//CHECK: unsupported intrinsic operation
//CHECK: intrinsic operation 'cooperative vector matrix multiply-add with transpose requires Float16 matrix interpretation and InferencingOptimal or TrainingOptimal matrix layout for OptiX' is not supported for the current target.
//CHECK: unsupported intrinsic operation
//CHECK: intrinsic operation 'cooperative vector matrix multiply-add with transpose requires Float16 matrix interpretation and InferencingOptimal or TrainingOptimal matrix layout for OptiX' is not supported for the current target.
