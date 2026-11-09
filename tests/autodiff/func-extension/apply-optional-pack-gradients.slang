//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

[Differentiable]
float squareIfFloat<T>(T value) where optional T : IDifferentiable
{
    if (T is float)
    {
        float x = (value as float).value;
        return x * x;
    }
    else
        return 0.0;
}

[Differentiable]
float sumSquares<each T>(expand each T values) where optional T : IDifferentiable
{
    float sum = 0.0;
    expand sum += squareIfFloat<each T>(each values);
    return sum;
}

Tuple<float, Tuple<expand DifferentialOrNone<each T>>> applyPack<each T>(expand each T values)
    where optional T : IDifferentiable
{
    let applied = __apply(sumSquares<expand each T>)(expand each values);
    typealias Function = __func_as_type(sumSquares<expand each T>);
    let context = Function.remat(applied._1, expand each values);
    Tuple<expand DifferentialOrNone<each T>> gradients;
    context(expand each gradients, 1.0);
    return Tuple<float, Tuple<expand DifferentialOrNone<each T>>>(applied._0, gradients);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let indexFirst = applyPack(uint(7), 3.0);
    let indexLast = applyPack(3.0, uint(7));
    let twoActive = applyPack(2.0, uint(7), 3.0);
    outputBuffer[0] = indexFirst._0;                 // CHECK: 9.0
    outputBuffer[1] = indexFirst._1._1;              // CHECK: 6.0
    outputBuffer[2] = indexLast._1._0;               // CHECK: 6.0
    outputBuffer[3] = applyPack(uint(7), uint(2))._0; // CHECK: 0.0
    outputBuffer[4] = applyPack()._0;                // CHECK: 0.0
    outputBuffer[5] = twoActive._0;                  // CHECK: 13.0
    outputBuffer[6] = twoActive._1._0;               // CHECK: 4.0
    outputBuffer[7] = twoActive._1._2;               // CHECK: 6.0
}
