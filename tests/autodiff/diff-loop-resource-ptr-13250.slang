// Regression repro for https://github.com/shader-slang/slang/issues/13250.
//
// Disabled because the fix is not yet in tree. Re-enable it (change
// DISABLE_TEST back to TEST) once the fix lands, so it guards against
// regression.
//
// Reverse-mode differentiation of a function that reads a resource-backed
// tensor through an interface (existential) inside a loop checkpoints the
// resource handle into the backward-pass context. Under SPIR-V logical
// addressing an OpVariable may not allocate a pointer type, so the handle
// materializes as an illegal Function-storage variable whose pointee is a
// logical pointer to a resource
// (OpVariable %_ptr_Function__ptr_Uniform_RWStructuredBuffer Function).
// Compilation succeeds (rc=0), so the invalid SPIR-V is emitted silently.
//
// This asserts on the emitted SPIR-V assembly text rather than on a validation
// return code: the in-tree spirv-val predates logical-pointer validation.
// Related: #9062 (array-of-resource-pointer autodiff variant).

//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -profile spirv_1_3

interface ITensor
{
    [Differentiable] float load(int index[2]);
    void accumulate(float gradient, int r, int c);
}
extension<T : ITensor> T
{
    [Differentiable]
    [BackwardDerivative(loadBackward)]
    float load(int r, int c) { return load(int[2](r, c)); }
    void loadBackward(int r, int c, float gradient) { accumulate(gradient, r, c); }
}
struct Tensor : ITensor
{
    StructuredBuffer<float> values;
    RWByteAddressBuffer gradients;
    [Differentiable]
    [BackwardDerivative(loadBackward)]
    float load(int index[2]) { return values[int(index[0]) * 2 + int(index[1])]; }
    void loadBackward(int index[2], float gradient) { accumulate(gradient, int(index[0]), int(index[1])); }
    void accumulate(float gradient, int r, int c) { gradients.InterlockedAddF32((r * 2 + c) * 4, gradient); }
}
[Differentiable]
float[2] weightedSum(ITensor weights, float x[2])
{
    float result[2];
    for (int r = 0; r < 2; ++r)
    {
        float sum = 0;
        for (int c = 0; c < 2; ++c)
            sum += weights.load(r, c) * x[c];
        result[r] = sum;
    }
    return result;
}
StructuredBuffer<float> values;
RWByteAddressBuffer gradients;
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Tensor t = { values, gradients };
    var x = diffPair(float[2](2, 5));
    bwd_diff(weightedSum)(t, x, float[2](1, 1));
}

// The module must be emitted at all, so the negative check below cannot pass
// vacuously on empty output:
// CHECK: OpEntryPoint
// CHECK-NOT: OpVariable %_ptr_Function__ptr_
