//TEST:COMPARE_COMPUTE(targets=vk+glsl+cuda+cpu, filecheck-buffer=CHECK):-output-using-type -Xslang -experimental-feature

// CHECK: type: float
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 10.000000
// CHECK-NEXT: 12.000000
// CHECK-NEXT: 11.000000
// CHECK-NEXT: 16.000000
// CHECK-NEXT: 21.000000
// CHECK-NEXT: 26.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 18.000000

// The selection checks expect four components each for the primal result, its tangent, and the
// adjoints for the true and false inputs, in that order.
// CHECK-NEXT: {{^}}2.000000{{$}}
// CHECK-NEXT: {{^}}20.000000{{$}}
// CHECK-NEXT: {{^}}4.000000{{$}}
// CHECK-NEXT: {{^}}40.000000{{$}}
// CHECK-NEXT: {{^}}1.000000{{$}}
// CHECK-NEXT: {{^}}20.000000{{$}}
// CHECK-NEXT: {{^}}3.000000{{$}}
// CHECK-NEXT: {{^}}40.000000{{$}}
// CHECK-NEXT: {{^}}2.000000{{$}}
// CHECK-NEXT: {{^}}0.000000{{$}}
// CHECK-NEXT: {{^}}4.000000{{$}}
// CHECK-NEXT: {{^}}0.000000{{$}}
// CHECK-NEXT: {{^}}0.000000{{$}}
// CHECK-NEXT: {{^}}3.000000{{$}}
// CHECK-NEXT: {{^}}0.000000{{$}}
// CHECK-NEXT: {{^}}5.000000{{$}}

//TEST_INPUT:ubuffer(count=33, stride=4):out,name=outputBuffer

import slang.numerics.differentiable;

RWStructuredBuffer<float> outputBuffer;

[Differentiable]
T exerciseDifferentiableConstruction<T : IDifferentiableFractional>(T value, T other, float scalar)
{
    let constructed = T(scalar);
    let repeated = T.fromScalar(T.Scalar(scalar));
    return ((value + other) - other) * constructed + repeated;
}

[Differentiable]
T chooseComponents<T : IDifferentiableNumericShapedType>(
    no_diff T.Mask condition,
    T ifTrue,
    T ifFalse)
{
    return select(condition, ifTrue, ifFalse);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let value = CoopVec<float, 4>(2.0, 3.0, 4.0, 5.0);
    let other = CoopVec<float, 4>(10.0, 20.0, 30.0, 40.0);
    let result = fwd_diff(exerciseDifferentiableConstruction<CoopVec<float, 4>>)(
        diffPair(
            value,
            CoopVec<float, 4>(1.0, 2.0, 3.0, 4.0)),
        diffPair(other, CoopVec<float, 4>(0.0)),
        diffPair(2.0, 3.0));

    [ForceUnroll]
    for (int index = 0; index < 4; ++index)
    {
        outputBuffer[index] = result.p[index];
        outputBuffer[index + 4] = result.d[index];
    }

    var reverseValue = diffPair(value, CoopVec<float, 4>(0.0));
    var reverseOther = diffPair(other, CoopVec<float, 4>(0.0));
    var reverseScalar = diffPair(2.0, 0.0);
    bwd_diff(exerciseDifferentiableConstruction<CoopVec<float, 4>>)(
        reverseValue,
        reverseOther,
        reverseScalar,
        CoopVec<float, 4>(1.0));

    [ForceUnroll]
    for (int index = 0; index < 4; ++index)
    {
        outputBuffer[index + 8] = reverseValue.d[index];
        outputBuffer[index + 12] = reverseOther.d[index];
    }
    outputBuffer[16] = reverseScalar.d;

    // The same mixed mask selects primal values and their derivatives. Reverse mode must route
    // each component's gradient to exactly one argument.
    let mask = bool4(true, false, true, false);
    let selected = fwd_diff(chooseComponents<CoopVec<float, 4>>)(
        mask,
        diffPair(value, CoopVec<float, 4>(1.0, 2.0, 3.0, 4.0)),
        diffPair(other, CoopVec<float, 4>(10.0, 20.0, 30.0, 40.0)));
    var reverseIfTrue = diffPair(value, CoopVec<float, 4>(0.0));
    var reverseIfFalse = diffPair(other, CoopVec<float, 4>(0.0));
    bwd_diff(chooseComponents<CoopVec<float, 4>>)(
        mask, reverseIfTrue, reverseIfFalse, CoopVec<float, 4>(2.0, 3.0, 4.0, 5.0));

    [ForceUnroll]
    for (int index = 0; index < 4; ++index)
    {
        outputBuffer[index + 17] = selected.p[index];
        outputBuffer[index + 21] = selected.d[index];
        outputBuffer[index + 25] = reverseIfTrue.d[index];
        outputBuffer[index + 29] = reverseIfFalse.d[index];
    }
}
