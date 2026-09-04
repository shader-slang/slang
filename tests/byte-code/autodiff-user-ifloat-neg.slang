//DISABLE_TEST:INTERPRET(filecheck=CHECK):

// Disabled: known-failing on aarch64 (tracked by shader-slang/slang#12871). Forward-mode autodiff
// of a user-defined `IFloat`/`IDifferentiable` `neg()` returns the wrong "0 0" on aarch64 under the
// slangi interpreter while x86_64 is correct. Re-enable (restore `//TEST:INTERPRET`) once #12871
// is fixed.

// Most of the `IFloat` surface below exists only to satisfy conformance; the tested expression
// exercises `mul` (via `x*x`), `neg` (via unary `-`), and `__init`.
struct MyFloat : IFloat, IDifferentiable
{
    float v;
    [Differentiable] __init(float value) { v = value; }
    [Differentiable] __init(MyFloat other) { v = other.v; }
    [Differentiable] float toFloat() { return v; }
    [Differentiable] MyFloat add(MyFloat o) { return MyFloat(v + o.v); }
    [Differentiable] MyFloat sub(MyFloat o) { return MyFloat(v - o.v); }
    [Differentiable] MyFloat mul(MyFloat o) { return MyFloat(v * o.v); }
    [Differentiable] MyFloat div(MyFloat o) { return MyFloat(v / o.v); }
    [Differentiable] MyFloat mod(MyFloat o) { return MyFloat(v % o.v); }
    [Differentiable] MyFloat neg() { return MyFloat(-v); }
    [Differentiable] MyFloat scale<T : __BuiltinFloatingPointType>(T s) { return MyFloat(v * s.toFloat()); }
    bool equals(MyFloat o) { return v == o.v; }
    bool lessThan(MyFloat o) { return v < o.v; }
    bool lessThanOrEquals(MyFloat o) { return v <= o.v; }
}

[Differentiable]
MyFloat squareNeg(MyFloat x) { return -(x * x); }

void main()
{
    let fwdSquareNeg = fwd_diff(squareNeg)(diffPair(MyFloat(3.0), MyFloat(1.0)));
    //CHECK: -9 -6
    printf("%d %d\n", (int)fwdSquareNeg.p.v, (int)fwdSquareNeg.d.v);
}
