//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry codegenMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry codegenMain -stage compute -experimental-feature

// Verify that differentiable generic matrix extrema use the layout-preserving shaped adapters.

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 10.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 40.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 6.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 20.0
// CHECK-NEXT: 30.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 1.0

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

// HLSL-DAG: return min(left_{{[0-9]+}}, right_{{[0-9]+}});
// HLSL-DAG: return max(left_{{[0-9]+}}, right_{{[0-9]+}});

// SPIRV-DAG: %[[V2_FLOAT:[A-Za-z0-9_]+]] = OpTypeVector %float 2
// SPIRV-DAG: OpExtInst %[[V2_FLOAT]] {{.*}} FMin
// SPIRV-DAG: OpExtInst %[[V2_FLOAT]] {{.*}} FMax

import slang.numerics.differentiable;

[Differentiable]
T genericMinimum<T : IDifferentiableRealOrderingFunctions>(T left, T right)
{
    return min(left, right);
}

[Differentiable]
T genericMaximum<T : IDifferentiableRealOrderingFunctions>(T left, T right)
{
    return max(left, right);
}

typealias Matrix = matrix<float, 2, 2, MatrixLayoutMode.RowMajor>;

[Differentiable]
Matrix matrixMinimum(Matrix left, Matrix right)
{
    return genericMinimum(left, right);
}

[Differentiable]
Matrix matrixMaximum(Matrix left, Matrix right)
{
    return genericMaximum(left, right);
}

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<Matrix> codegenOutput;

[shader("compute")]
[numthreads(1, 1, 1)]
void codegenMain()
{
    Matrix left = Matrix(1.0, 4.0, 5.0, 2.0);
    Matrix right = Matrix(3.0, 2.0, 4.0, 6.0);
    codegenOutput[0] = genericMinimum(left, right);
    codegenOutput[1] = genericMaximum(left, right);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Matrix left = Matrix(1.0, 4.0, 5.0, 2.0);
    Matrix right = Matrix(3.0, 2.0, 4.0, 6.0);
    Matrix leftTangent = Matrix(10.0, 20.0, 30.0, 40.0);
    Matrix rightTangent = Matrix(1.0, 2.0, 3.0, 4.0);

    let minimumResult = fwd_diff(matrixMinimum)(
        diffPair(left, leftTangent), diffPair(right, rightTangent));
    let maximumResult = fwd_diff(matrixMaximum)(
        diffPair(left, leftTangent), diffPair(right, rightTangent));

    [ForceUnroll]
    for (int row = 0; row < 2; ++row)
    {
        [ForceUnroll]
        for (int column = 0; column < 2; ++column)
        {
            int index = row * 2 + column;
            outputBuffer[index] = minimumResult.p[row][column];
            outputBuffer[index + 4] = minimumResult.d[row][column];
            outputBuffer[index + 8] = maximumResult.p[row][column];
            outputBuffer[index + 12] = maximumResult.d[row][column];
        }
    }

    var minimumLeft = diffPair(left, Matrix(0.0));
    var minimumRight = diffPair(right, Matrix(0.0));
    bwd_diff(matrixMinimum)(minimumLeft, minimumRight, Matrix(1.0));
    outputBuffer[16] = minimumLeft.d[0][0];
    outputBuffer[17] = minimumRight.d[0][0];

    var maximumLeft = diffPair(left, Matrix(0.0));
    var maximumRight = diffPair(right, Matrix(0.0));
    bwd_diff(matrixMaximum)(maximumLeft, maximumRight, Matrix(1.0));
    outputBuffer[18] = maximumLeft.d[0][0];
    outputBuffer[19] = maximumRight.d[0][0];
}
