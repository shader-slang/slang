// Check that callable entry-point parameters are lowered correctly for each backend.

//TEST:SIMPLE(filecheck=SPIRV_INOUT): -target spirv-assembly -entry callableInOut -stage callable -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=SPIRV_INOUT): -target spirv -entry callableInOut -stage callable
//TEST:SIMPLE(filecheck=DXIL_INOUT): -target dxil-assembly -entry callableInOut -stage callable -profile sm_6_5
//TEST:SIMPLE(filecheck=CUDA_INOUT): -target cuda -entry callableInOut -stage callable

// DXC requires callable payload parameters to be `inout`, while SPIR-V and CUDA also support
// `out`. Keep the latter cases here to cover both mutable-parameter legalization paths.
//TEST:SIMPLE(filecheck=SPIRV_OUT): -target spirv-assembly -entry callableOut -stage callable -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=SPIRV_OUT): -target spirv -entry callableOut -stage callable
//TEST:SIMPLE(filecheck=CUDA_OUT): -target cuda -entry callableOut -stage callable

struct Payload
{
    int value;
};

void callableInOut(inout Payload payload)
{
    payload.value += 1;
}

void callableOut(out Payload payload)
{
    payload.value = 2;
}

// SPIRV_INOUT: OpEntryPoint Callable{{NV|KHR}}
// SPIRV_INOUT-DAG: OpTypePointer IncomingCallableData{{NV|KHR}}
// SPIRV_INOUT-DAG: OpVariable %{{.*}} IncomingCallableData{{NV|KHR}}

// SPIRV_OUT: OpEntryPoint Callable{{NV|KHR}}
// SPIRV_OUT-DAG: OpTypePointer IncomingCallableData{{NV|KHR}}
// SPIRV_OUT-DAG: OpVariable %{{.*}} IncomingCallableData{{NV|KHR}}

// DXIL_INOUT: define void @{{.*}}callableInOut{{.*}}(%struct.Payload_0* {{.*}}payload_0)

// CUDA_INOUT-LABEL: extern "C" __device__ void __direct_callable__callableInOut(
// CUDA_INOUT-SAME: Payload_0 * payload_0)

// CUDA_OUT-LABEL: extern "C" __device__ void __direct_callable__callableOut(
// CUDA_OUT-SAME: Payload_0 * payload_0)
