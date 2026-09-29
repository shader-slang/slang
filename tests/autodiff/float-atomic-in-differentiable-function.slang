// A float atomic inside a differentiated function is non-differentiable like
// the integer ones: its value operand depends on the differentiable input,
// but no derivative flows through the atomic, which `no_diff` acknowledges.
// Float atomic add is not available on every target, so this test compiles
// for targets that have it.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain

RWStructuredBuffer<float> floatBuffer;
RWStructuredBuffer<Atomic<float>> accumulator;

[Differentiable]
float f(float x)
{
    float original = no_diff accumulator[0].add(x * 2.0f);
    float interlockedOriginal;
    no_diff InterlockedAdd(floatBuffer[3], x, interlockedOriginal);
    accumulator[1].add(interlockedOriginal);
    return x * x + original + interlockedOriginal;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let forward = fwd_diff(f)(diffPair(floatBuffer[0], 1.0f));
    var backward = diffPair(floatBuffer[1], 0.0f);
    bwd_diff(f)(backward, 1.0f);
    floatBuffer[2] = forward.d + backward.d;
}

// SPIRV: OpAtomicFAddEXT

// The forward derivative keeps the atomic and feeds it the primal value only.
// CUDA: DiffPair_float_0 s_fwd_f_0(DiffPair_float_0
// CUDA: atomicAdd({{.*}}primal_0 * 2.0f)
