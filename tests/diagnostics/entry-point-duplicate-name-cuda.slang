// CUDA prefixes the symbol of an OptiX ray-tracing entry point with its program type
// (shader-slang/slang#14053). A ray generation and a miss shader can therefore share a source
// name, but a compute kernel whose name already carries the ray-generation prefix collides with
// a ray generation shader of the unprefixed name. SPIR-V keeps the two ray-tracing stages in
// separate namespaces as well.

//TEST:SIMPLE(filecheck=CUDA):-target cuda
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -DCOLLIDE

// CUDA-DAG: __global__ void __raygen__Foo()
// CUDA-DAG: __global__ void __miss__Foo()
// SPIRV-DAG: OpEntryPoint RayGenerationKHR %{{.*}} "Foo"
// SPIRV-DAG: OpEntryPoint MissKHR %{{.*}} "Foo"

struct Payload
{
    float value;
};

[shader("raygeneration")]
void Foo()
/*CHECK:
     ^^^ see the other entry point named '__raygen__Foo'
*/
{
}

[shader("miss")]
void Foo(inout Payload payload)
{
    payload.value = 1;
}

#if defined(COLLIDE)
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void __raygen__Foo(uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^^^^^^^^^ more than one entry point is named '__raygen__Foo' in the output for target 'cuda'
     ^^^^^^^^^^^^^ entry point named '__raygen__Foo' conflicts with another entry point
*/
{
    outBuf[0] = 1;
}
#endif
