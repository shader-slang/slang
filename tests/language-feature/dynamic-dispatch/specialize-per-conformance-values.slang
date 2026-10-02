//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

// Deliberately compare the existing copy-in/copy-out behavior for aliased state
// arguments against the annotated helper, without prescribing an alias ordering.
#pragma warning(disable:30051)

interface IValue
{
    uint get();
    [mutating] void increment();
}

struct A : IValue
{
    uint value;
    uint get() { return value; }
    [mutating] void increment() { value++; }
}

struct B : IValue
{
    uint value;
    uint get() { return value * 2; }
    [mutating] void increment() { value += 2; }
}

[__specializePerConformance]
void consume<T : IValue, let Bias : uint = 4>(T value, inout uint result)
{
    result = value.get() + Bias;
}

// These shapes deliberately retain the normal specialization path. The attribute
// must not change return values, mutation of a dynamic payload, or independent tags.
[__specializePerConformance]
uint returnValue<T : IValue>(T value)
{
    return value.get();
}

[__specializePerConformance]
void mutate<T : IValue>(inout T value)
{
    value.increment();
}

void mutateValue<T : IValue>(T value, out uint result)
{
    mutate(value);
    result = value.get();
}

[__specializePerConformance]
void pair<AType : IValue, BType : IValue>(AType a, BType b, out uint result)
{
    result = a.get() * 10 + b.get();
}

[__specializePerConformance]
void aliasState<T : IValue>(T value, inout uint a, inout uint b)
{
    a += value.get();
    b += a;
}

void ordinaryAliasState<T : IValue>(T value, inout uint a, inout uint b)
{
    a += value.get();
    b += a;
}

//TEST_INPUT: type_conformance A:IValue = 3
//TEST_INPUT: type_conformance B:IValue = 7
//TEST_INPUT:ubuffer(data=[3 7], stride=4):name=ids
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
StructuredBuffer<uint> ids;
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let value = createDynamicObject<IValue>(ids[tid.x], 5u);
    uint result = 0;
    consume(value, result);
    uint offset = tid.x * 5;
    outputBuffer[offset] = result;
    outputBuffer[offset + 1] = returnValue(value);
    mutateValue(value, result);
    outputBuffer[offset + 2] = result;
    pair(value, createDynamicObject<IValue>(ids[1 - tid.x], 2u), result);
    outputBuffer[offset + 3] = result;
    uint annotated = 1;
    uint ordinary = 1;
    aliasState(value, annotated, annotated);
    ordinaryAliasState(value, ordinary, ordinary);
    outputBuffer[offset + 4] = annotated == ordinary ? 1 : 0;
}

// CHECK: 9
// CHECK: 5
// CHECK: 6
// CHECK: 54
// CHECK: 1
// CHECK: 14
// CHECK: 10
// CHECK: 14
// CHECK: 102
// CHECK: 1
