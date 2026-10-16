// Regression test for https://github.com/shader-slang/slang/issues/12817.
//
// An equality constraint is represented by a named interface requirement key with no separate
// requirement value. Interface lowering previously removed the key's linkage decoration as if
// the key were a method's requirement value. Two equality constraints therefore collided under
// an empty mangled name when a generic conformance was linked.

//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -conformance "ScalarValue:IScalar=0" -o gh-12817.hlsl
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -conformance "ScalarValue:IScalar=0" -o gh-12817.spv
//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -conformance "ScalarValue:IScalar=0" -o gh-12817.cpp
//TEST:INTERPRET(filecheck=CHECK): -entry interpretMain

interface IShaped
{
    associatedtype Scalar;
    associatedtype Mask;
}

interface IAggregate : IShaped
{
    __constraint Scalar : IScalar;
}

interface IScalar : IAggregate
{
    __constraint Scalar == This;
    __constraint Mask == bool;

    int getValue();
}

struct ScalarValue : IScalar
{
    typealias Scalar = This;
    typealias Mask = bool;

    int value;
    int getValue() { return value; }
}

struct AggregateValue<S : IScalar> : IAggregate
{
    typealias Scalar = S;
    typealias Mask = bool;
}

T identity<T : IAggregate>(T value)
{
    return value;
}

T.Scalar asScalar<T : IScalar>(T value)
{
    return value;
}

T asValue<T : IScalar>(T.Scalar value)
{
    return value;
}

T.Mask asMask<T : IScalar>(bool value)
{
    return value;
}

bool asBool<T : IScalar>(T.Mask value)
{
    return value;
}

int readScalar(IScalar value)
{
    return value.getValue();
}

void interpretMain()
{
    AggregateValue<ScalarValue> aggregate;
    aggregate = identity<AggregateValue<ScalarValue>>(aggregate);

    ScalarValue value;
    value.value = 23;
    ScalarValue roundTripValue = asValue<ScalarValue>(asScalar<ScalarValue>(value));
    bool roundTripMask = asBool<ScalarValue>(asMask<ScalarValue>(true));

    // CHECK: 23 1
    printf("%d %d\n", roundTripValue.value, roundTripMask ? 1 : 0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    AggregateValue<ScalarValue> value;
    value = identity<AggregateValue<ScalarValue>>(value);

    ScalarValue scalar;
    scalar.value = 23;
    IScalar dynamicScalar =
        createDynamicObject<IScalar, ScalarValue>(dispatchThreadID.x, scalar);
    int dynamicValue = readScalar(dynamicScalar);
}
