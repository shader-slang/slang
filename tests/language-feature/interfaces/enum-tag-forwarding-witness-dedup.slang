//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Regression for https://github.com/shader-slang/slang/issues/12540.
//
// A generic extension can make every enum conform to an interface by forwarding
// the operation to the enum's underlying `__Tag` type:
//
//   __generic<T : __EnumType>
//   extension T : IValue
//       where T.__Tag : IValue
//   {
//       uint getValue() { return __slang_noop_cast<T.__Tag>(this).getValue(); }
//   }
//
// When this conformance is specialized at a concrete enum `T` whose `__Tag` is
// e.g. `int`, the resulting witness table for `T : IValue` is structurally
// identical (same op/type/operands under `IRInstKey::operator==`, which does
// not compare table entries) to `int`'s own, unrelated `IValue` witness table.
// Global value numbering (`_findOrEmitHoistableInst`) then deduplicates the two
// onto a single IR node. Before the fix, `cloneInst` (slang-ir-clone.cpp) would
// still graft the generic extension's `getValue` entry onto that pre-existing,
// already-populated table, leaving two entries for the same requirement key;
// `findWitnessTableEntry`'s first-match lookup could then resolve the forwarded
// call to itself, producing a genuine IR self-call (diagnosed as `E55201:
// recursion not allowed`, or left to corrupt codegen silently depending on the
// call shape). Reaching the conformance only through a concrete-enum direct
// call bypasses the generic instantiation entirely, so this only reproduces
// through an `IValue`-typed value obtained via interface dispatch.

interface IValue
{
    uint getValue();
}

extension int : IValue
{
    uint getValue()
    {
        return asuint(this);
    }
}

__generic<T : __EnumType>
extension T : IValue
    where T.__Tag : IValue
{
    uint getValue()
    {
        return __slang_noop_cast<T.__Tag>(this).getValue();
    }
}

enum class MyEnum : int
{
    value = 42,
};

uint consume(IValue value)
{
    return value.getValue();
}

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = consume(MyEnum::value);
}

// CHECK: 42
