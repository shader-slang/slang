//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Pins the nested-in-generic differentiation shape end-to-end: the only
// auto-diff translation request in this module lives INSIDE the generic body
// of `derivInGeneric` — `__fwd_diff(genericSquare)` applied to a `T`-dependent
// argument lowers to `ForwardDifferentiate(specialize(genericSquare, T))`,
// which depends on `T` and therefore hoists only into the enclosing generic,
// never to module scope. This is the program shape that motivates the
// recursive walk in `doesModuleUseAutodiff` (slang-ir-link.cpp); note the
// walk's necessity is otherwise pinned broadly — a module-scope-only scan
// fails a large portion of tests/autodiff/ — so what this test adds is an
// explicit minimal example of the nested request compiling and producing the
// correct derivative, including the `where T.Differential == T` constraint
// needed to state it.

typedef DifferentialPair<float> dpfloat;

[ForwardDifferentiable]
T genericSquare<T : __BuiltinFloatingPointType>(T x)
{
    return x * x;
}

T derivInGeneric<T : __BuiltinFloatingPointType>(T x)
    where T.Differential == T
{
    let dp = __fwd_diff(genericSquare)(DifferentialPair<T>(x, T(1)));
    return dp.d;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // d/dx (x*x) at x = 3 is 6.
    outputBuffer[0] = derivInGeneric<float>(3.0);
    // CHECK: 6.0
}
