//TEST:SIMPLE(filecheck=CHECK): -target cuh -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none

// The CUDA header declares `__extern_cpp` OptiX programs with the same
// signatures as their definitions in the `.cu` (see #14033). Ray-tracing
// stages receive their payload, attributes and uniforms through OptiX
// intrinsics and the shader binding table, so their kernels take no
// parameters; a direct callable keeps its pointer parameter and is a
// `__device__` function.

//CHECK-DAG: extern "C" __global__ void __raygen__rgMain();
//CHECK-DAG: extern "C" __global__ void __closesthit__chMain();
//CHECK-DAG: extern "C" __device__ void __direct_callable__callMain(Payload{{[[:alnum:]_]*}} * {{[[:alnum:]_]+}});

//CUDA-DAG: extern "C" __global__ void __raygen__rgMain()
//CUDA-DAG: extern "C" __global__ void __closesthit__chMain()
//CUDA-DAG: extern "C" __device__ void __direct_callable__callMain(Payload{{[[:alnum:]_]*}} * {{[[:alnum:]_]+}})

struct Payload
{
    float4 color;
};

[shader("raygeneration")]
__extern_cpp void rgMain(uniform float scale)
{
}

[shader("closesthit")]
__extern_cpp void chMain(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = float4(attr.barycentrics, 0, 1);
}

[shader("callable")]
__extern_cpp void callMain(inout Payload payload)
{
    payload.color = float4(1, 2, 3, 4);
}
