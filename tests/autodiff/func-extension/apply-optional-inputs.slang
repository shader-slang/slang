//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

[Differentiable] float squareWithMetadata<T>(T metadata, float value) where optional T
    : IDifferentiable
{
    return value * value;
}

// Both specializations use the same call syntax, even while T's evidence is unresolved.
float squareGradient<T>(T metadata, float value) where optional T : IDifferentiable
{
    let applied = __apply(squareWithMetadata<T>)(metadata, value);
    typealias Function = __func_as_type(squareWithMetadata<T>);
    let context = Function.remat(applied._1, metadata, value);
    DifferentialOrNone<T> metadataGradient;
    float valueGradient;
    context(metadataGradient, valueGradient, 1.0);
    return applied._0 + valueGradient;
}

[Differentiable] float ignoreInputs<each T>(expand each T values) where optional T : IDifferentiable
{
    return 3.0;
}

Tuple<float, Tuple<expand DifferentialOrNone<each T>>> applyPack<each T>(
    expand each T values) where optional T : IDifferentiable
{
    let applied = __apply(ignoreInputs<expand each T>)(expand each values);
    typealias Function = __func_as_type(ignoreInputs<expand each T>);
    let context = Function.remat(applied._1, expand each values);
    Tuple<expand DifferentialOrNone<each T>> gradients;
    context(expand each gradients, 1.0);
    return Tuple<float, Tuple<expand DifferentialOrNone<each T>>>(applied._0, gradients);
}

// TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)] void computeMain()
{
    DifferentialOrNone<float> dx = 4.0;
    DifferentialOrNone<uint> noGradient = __none_t();
    outputBuffer[0] = dx;                              // CHECK: 4.0
    outputBuffer[1] = squareGradient<uint>(7, 3.0);    // CHECK: 15.0
    outputBuffer[2] = squareGradient<float>(7.0, 3.0); // CHECK: 15.0
    let indexFirst = applyPack(uint(7), 3.0);
    let indexLast = applyPack(3.0, uint(7));
    outputBuffer[3] = indexFirst._0;                  // CHECK: 3.0
    outputBuffer[4] = indexLast._0;                   // CHECK: 3.0
    outputBuffer[5] = applyPack(uint(7), uint(2))._0; // CHECK: 3.0
    outputBuffer[6] = applyPack()._0;                 // CHECK: 3.0

    // Concrete specializations retain the existing legacy derivative interface.
    var value = diffPair(3.0);
    bwd_diff(squareWithMetadata<uint>)(7, value, 1.0);
    outputBuffer[7] = value.d; // CHECK: 6.0

    // The active output slots must be written even when their derivative is zero.
    outputBuffer[8] = indexFirst._1._1; // CHECK: 0.0
    outputBuffer[9] = indexLast._1._0;  // CHECK: 0.0
}
