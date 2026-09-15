//TEST:INTERPRET(filecheck=CHECK):

// Regression for shader-slang/slang#12871. Keep the `(int)` casts of the autodiff result: the bug
// was a signed-int cast in the byte-code interpreter (a negative float→int printed "0 0" on aarch64),
// so simplifying these to `%f` would silently drop the coverage. The autodiff itself is correct.

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
