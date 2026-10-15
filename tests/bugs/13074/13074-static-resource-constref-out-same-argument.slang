//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target cpp -entry computeMain -stage compute -validate-ir -Wno-30051

// The call below intentionally aliases an `__constref` parameter with an `out` parameter. We
// suppress the unrelated aliasing warning so that exhaustive diagnostic matching isolates the
// uninitialized-value analysis. The C++ target can represent the generated resource local.

Texture2D<float4> textures[1];
RWStructuredBuffer<float4> output;
static Texture2D<float4> selectedTexture;

[noinline]
void initializeSelectedTexture(
    __constref Texture2D<float4> unusedInput,
    out Texture2D<float4> result)
{
    result = textures[0];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // We pass the same resource variable to parameters with different directions. The module-wide
    // uninitialized-value check preserves each exact argument use, so it classifies the second
    // `IRUse` from the `out` parameter instead of classifying both uses from the first `__constref`
    // parameter.
//CHECK-NOT: use of uninitialized global variable
    initializeSelectedTexture(selectedTexture, selectedTexture);
//CHECK:                     ^ use of uninitialized variable
//CHECK:                     ^ use of uninitialized variable 'selectedTexture'
//CHECK-NOT: use of uninitialized global variable

    // The later per-entry analysis conservatively treats the `__constref` argument as a read before
    // the `out` copy-back, which accounts for the diagnostic above. Because the `out` parameter
    // assigns the whole value on every normal return, the following read is initialized.
    output[0] = selectedTexture.Load(int3(0, 0, 0));
}
