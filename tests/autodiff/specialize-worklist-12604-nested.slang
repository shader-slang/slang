//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Regression guard for shader-slang/slang#12604 (on-demand specialization path). Drives a nested
// generic-in-generic shape (`scaledSquare<Wrap<Doubler>>`) through `__fwd_diff`, which specializes
// via `specializeChildInsts` (the on-demand path used by the autodiff/translate flow), and checks
// it fully specializes to the correct numeric result. Output-neutral coverage of that path.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

interface IScale
{
    float factor();
}

struct Doubler : IScale
{
    float factor() { return 2.0; }
}

// Inner generic `H`: its concrete instantiation `Wrap<T>` is what a nested `specialize(H, T)`
// produces. Used as the element type of an outer generic below, so the outer generic's argument is
// a type-wrapped nested specialize.
struct Wrap<T : IScale> : IScale
{
    T inner;
    float factor() { return inner.factor(); }
}

// Outer generic `G` over any IScale, forward-differentiable in `x`. When instantiated with
// `Wrap<Doubler>`, the argument is `specialize(Wrap, Doubler)` — a nested specialize reached only
// through the outer specialization. `factor()` is non-differentiable, so `no_diff`.
[ForwardDifferentiable]
float scaledSquare<S : IScale>(S s, float x)
{
    return (no_diff s.factor()) * x * x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Nested generic-in-generic through autodiff: scaledSquare<Wrap<Doubler>>.
    // factor() = 2, f(x) = 2*x*x, f'(x) = 2*2*x = 4*x.
    Wrap<Doubler> w;
    let r = __fwd_diff(scaledSquare)(w, dpfloat(3.0, 1.0));
    outputBuffer[0] = r.p; // CHECK: 18.0
    outputBuffer[1] = r.d; // CHECK: 12.0
}
