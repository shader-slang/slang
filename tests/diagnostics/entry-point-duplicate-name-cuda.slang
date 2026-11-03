// CUDA writes all kernels into one namespace but prefixes the symbol of an OptiX ray-tracing
// kernel with its program type (shader-slang/slang#14053). A ray generation and a miss shader can
// therefore share a source name, while a compute kernel whose name already carries the
// ray-generation prefix collides with a ray generation shader of the unprefixed name. SPIR-V
// accepts the ray generation and miss pair because it keeps one namespace per execution model.

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
     ^^^ the other entry point emitted as '__raygen__Foo' is here
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
     ^^^^^^^^^^^^^ more than one entry point would be emitted as '__raygen__Foo' for target 'cuda'
     ^^^^^^^^^^^^^ this entry point would also be emitted as '__raygen__Foo'
*/
{
    outBuf[0] = 1;
}
#endif
