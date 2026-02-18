//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl -ignore-capabilities

// CHECK_IGNORE_CAPS-NOT: error
SamplerState samplerState;
Texture2D<int> rw;

[numthreads(1,1,1)]
void computeMain()
/*CHECK:
     ^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^ entrypoint 'computeMain' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    clip(0.0f); // clip is not supported in compute shader, so this throws an error.
/*CHECK:
    ^^^^ see using of 'clip'
*/
}