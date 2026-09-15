//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// An interface belonging to another module is lowered as a declaration and its
// definition supplied by `prelinkIR`, rather than re-derived from the AST in
// every module that mentions it.
//
// The entries are deferred, not dropped, so anything that reads an interface's
// requirement list must still see a complete interface. Two kinds of requirement
// have to keep working, and they reach the consumer by different routes:
//
//  - Built-in requirements -- `IDifferentiable.Differential`, `.dzero`, `.dadd`,
//    and the `Differential : IDifferentiable` conformance witness -- are keyed on
//    an integer role rather than a name. Autodiff looks them up by role and
//    asserts on the requirement count, so a module that conforms to a core
//    interface exercises the path most sensitive to this change.
//  - Requirements with a mangled name, like `IScaled.scaled` in the imported
//    module below, are the ordinary case.
//
// `Vec2` covers the first, `IScaled` covers the second across a module boundary.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import "imported-interface-requirements-lib";

// Hand-written `dzero`/`dadd` rather than synthesized ones, so the built-in
// requirement keys are all present in the witness table.
struct Vec2 : IDifferentiable
{
    float x;
    float y;

    typealias Differential = Vec2;

    [Differentiable]
    static Vec2 dzero()
    {
        Vec2 d;
        d.x = 0.0;
        d.y = 0.0;
        return d;
    }

    [Differentiable]
    static Vec2 dadd(Vec2 a, Vec2 b)
    {
        Vec2 d;
        d.x = a.x + b.x;
        d.y = a.y + b.y;
        return d;
    }
}

[Differentiable]
float lengthSquared(Vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

// Generic over an interface from the other module, so resolving `scaled` goes
// through a named requirement key on an interface this module does not own.
[Differentiable]
float scaleThenSquare<T : IScaled>(T s, float x)
{
    return s.scaled(x) * s.scaled(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    Vec2 v;
    v.x = 3.0;
    v.y = 4.0;

    // Forward mode through a user conformance to a core interface.
    var dpv = diffPair(v, Vec2.dadd(Vec2.dzero(), {1.0, 0.0}));
    outputBuffer[0] = fwd_diff(lengthSquared)(dpv).d;

    // Reverse mode through the same conformance.
    var rpv = diffPair(v);
    bwd_diff(lengthSquared)(rpv, 1.0);
    outputBuffer[1] = rpv.d.x;

    // Forward mode through a named requirement on the imported interface.
    Doubler d;
    var dpx = diffPair(2.0, 1.0);
    outputBuffer[2] = fwd_diff(scaleThenSquare<Doubler>)(d, dpx).d;

    // Reverse mode through that same named requirement. Reverse-mode
    // transcription reaches an interface's requirement list by a different route
    // than forward mode, so the borrowed interface has to hold up under both.
    var rpx = diffPair(2.0);
    bwd_diff(scaleThenSquare<Doubler>)(d, rpx, 1.0);
    outputBuffer[3] = rpx.d;

    outputBuffer[4] = lengthSquared(v);
}

// d/dx (x^2 + y^2) at (3,4) along (1,0) = 2*3*1 = 6
//CHECK: 6.0
// d/dx (x^2 + y^2) at x=3 = 6
//CHECK-NEXT: 6.0
// d/dx (2x)^2 = 8x, at x=2 -> 16
//CHECK-NEXT: 16.0
// the same derivative taken in reverse mode
//CHECK-NEXT: 16.0
// 3^2 + 4^2 = 25
//CHECK-NEXT: 25.0
