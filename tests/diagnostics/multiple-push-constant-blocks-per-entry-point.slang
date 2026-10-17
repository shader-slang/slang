// Vulkan allows at most one push-constant block per entry point. A global
// `[[vk::push_constant]]` block and the push-constant block that holds an
// entry point's `uniform` parameters both start at offset 0, so an entry point
// that reads both must be rejected instead of producing invalid SPIR-V/GLSL
// (shader-slang/slang#13713).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -emit-spirv-via-glsl -entry main -stage compute

struct PC
{
    float4 a;
}

[[vk::push_constant]]
uniform PC push;
//CHECK:   ^^^^ push-constant block `push` is used by this entry point

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID, uniform float k, uniform float2 scale)
/*CHECK:
     ^^^^ error E39033
     ^^^^ entry point `main` uses more than one push-constant block, but Vulkan allows at most one per entry point
     ^^^^ the push-constant block that holds the `uniform` parameters `k`, `scale` of `main` is used by this entry point
     ^^^^ combine the data into one
*/
{
    outBuf[tid.x] = push.a.x + k * scale.y + tid.x;
}
