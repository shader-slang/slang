//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IValueHolder : IDifferentiable
{
    property Value : float
    {
        [Differentiable]
        get;

        [Differentiable]
        set;
    }
}

struct LinearValueHolder : IValueHolder
{
    float value;

    __init(float value)
    {
        this.value = value;
    }

    property Value : float
    {
        [Differentiable]
        get
        {
            return value * 2.0;
        }

        [Differentiable]
        set
        {
            value = newValue * 3.0;
        }
    }
}

// Keep this associated differential type distinct so dispatch marshalling cannot assume that a
// concrete primal type is also its differential type.
struct SquaredValueHolderDifferential : IDifferentiable
{
    float value;
}

struct SquaredValueHolder : IValueHolder
{
    typealias Differential = SquaredValueHolderDifferential;

    [DerivativeMember(Differential.value)]
    float value;

    __init(float value)
    {
        this.value = value;
    }

    property Value : float
    {
        [Differentiable]
        get
        {
            return value * value;
        }

        [Differentiable]
        set
        {
            value = newValue * 2.0;
        }
    }
}

[Differentiable]
float scaleAndStore(IValueHolder valueHolder, float factor)
{
    valueHolder.Value = valueHolder.Value * factor;
    return valueHolder.Value;
}

[Differentiable]
float dispatchSingleType(float factor)
{
    IValueHolder valueHolder = no_diff(LinearValueHolder(2.0));
    return scaleAndStore(valueHolder, factor);
}

[Differentiable]
float dispatchMultipleTypes(uint typeID, float factor)
{
    IValueHolder valueHolder;
    if (typeID == 0)
        valueHolder = no_diff(LinearValueHolder(3.0));
    else
        valueHolder = no_diff(SquaredValueHolder(2.0));
    return scaleAndStore(valueHolder, factor);
}

//TEST_INPUT: type_conformance LinearValueHolder:IValueHolder = 0
//TEST_INPUT: type_conformance SquaredValueHolder:IValueHolder = 1

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // The single-type path still reaches both accessors through IValueHolder. Starting from
    // value=2 and factor=4, the get-set-get sequence is 4 -> 48 -> 96. Its derivative with
    // respect to factor is 24, so a reverse seed of 2 produces 48.
    let singleFwd = fwd_diff(dispatchSingleType)(diffPair(4.0, 0.5));
    var singleFactorPair = diffPair(4.0);
    bwd_diff(dispatchSingleType)(singleFactorPair, 2.0);
    outputBuffer[0] = singleFwd.p; // CHECK: 96.0
    outputBuffer[1] = singleFwd.d; // CHECK: 12.0
    outputBuffer[2] = singleFactorPair.d; // CHECK: 48.0

    // The first multiple-type branch selects LinearValueHolder dynamically. Its result is 72,
    // and a reverse seed of 0.5 scales d/dfactor=36 to 18.
    let linearFwd = fwd_diff(dispatchMultipleTypes)(0, diffPair(2.0, 2.0));
    var linearFactorPair = diffPair(2.0);
    bwd_diff(dispatchMultipleTypes)(0, linearFactorPair, 0.5);
    outputBuffer[3] = linearFwd.p; // CHECK: 72.0
    outputBuffer[4] = linearFwd.d; // CHECK: 72.0
    outputBuffer[5] = linearFactorPair.d; // CHECK: 18.0

    // The second branch selects SquaredValueHolder. Its get-set-get sequence is
    // 4 -> 24 -> 576, and a reverse seed of 0.5 scales d/dfactor=384 to 192.
    let squaredFwd = fwd_diff(dispatchMultipleTypes)(1, diffPair(3.0, 0.25));
    var squaredFactorPair = diffPair(3.0);
    bwd_diff(dispatchMultipleTypes)(1, squaredFactorPair, 0.5);
    outputBuffer[6] = squaredFwd.p; // CHECK: 576.0
    outputBuffer[7] = squaredFwd.d; // CHECK: 96.0
    outputBuffer[8] = squaredFactorPair.d; // CHECK: 192.0
}
