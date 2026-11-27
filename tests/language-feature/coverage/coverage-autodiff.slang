// A function that is differentiated compiles under the counting coverage
// modes. Coverage rewrites its markers into atomic counter increments before
// autodiff runs, so the derivative functions carry those atomics as
// non-differentiable side effects. The forward derivative runs the primal
// code and keeps them. A backward derivative runs them only when it needs
// primal values, which this one does not, so only the forward derivative is
// checked.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage -trace-function-coverage -trace-branch-coverage

RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float scaled(float x)
{
    if (x > 1.0f)
        return x * x;
    return x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let forward = fwd_diff(scaled)(diffPair(outputBuffer[0], 1.0f));
    var backward = diffPair(outputBuffer[1], 0.0f);
    bwd_diff(scaled)(backward, 1.0f);
    outputBuffer[2] = forward.d + backward.d;
}

//CHECK-LABEL: s_fwd_scaled_0(DiffPair_float_0
//CHECK: _slang_atomic_add_u64{{.*}}_slang_coverage
//CHECK-LABEL: void _computeMain(
