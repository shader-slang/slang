//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IValueHolder : IDifferentiable
{
    [Differentiable]
    float getValue();
}

struct LinearValueHolder : IValueHolder
{
    float value;

    [Differentiable]
    __init(float value)
    {
        this.value = value;
    }

    [Differentiable]
    float getValue()
    {
        return value;
    }
}

struct SquaredValueHolder : IValueHolder
{
    float value;

    [Differentiable]
    __init(float value)
    {
        this.value = value;
    }

    [Differentiable]
    float getValue()
    {
        return value * value;
    }
}

[Differentiable]
IValueHolder makeLinearValue(float value)
{
    return LinearValueHolder(value);
}

[Differentiable]
IValueHolder makeSquaredValue(float value)
{
    return SquaredValueHolder(value);
}

[Differentiable]
float readValue(IValueHolder valueHolder)
{
    return valueHolder.getValue();
}

typealias ValuePair = DifferentialPair<IValueHolder>;
typealias PairArray = Array<ValuePair, 2>;
typealias PairTuple = Tuple<ValuePair, ValuePair>;
typealias OptionalPair = Optional<ValuePair>;

interface ICompositeConsumer
{
    float consumeArray(PairArray pairs);
    float consumeTuple(PairTuple pairs);
    float consumeOptional(OptionalPair pair);
}

struct CompositeConsumer : ICompositeConsumer
{
    float consumeArray(PairArray pairs)
    {
        let first = fwd_diff(readValue)(pairs[0]);
        let second = fwd_diff(readValue)(pairs[1]);
        return first.p * 100.0 + first.d * 10.0 + second.p + second.d;
    }

    float consumeTuple(PairTuple pairs)
    {
        let first = fwd_diff(readValue)(pairs._0);
        let second = fwd_diff(readValue)(pairs._1);
        return first.p * 100.0 + first.d * 10.0 + second.p + second.d;
    }

    float consumeOptional(OptionalPair pair)
    {
        if (!pair.hasValue)
            return 0.0;

        let value = fwd_diff(readValue)(pair.value);
        return value.p * 10.0 + value.d;
    }
}

//TEST_INPUT: type_conformance LinearValueHolder:IValueHolder = 0
//TEST_INPUT: type_conformance SquaredValueHolder:IValueHolder = 1
//TEST_INPUT: type_conformance CompositeConsumer:ICompositeConsumer = 2

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ValuePair pair1 = fwd_diff(makeLinearValue)(diffPair(1.0, 0.5));
    ValuePair pair2 = fwd_diff(makeSquaredValue)(diffPair(2.0, 0.25));
    ValuePair pair3 = fwd_diff(makeLinearValue)(diffPair(3.0, 0.75));
    ValuePair pair4 = fwd_diff(makeSquaredValue)(diffPair(2.0, 0.5));
    ValuePair pair5 = fwd_diff(makeLinearValue)(diffPair(5.0, 1.25));

    PairArray arrayPairs = {pair1, pair2};
    PairTuple tuplePairs = {pair3, pair4};
    OptionalPair optionalPair = pair5;

    // Each call crosses a witness-table boundary with a different composite containing
    // existential differential pairs. The dispatcher must recurse through the outer composite
    // and then adapt the pair's named primal and differential components. Treating propagation
    // info as an ordinary two-element tuple loses which nested tuples are pairs in these shapes.
    // Each consumer applies `fwd_diff(readValue)` so the checks validate both adapted components;
    // for SquaredValueHolder the tangent is `2 * value * inputTangent`.
    ICompositeConsumer consumer = CompositeConsumer();
    outputBuffer[0] = consumer.consumeArray(arrayPairs); // CHECK: 110.0
    outputBuffer[1] = consumer.consumeTuple(tuplePairs); // CHECK: 313.5
    outputBuffer[2] = consumer.consumeOptional(optionalPair); // CHECK: 51.25
}
