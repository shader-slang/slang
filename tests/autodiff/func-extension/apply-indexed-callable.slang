//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

// A calculated function type keeps the callable's concrete <uint, float> pack as one type
// operand. Source arity checking must still count two explicit arguments, plus the receiver.
struct IndexedSquare : IDifferentiableFuncWithDiffThis<float, uint, float>
{
    typealias Differential = IndexedSquare;

    [Differentiable]
    float operator()(uint index, float value)
    {
        return value * value + float(index);
    }
}

float applyIndexed<F : IDifferentiableFuncWithDiffThis<float, uint, float>>(
    DifferentialPtrPair<F> function, uint index, float value)
{
    let applied = __apply(function.p.operator())(function, index, value);
    // Both IFunc and IDifferentiableFuncWithDiffThis contribute operator() requirements.
    // Member lookup must select the differentiable requirement before looking up remat.
    let backward = function.p.operator().remat(applied._1, function, index, value);
    __none_t receiverGradient;
    __none_t indexGradient;
    float valueGradient;
    backward(receiverGradient, indexGradient, valueGradient, 1.0f);
    return applied._0 + valueGradient;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let function = IndexedSquare();
    outputBuffer[0] = applyIndexed(
        DifferentialPtrPair<IndexedSquare>(function, function), 7, 3.0f);
    // CHECK: 22.0
}
