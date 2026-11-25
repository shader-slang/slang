// A `[PreferRecompute]` call that is passed the address of a global for a borrow parameter is
// recomputed in the backward pass, as the callee asks, rather than stored in the context.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -line-directive-mode none

static float4 gv;

[PreferRecompute]
[__NoSideEffect]
float h(__constref float4 v)
{
    return v.x;
}

[Differentiable]
float f(float x)
{
    return x * no_diff(h(gv));
}

RWStructuredBuffer<float> o;

[numthreads(1, 1, 1)]
void main()
{
    gv = float4(o[1], 0, 0, 0);
    var x = diffPair(o[0]);
    bwd_diff(f)(x, 1.0);
    o[2] = x.d;
}

// CHECK-LABEL: void s_bwdProp_f
// CHECK: h_0(gv_0)
