// Required surface inlining must reject recursion even when optional validation is disabled.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -disable-non-essential-validations -O0

RWTexture2D<float> image;
RWStructuredBuffer<float> output;
[noinline] float readRecursive(RWTexture2D<float> value, uint depth)
{
    if (depth == 0) return value[int2(0)];
    return readRecursive(value, depth - 1);
//CHECK:                ^ recursion not allowed
//CHECK:                ^ recursion detected in call to 'readRecursive', but the current code generation target does not allow recursion.
}
[numthreads(1,1,1)]
void computeMain(uniform uint depth) { output[0] = readRecursive(image, depth); }
