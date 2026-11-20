// Selection through the differentiable shape contract routes derivatives to the chosen components.
// Mixed masks distinguish component-wise selection from choosing an entire aggregate.

//TEST:COMPARE_COMPUTE(targets=cpu+vk+cuda, filecheck-buffer=CHECK):-output-using-type -Xslang -experimental-feature

import slang.numerics.differentiable;

//TEST_INPUT:ubuffer(count=14, stride=4):out,name=results
RWStructuredBuffer<uint> results;

// A user-defined representation must dispatch through its differentiable conformance as well.
struct TwoValues : IDifferentiableNumericShapedType
{
    typealias Scalar = float;
    typealias Mask = bool2;

    float first;
    float second;

    [Differentiable]
    static This fromScalar(Scalar value)
    {
        return {value, value};
    }

    [Differentiable]
    static This selectComponents(no_diff Mask condition, This ifTrue, This ifFalse)
    {
        return {
            condition.x ? ifTrue.first : ifFalse.first,
            condition.y ? ifTrue.second : ifFalse.second};
    }
}

[Differentiable]
T chooseComponents<T : IDifferentiableNumericShapedType>(
    no_diff T.Mask condition, T ifTrue, T ifFalse)
{
    return select(condition, ifTrue, ifFalse);
}

// Checks scalar selection in forward mode. For a vector, a column-major matrix, and `TwoValues`,
// the checks cover both forward and reverse modes.
// The harness provides 14 entries in `results`. Each check writes 1 for success or 0 for failure.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // For scalars, both the primal result and its tangent come from the chosen argument.
    let scalar = fwd_diff(chooseComponents<float>)(
        false, diffPair(2.0, 3.0), diffPair(7.0, 11.0));
    results[0] = scalar.p == 7.0; // CHECK: 1
    results[1] = scalar.d == 11.0; // CHECK: 1

    // Each selected vector component has its own incoming tangent and outgoing adjoint.
    let mask = bool2(true, false);
    let vectorForward = fwd_diff(chooseComponents<float2>)(
        mask, diffPair(float2(2.0, 3.0), float2(1.0, 2.0)),
        diffPair(float2(10.0, 20.0), float2(3.0, 4.0)));
    results[2] = (vectorForward.p == float2(2.0, 20.0)).all(); // CHECK: 1
    results[3] = (vectorForward.d == float2(1.0, 4.0)).all(); // CHECK: 1

    var vectorLeft = diffPair(float2(2.0, 3.0), float2(0.0));
    var vectorRight = diffPair(float2(10.0, 20.0), float2(0.0));
    bwd_diff(chooseComponents<float2>)(mask, vectorLeft, vectorRight, float2(5.0, 7.0));
    results[4] = (vectorLeft.d == float2(5.0, 0.0)).all(); // CHECK: 1
    results[5] = (vectorRight.d == float2(0.0, 7.0)).all(); // CHECK: 1

    // We use a column-major matrix and give each component a distinct value.
    // Swapping result columns then changes their values even when their mask bits match.
    typealias Matrix = matrix<float, 2, 3, MatrixLayoutMode.ColumnMajor>;
    typealias MatrixMask = matrix<bool, 2, 3, MatrixLayoutMode.ColumnMajor>;
    let matrixMask = MatrixMask(true, false, true, false, true, false);
    let matrixIfTrue = Matrix(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    let matrixIfFalse = Matrix(11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
    let matrixForward = fwd_diff(chooseComponents<Matrix>)(
        matrixMask,
        diffPair(matrixIfTrue, Matrix(21.0, 22.0, 23.0, 24.0, 25.0, 26.0)),
        diffPair(matrixIfFalse, Matrix(31.0, 32.0, 33.0, 34.0, 35.0, 36.0)));
    results[6] = (matrixForward.p == Matrix(1.0, 12.0, 3.0, 14.0, 5.0, 16.0)).all(); // CHECK: 1
    results[7] = (matrixForward.d == Matrix(21.0, 32.0, 23.0, 34.0, 25.0, 36.0)).all(); // CHECK: 1

    var reverseIfTrue = diffPair(matrixIfTrue, Matrix(0.0));
    var reverseIfFalse = diffPair(matrixIfFalse, Matrix(0.0));
    let matrixOutputDifferential = Matrix(41.0, 42.0, 43.0, 44.0, 45.0, 46.0);
    bwd_diff(chooseComponents<Matrix>)(
        matrixMask, reverseIfTrue, reverseIfFalse, matrixOutputDifferential);
    results[8] = (reverseIfTrue.d == Matrix(41.0, 0.0, 43.0, 0.0, 45.0, 0.0)).all(); // CHECK: 1
    results[9] = (reverseIfFalse.d == Matrix(0.0, 42.0, 0.0, 44.0, 0.0, 46.0)).all(); // CHECK: 1

    let custom = fwd_diff(chooseComponents<TwoValues>)(
        mask, diffPair(TwoValues(2.0, 3.0), TwoValues.Differential(1.0, 2.0)),
        diffPair(TwoValues(10.0, 20.0), TwoValues.Differential(3.0, 4.0)));
    results[10] = bool2(custom.p.first == 2.0, custom.p.second == 20.0).all(); // CHECK: 1
    results[11] = bool2(custom.d.first == 1.0, custom.d.second == 4.0).all(); // CHECK: 1

    var customLeft = diffPair(TwoValues(2.0, 3.0));
    var customRight = diffPair(TwoValues(10.0, 20.0));
    bwd_diff(chooseComponents<TwoValues>)(
        mask, customLeft, customRight, TwoValues.Differential(5.0, 7.0));
    results[12] = bool2(customLeft.d.first == 5.0, customLeft.d.second == 0.0).all(); // CHECK: 1
    results[13] = bool2(customRight.d.first == 0.0, customRight.d.second == 7.0).all(); // CHECK: 1
}
