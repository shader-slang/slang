//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IValueHolder : IDifferentiable
{
    __subscript(int index) -> float
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

    __subscript(int index) -> float
    {
        [Differentiable]
        get
        {
            return value * 2.0 + float(index);
        }

        [Differentiable]
        set
        {
            value = newValue * 3.0 + float(index);
        }
    }
}

// Keep this associated differential type distinct so subscript dispatch cannot assume that a
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

    __subscript(int index) -> float
    {
        [Differentiable]
        get
        {
            return value * value + float(index);
        }

        [Differentiable]
        set
        {
            value = newValue * 2.0 + float(index);
        }
    }
}

[Differentiable]
float scaleAndStore(IValueHolder valueHolder, int index, float factor)
{
    // Both accessors are reached through the existential. The index must be forwarded to each
    // selected implementation, while the setter's inout receiver crosses the specialized
    // differential representation in both directions.
    valueHolder[index] = valueHolder[index] * factor;
    return valueHolder[index];
}

[Differentiable]
float dispatchSingleType(float factor)
{
    IValueHolder valueHolder = no_diff(LinearValueHolder(2.0));
    return scaleAndStore(valueHolder, 1, factor);
}

[Differentiable]
float dispatchMultipleTypes(uint typeID, float factor)
{
    IValueHolder valueHolder;
    if (typeID == 0)
        valueHolder = no_diff(LinearValueHolder(3.0));
    else
        valueHolder = no_diff(SquaredValueHolder(2.0));
    return scaleAndStore(valueHolder, 1, factor);
}

//TEST_INPUT: type_conformance LinearValueHolder:IValueHolder = 0
//TEST_INPUT: type_conformance SquaredValueHolder:IValueHolder = 1

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // LinearValueHolder(2), index 1, and factor 4 follows 5 -> 61 -> 123 through
    // get-set-get. Its derivative with respect to factor is 30.
    let singleFwd = fwd_diff(dispatchSingleType)(diffPair(4.0, 0.5));
    var singleFactorPair = diffPair(4.0);
    bwd_diff(dispatchSingleType)(singleFactorPair, 2.0);
    outputBuffer[0] = singleFwd.p; // CHECK: 123.0
    outputBuffer[1] = singleFwd.d; // CHECK: 15.0
    outputBuffer[2] = singleFactorPair.d; // CHECK: 60.0

    // The first multiple-type branch selects LinearValueHolder(3). Its sequence is
    // 7 -> 43 -> 87, with d/dfactor=42.
    let linearFwd = fwd_diff(dispatchMultipleTypes)(0, diffPair(2.0, 2.0));
    var linearFactorPair = diffPair(2.0);
    bwd_diff(dispatchMultipleTypes)(0, linearFactorPair, 0.5);
    outputBuffer[3] = linearFwd.p; // CHECK: 87.0
    outputBuffer[4] = linearFwd.d; // CHECK: 84.0
    outputBuffer[5] = linearFactorPair.d; // CHECK: 21.0

    // The second branch selects SquaredValueHolder(2). Its sequence is 5 -> 31 -> 962,
    // with d/dfactor=620.
    let squaredFwd = fwd_diff(dispatchMultipleTypes)(1, diffPair(3.0, 0.25));
    var squaredFactorPair = diffPair(3.0);
    bwd_diff(dispatchMultipleTypes)(1, squaredFactorPair, 0.5);
    outputBuffer[6] = squaredFwd.p; // CHECK: 962.0
    outputBuffer[7] = squaredFwd.d; // CHECK: 155.0
    outputBuffer[8] = squaredFactorPair.d; // CHECK: 310.0
}
