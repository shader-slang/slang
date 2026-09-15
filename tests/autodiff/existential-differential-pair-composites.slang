//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
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
    float replacePair(inout ValuePair pair);
    void replaceArray(inout PairArray pairs);
    [Differentiable]
    float consumeArrayReverse(inout PairArray pairs);
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

    float replacePair(inout ValuePair pair)
    {
        let incoming = fwd_diff(readValue)(pair);

        // Change both the value and its concrete implementation. The witness wrapper must cast
        // the incoming type-flow pair to the implementation parameter, then cast the updated
        // nominal pair back through the wrapper-side pointer after the call.
        pair = fwd_diff(makeSquaredValue)(diffPair(incoming.p + 1.0, incoming.d));
        return incoming.p * 1000.0 + incoming.d * 100.0;
    }

    void replaceArray(inout PairArray pairs)
    {
        let first = fwd_diff(readValue)(pairs[0]);
        let second = fwd_diff(readValue)(pairs[1]);

        // This is the same write-back case nested inside an array. Make each output use the other
        // concrete implementation so a stale or one-way conversion cannot satisfy the checks.
        pairs[0] = fwd_diff(makeSquaredValue)(diffPair(first.p + 1.0, first.d));
        pairs[1] = fwd_diff(makeLinearValue)(diffPair(second.p + 1.0, second.d));
    }

    [Differentiable]
    float consumeArrayReverse(inout PairArray pairs)
    {
        return readValue(pairs[0].p) + readValue(pairs[1].p);
    }
}

[Differentiable]
PairArray makeReversePairs()
{
    PairArray pairs = {
        fwd_diff(makeLinearValue)(diffPair(2.0, 0.0)),
        fwd_diff(makeSquaredValue)(diffPair(3.0, 0.0))};
    return pairs;
}

[Differentiable]
float dispatchArrayReverse(inout PairArray pairs)
{
    ICompositeConsumer consumer = CompositeConsumer();
    return consumer.consumeArrayReverse(pairs);
}

[Differentiable]
float evaluateArrayReverse(PairArray pairs)
{
    return dispatchArrayReverse(pairs);
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

    ValuePair inoutPair = fwd_diff(makeLinearValue)(diffPair(2.0, 0.5));
    float incoming = consumer.replacePair(inoutPair);
    let updated = fwd_diff(readValue)(inoutPair);

    // `incoming` encodes (2, 0.5). The replacement is SquaredValueHolder(3), so reading it yields
    // primal 9 and tangent 3. Observing both values here proves the wrapper wrote the pair back.
    outputBuffer[3] = incoming + updated.p * 10.0 + updated.d; // CHECK: 2143

    PairArray inoutPairs = {
        fwd_diff(makeLinearValue)(diffPair(1.0, 0.25)),
        fwd_diff(makeSquaredValue)(diffPair(2.0, 0.5))};
    consumer.replaceArray(inoutPairs);
    let updatedFirst = fwd_diff(readValue)(inoutPairs[0]);
    let updatedSecond = fwd_diff(readValue)(inoutPairs[1]);

    // The updated values are (4, 1) and (5, 2), including the nontrivial squared tangent.
    outputBuffer[4] =
        updatedFirst.p * 1000.0 + updatedFirst.d * 100.0 + updatedSecond.p * 10.0 + updatedSecond.d;
    // CHECK: 4152

    // Differentiating this wrapper reaches an `inout PairArray` call through the witness table.
    // Its backward wrapper must recursively cast the composite and write the accumulated
    // existential differentials back through both the inout pointer and the outer array.
    DifferentialPair<PairArray> reversePairs = fwd_diff(makeReversePairs)();
    bwd_diff(dispatchArrayReverse)(reversePairs, 1.0);

    // Reapply the function in forward mode with the accumulated cotangent as its direction. The
    // resulting tangent is the gradient's squared norm: 1 * 1 + 6 * 6 = 37. This checks both
    // returned run-time differential values without downcasting an associated existential type.
    let gradientCheck = fwd_diff(evaluateArrayReverse)(reversePairs);
    outputBuffer[5] = gradientCheck.d; // CHECK: 37
}
