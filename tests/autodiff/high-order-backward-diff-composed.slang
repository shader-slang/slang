// `bwd_diff(fwd_diff(f))` composes a backward derivative directly over a forward derivative,
// without a `[Differentiable]` function wrapping `fwd_diff(f)`. For `f(s) = s * s` at `s = 3`
// with tangent `ds = 1`, the forward derivative computes `(s * s, 2 * s * ds)`. Its backward
// derivative with respect to `(s, ds)` is therefore `(2 * ds, 2 * s) = (2, 6)` for a gradient on
// the output tangent, and `(2 * s, 0) = (6, 0)` for a gradient on the output primal.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=4):out,name=outputBuffer
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
    // A differentiable custom derivative of `s * s` that reports `5 * s * ds`, so the backward
    // derivative of the forward derivative is `(5 * ds, 5 * s) = (5, 15)`.
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
}
