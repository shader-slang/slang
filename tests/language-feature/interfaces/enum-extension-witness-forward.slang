//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// A generic `__EnumType` extension conforms an enum to an interface by forwarding the requirement
// to the enum's `__Tag` type. The forward must resolve to the tag type's own conformance, not to
// itself. It has to be exercised through interface dispatch (which instantiates the enum
// extension's witness table) with both an enum and an `int` flowing through the same interface —
// the two conformances that erase to the same concrete type and must stay distinct.

interface IValue
{
    uint getValue();
}

extension int : IValue
{
    uint getValue() { return asuint(this); }
}

__generic<T : __EnumType>
extension T : IValue
    where T.__Tag : IValue
{
    uint getValue() { return __slang_noop_cast<T.__Tag>(this).getValue(); }
}

enum class MyEnum : int
{
    value = 42,
}

uint consume(IValue value)
{
    return value.getValue();
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // The enum, via the generic extension's forwarding witness.
    // CHECK: 42
    outputBuffer[0] = consume(MyEnum::value);

    // An `int`, via its own `IValue` conformance at the same erased concrete type.
    // CHECK: 7
    outputBuffer[1] = consume(7);
}
