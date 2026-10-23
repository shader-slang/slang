//TEST:SIMPLE: -target spirv -o overload-constraint-suggestions-valid.spv

// Check that the suggested constraints compile without diagnostics.

interface IBase {}
interface IDerived : IBase {}
interface IUnrelated {}

// Check that the rejected IDerived overload produces no hint when the IBase overload succeeds.
void selected<U : IDerived>(U value) {}
void selected<U : IBase>(U value) {}
void validSelection<T : IBase>(T value) { selected(value); }

void derivedOp<U : IDerived>(U value) {}
void unrelatedOp<U : IUnrelated>(U value) {}
void strengthened<T : IDerived>(T value) { derivedOp(value); }
void conjoined<T : IBase & IUnrelated>(T value) { unrelatedOp(value); }
void preserved<T : IUnrelated & IDerived>(T value) { derivedOp(value); }
void coreScalar<T : __BuiltinFloatingPointType>(T value) { abs(value); }

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}

// Check that the suggested outer constraint compiles when the method shadows the interface name.
struct Outer<T : IDerived>
{
    void method<IDerived>(T value) { derivedOp(value); }
}
