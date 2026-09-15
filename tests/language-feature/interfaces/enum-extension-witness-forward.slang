//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// An enum and `int` can conform to the same interface at the same concrete IR type: enum lowering
// erases `MyEnum` to its tag type `int`, so the enum's generic-extension `IValue` conformance and
// `int`'s own `IValue` conformance share the same (interface, concrete type). Their witness tables
// must stay distinct nodes; if they collapse onto one, interface dispatch on the enum selects the
// wrong `getValue`.
//
// The extension's `getValue` differs from `int`'s by `+ 1` so that a wrong selection is observable
// rather than silent. The three checks pin the three dispatch paths:
//   - enum via interface dispatch must reach the extension's `getValue` (42 + 1 = 43);
//   - plain `int` via interface dispatch must reach `int`'s own `getValue` (7);
//   - enum via a direct call resolves statically to the extension and is the control (43).
// If the two conformances collapse onto one node, enum interface dispatch instead yields 42 while
// the direct call stays 43 — the divergence this test guards against.

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
    uint getValue() { return __slang_noop_cast<T.__Tag>(this).getValue() + 1; }
}

enum class MyEnum : int
{
    value = 42,
}

uint consume(IValue value)
{
    return value.getValue();
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // The enum through interface dispatch: must use the generic extension's forwarding `getValue`.
    // CHECK: 43
    outputBuffer[0] = consume(MyEnum::value);

    // A plain `int` through its own `IValue` conformance at the same erased concrete type.
    // CHECK: 7
    outputBuffer[1] = consume(7);

    // The enum through a direct (statically resolved) call: also the extension's `getValue`. This
    // agrees with interface dispatch only when the two conformances stay distinct.
    // CHECK: 43
    MyEnum e = MyEnum::value;
    outputBuffer[2] = e.getValue();
}
