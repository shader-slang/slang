// `bwd_diff(fwd_diff(f))` composes a backward derivative directly over a forward derivative,
// without a `[Differentiable]` function wrapping `fwd_diff(f)`
// (https://github.com/shader-slang/slang/issues/13321). For `f(s) = s * s` at `s = 3` with
// tangent `ds = 1`, the forward derivative computes `(s * s, 2 * s * ds)`. Its backward
// derivative with respect to `(s, ds)` is therefore `(2 * ds, 2 * s) = (2, 6)` for a gradient on
// the output tangent, and `(2 * s, 0) = (6, 0)` for a gradient on the output primal. The
// `IView` conformers cover methods that implement an interface requirement, called directly on
// the concrete type, and the `out`/`inout` functions cover the other parameter directions.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float f(float s) { return s * s; }

interface IView
{
    [Differentiable]
    float read(float s);
}

struct PlainView : IView
{
    [Differentiable]
    float read(float s) { return s * s; }
}

struct CustomView : IView
{
    // The custom derivative of `s * s` deliberately reports `5 * s * ds` rather than
    // `2 * s * ds`, so the output shows that it ran. It is differentiable, so the backward
    // derivative of the forward derivative is `(5 * ds, 5 * s)` for a gradient on the output
    // tangent.
    [Differentiable]
    DifferentialPair<float> read_fwd(DifferentialPair<float> s)
    {
        return diffPair(s.p * s.p, 5.0 * s.p * s.d);
    }

    [ForwardDerivative(read_fwd)]
    [Differentiable]
    float read(float s) { return s * s; }
}

[Differentiable]
float viaPlain(PlainView v, float s) { return v.read(s); }

[Differentiable]
float viaCustom(CustomView v, float s) { return v.read(s); }

[Differentiable]
void sqOut(float x, out float y) { y = x * x; }

[Differentiable]
void sqInout(float x, inout float y) { y = y * x * x; }

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: {{^}}2.0
    // CHECK-NEXT: {{^}}6.0
    var a = diffPair(diffPair(3.0, 1.0));
    bwd_diff(fwd_diff(f))(a, diffPair(0.0, 1.0));
    outputBuffer[0] = a.d.p;
    outputBuffer[1] = a.d.d;

    // CHECK-NEXT: {{^}}6.0
    // CHECK-NEXT: {{^}}0.0
    var b = diffPair(diffPair(3.0, 1.0));
    bwd_diff(fwd_diff(f))(b, diffPair(1.0, 0.0));
    outputBuffer[2] = b.d.p;
    outputBuffer[3] = b.d.d;

    // CHECK-NEXT: {{^}}2.0
    // CHECK-NEXT: {{^}}6.0
    var c = diffPair(diffPair(3.0, 1.0));
    bwd_diff(fwd_diff(viaPlain))(PlainView(), c, diffPair(0.0, 1.0));
    outputBuffer[4] = c.d.p;
    outputBuffer[5] = c.d.d;

    // CHECK-NEXT: {{^}}5.0
    // CHECK-NEXT: {{^}}15.0
    var d = diffPair(diffPair(3.0, 1.0));
    bwd_diff(fwd_diff(viaCustom))(CustomView(), d, diffPair(0.0, 1.0));
    outputBuffer[6] = d.d.p;
    outputBuffer[7] = d.d.d;

    // The primal of the custom derivative is `s * s`, so a gradient on the output primal gives
    // `(2 * s, 0)`.
    // CHECK-NEXT: {{^}}6.0
    // CHECK-NEXT: {{^}}0.0
    var d2 = diffPair(diffPair(3.0, 1.0));
    bwd_diff(fwd_diff(viaCustom))(CustomView(), d2, diffPair(1.0, 0.0));
    outputBuffer[8] = d2.d.p;
    outputBuffer[9] = d2.d.d;

    // `y.d = 2 * x * dx`, so a gradient on the output tangent gives `(2 * dx, 2 * x) = (2, 6)`.
    // CHECK-NEXT: {{^}}2.0
    // CHECK-NEXT: {{^}}6.0
    var e = diffPair(diffPair(3.0, 1.0));
    bwd_diff(fwd_diff(sqOut))(e, diffPair(0.0, 1.0));
    outputBuffer[10] = e.d.p;
    outputBuffer[11] = e.d.d;

    // `y'.d = dy * x * x + 2 * x * y * dx` at `x = 3, dx = 1, y = 2, dy = 0`. A gradient on the
    // output tangent gives `(2 * x * dy + 2 * y * dx, 2 * x * y) = (4, 12)` for `(x, dx)` and
    // `(2 * x * dx, x * x) = (6, 9)` for `(y, dy)`.
    // CHECK-NEXT: {{^}}4.0
    // CHECK-NEXT: {{^}}12.0
    // CHECK-NEXT: {{^}}6.0
    // CHECK-NEXT: {{^}}9.0
    var gx = diffPair(diffPair(3.0, 1.0));
    var gy = diffPair(diffPair(2.0, 0.0), diffPair(0.0, 1.0));
    bwd_diff(fwd_diff(sqInout))(gx, gy);
    outputBuffer[12] = gx.d.p;
    outputBuffer[13] = gx.d.d;
    outputBuffer[14] = gy.d.p;
    outputBuffer[15] = gy.d.d;

    // A non-unit tangent `ds = 2` scales the first entry: `(2 * ds, 2 * s) = (4, 6)`.
    // CHECK-NEXT: {{^}}4.0
    // CHECK-NEXT: {{^}}6.0
    var a2 = diffPair(diffPair(3.0, 2.0));
    bwd_diff(fwd_diff(f))(a2, diffPair(0.0, 1.0));
    outputBuffer[16] = a2.d.p;
    outputBuffer[17] = a2.d.d;

    // With `dy = 0.5`, the `dy` term reaches the `x` gradient: at `x = 3, dx = 1, y = 2`, a
    // gradient on the output tangent gives `(2 * x * dy + 2 * y * dx, 2 * x * y) = (7, 12)` for
    // `(x, dx)` and `(2 * x * dx, x * x) = (6, 9)` for `(y, dy)`.
    // CHECK-NEXT: {{^}}7.0
    // CHECK-NEXT: {{^}}12.0
    // CHECK-NEXT: {{^}}6.0
    // CHECK-NEXT: {{^}}9.0
    var hx = diffPair(diffPair(3.0, 1.0));
    var hy = diffPair(diffPair(2.0, 0.5), diffPair(0.0, 1.0));
    bwd_diff(fwd_diff(sqInout))(hx, hy);
    outputBuffer[18] = hx.d.p;
    outputBuffer[19] = hx.d.d;
    outputBuffer[20] = hy.d.p;
    outputBuffer[21] = hy.d.d;

    // A gradient on the output primal `y' = y * x * x` gives `(2 * x * y, 0) = (12, 0)` for
    // `(x, dx)` and `(x * x, 0) = (9, 0)` for `(y, dy)`.
    // CHECK-NEXT: {{^}}12.0
    // CHECK-NEXT: {{^}}0.0
    // CHECK-NEXT: {{^}}9.0
    // CHECK-NEXT: {{^}}0.0
    var kx = diffPair(diffPair(3.0, 1.0));
    var ky = diffPair(diffPair(2.0, 0.5), diffPair(1.0, 0.0));
    bwd_diff(fwd_diff(sqInout))(kx, ky);
    outputBuffer[22] = kx.d.p;
    outputBuffer[23] = kx.d.d;
    outputBuffer[24] = ky.d.p;
    outputBuffer[25] = ky.d.d;
}
