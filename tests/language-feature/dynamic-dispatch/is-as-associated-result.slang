// Regression test for https://github.com/shader-slang/slang/issues/13261
//
// `is`/`as` applied to a value whose static type is an *inferred associated type*
// of a dynamically-dispatched result must perform a runtime type check, not fold to
// a constant. Before the fix, `let value = createDynamicObject<IFactory>(...).make(n)`
// gave `value` the static type `IFactory.Result` (an associated type, not the bound
// `IValue`); the checker only opened an existential when the static type was literally
// an interface, so `value is Value` / `value as Value` emitted a static
// `TypeEquals(IFactory.Result, Value)` that constant-folded to false across every
// target, silently. `value.read()` (interface-method dispatch) always worked, which is
// why the miscompile was silent. Runtime dispatch is forced via createDynamicObject +
// two type_conformances so the result cannot be specialized to a single concrete type.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

interface IValue
{
    uint read();
}

struct Value : IValue
{
    uint n;
    uint read() { return n; }
}

struct OtherValue : IValue
{
    uint n;
    uint read() { return n; }
}

interface IFactory
{
    associatedtype Result : IValue;
    Result make(uint n);
}

struct Factory : IFactory
{
    typealias Result = Value;
    uint pad; // data field for createDynamicObject AnyValue packing
    Value make(uint n) { return Value(n); }
}

struct OtherFactory : IFactory
{
    typealias Result = OtherValue;
    uint pad; // data field for createDynamicObject AnyValue packing
    OtherValue make(uint n) { return OtherValue(n); }
}

//TEST_INPUT: type_conformance Factory:IFactory = 0
//TEST_INPUT: type_conformance OtherFactory:IFactory = 1

IFactory createFactory(uint id)
{
    return createDynamicObject<IFactory>(id, 0u);
}

// `value` has static type `IFactory.Result` (the inferred associated type), not `IValue`.
uint4 probe(uint id, uint n)
{
    let value = createFactory(id).make(n);
    uint asValue = 0;
    if (let v = value as Value)
        asValue = v.read();
    return uint4(
        value.read(),              // interface-method dispatch: always correct
        (uint)(value is Value),    // runtime type check
        asValue,                   // runtime cast payload
        (uint)(value is OtherValue));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Factory -> Value: read()=37, is Value=1, (as Value).read()=37, is OtherValue=0
    uint4 a = probe(0, 37);
    outputBuffer[0] = a.x;
    outputBuffer[1] = a.y;
    outputBuffer[2] = a.z;
    outputBuffer[3] = a.w;
    // CHECK: 37
    // CHECK-NEXT: 1
    // CHECK-NEXT: 37
    // CHECK-NEXT: 0

    // OtherFactory -> OtherValue: read()=38, is Value=0, (as Value)=none->0, is OtherValue=1
    uint4 b = probe(1, 38);
    outputBuffer[4] = b.x;
    outputBuffer[5] = b.y;
    outputBuffer[6] = b.z;
    outputBuffer[7] = b.w;
    // CHECK-NEXT: 38
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
}
