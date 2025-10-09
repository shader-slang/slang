//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK_PTX): -target ptx

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Multi-level traversal intrinsics
//CHECK-DAG: optixGetTransformTypeFromHandle
//CHECK-DAG: optixGetInstanceTraversableFromIAS
//CHECK-DAG: optixGetInstanceIdFromHandle
//CHECK-DAG: optixGetInstanceChildFromHandle
//CHECK-DAG: optixGetInstanceTransformFromHandle
//CHECK-DAG: optixGetInstanceInverseTransformFromHandle

// PTX intrinsics validation
//CHECK_PTX-DAG: _optix_get_transform_type_from_handle
//CHECK_PTX-DAG: _optix_get_instance_traversable_from_ias
//CHECK_PTX-DAG: _optix_get_instance_id_from_handle
//CHECK_PTX-DAG: _optix_get_instance_child_from_handle
//CHECK_PTX-DAG: _optix_get_instance_transform_from_handle
//CHECK_PTX-DAG: _optix_get_instance_inverse_transform_from_handle

struct TestPayload
{
    uint transformType;
    uint instanceId;
    uint64_t transformPtr;
    uint64_t inverseTransformPtr;
};

[shader("raygeneration")]
void rayGen()
{
    TestPayload payload;

    // Test all the new multi-level traversal intrinsics
    OptixTraversableHandle handle = OptixTraversableHandle(0);

    // Test 1: Get transform type
    payload.transformType = (uint)optixGetTransformTypeFromHandle(handle);

    // Test 2: Get instance traversable from IAS
    OptixTraversableHandle instanceHandle = optixGetInstanceTraversableFromIAS(handle, 0);

    // Test 3: Get instance ID
    payload.instanceId = optixGetInstanceIdFromHandle(instanceHandle);

    // Test 4: Get child handle
    OptixTraversableHandle childHandle = optixGetInstanceChildFromHandle(instanceHandle);

    // Test 5 & 6: Get transforms (returned as pointers cast to uint64_t)
    payload.transformPtr = optixGetInstanceTransformFromHandle(instanceHandle);
    payload.inverseTransformPtr = optixGetInstanceInverseTransformFromHandle(instanceHandle);

    // Use the child handle to make sure all intrinsics are called and not optimized out
    payload.instanceId += optixGetInstanceIdFromHandle(childHandle);

    // Write results to output buffer to prevent optimization
    outputBuffer[0] = payload.transformType;
    outputBuffer[1] = payload.instanceId;
    outputBuffer[2] = uint(payload.transformPtr & 0xFFFFFFFF);
    outputBuffer[3] = uint(payload.inverseTransformPtr & 0xFFFFFFFF);
}
