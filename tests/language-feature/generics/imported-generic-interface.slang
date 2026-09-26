//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// A *generic* interface owned by another module. Lowering borrows it from that
// module rather than re-deriving it, and the generic shape is what makes the
// borrow non-trivial: the symbol found under the mangled name is the outer
// `IRGeneric`, so it has to be unwrapped to recognise the interface, and the
// declaration emitted locally has to be wrapped in a matching generic or
// `prelinkIR` could not pair the two by name.
//
// The generic machinery itself is not new coverage -- `imported-interface-
// requirements.slang` already borrows the generic core-module interfaces
// `IForwardDifferentiable`, `IBackwardDifferentiable` and `IBwdCallable`. What is
// new here is where the definition comes from: `prelinkIR` draws on the core
// modules and on `getModuleDependencies()` separately, so a generic interface
// borrowed from an ordinary imported module resolves through a different source
// than one borrowed from a core module.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import "imported-generic-interface-lib";

// Constrained on the imported generic interface at a concrete argument, so
// resolving `get` and `count` goes through requirement keys on a specialization
// of an interface this module does not own.
float sumOf<C : IContainer<float>>(C c)
{
    float total = 0.0;
    for (uint i = 0; i < c.count(); ++i)
        total += c.get(i);
    return total;
}

// Also use the interface generically over its own type parameter, so the
// borrowed generic is specialized here at more than one argument.
T firstOf<T, C : IContainer<T>>(C c)
{
    return c.get(0);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    Doubling<float> d = Doubling<float>(2.5);

    // count() == 4, get(i) == 5.0 each -> 20.0
    outputBuffer[0] = sumOf<Doubling<float> >(d);

    outputBuffer[1] = firstOf<float, Doubling<float> >(d);

    Doubling<float> e = Doubling<float>(1.0);
    outputBuffer[2] = sumOf<Doubling<float> >(e);
    outputBuffer[3] = firstOf<float, Doubling<float> >(e);
}

// 4 * (2.5 + 2.5) = 20
//CHECK: 20.0
// 2.5 + 2.5 = 5
//CHECK-NEXT: 5.0
// 4 * (1 + 1) = 8
//CHECK-NEXT: 8.0
// 1 + 1 = 2
//CHECK-NEXT: 2.0
