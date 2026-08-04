// Positive companion to no-diff-carry-over-call.slang for issue #11285:
// pins the soundness of the readNone-gated Call-rule optimization.
//
// A user-supplied `[BackwardDerivative]` callee with *no* differentiable
// input arguments can still produce a derivative — its user-defined
// derivative is free to read from captured buffers, textures, etc. The
// auto-diff check cannot prove the result is derivative-free just because
// every value-typed arg is `no_diff`, so storing the result into a
// `no_diff` location must still fire E41031.
//
// The Call-rule tightening only applies when the callee is `[__readNone]`;
// for everything else we fall back to "differentiable call always carries".

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

public struct DirSample
{
    public float distance;
}

void bwd_load_value(in uint index, in float.Differential dy)
{
    // User-supplied backward — primal/backward are decoupled, and the
    // analysis cannot prove the derivative is zero from the signature.
}

// Non-readNone: a real load-style function with a user-defined backward.
[BackwardDerivative(bwd_load_value)]
float load_value(uint index)
{
    return 0.0;
}

[Differentiable]
public void store_load_result(no_diff uint index, no_diff out DirSample sample)
{
    sample = {};
    // `load_value(index)` has no differentiable input args (`uint` is
    // non-diff). Under the readNone-gated tightening this falls back to
    // "differentiable call always carries", so the store into the no_diff
    // field must still be diagnosed.
    sample.distance = load_value(index);
}

RWStructuredBuffer<float> out_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    DirSample s;
    store_load_result(0u, s);
    out_buf[0] = s.distance;
}
