//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK_PTX): -target ptx

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Multi-level traversal intrinsics - check for OptiX function calls in CUDA output
//CHECK-DAG: optixGetTransformTypeFromHandle
//CHECK-DAG: optixGetInstanceTraversableFromIAS
//CHECK-DAG: optixGetInstanceIdFromHandle
//CHECK-DAG: optixGetInstanceChildFromHandle
//CHECK-DAG: _slang_optixGetInstanceTransformFromHandle
//CHECK-DAG: _slang_optixGetInstanceInverseTransformFromHandle

// PTX intrinsics validation
//CHECK_PTX-DAG: _optix_get_transform_type_from_handle
//CHECK_PTX-DAG: _optix_get_instance_traversable_from_ias
//CHECK_PTX-DAG: _optix_get_instance_id_from_handle
//CHECK_PTX-DAG: _optix_get_instance_child_from_handle
//CHECK_PTX-DAG: _optix_get_instance_transform_from_handle
//CHECK_PTX-DAG: _optix_get_instance_inverse_transform_from_handle

// Transform list intrinsics (context-dependent, only in hit shaders)
//CHECK-DAG: optixGetTransformListSize
//CHECK-DAG: optixGetTransformListHandle

//CHECK_PTX-DAG: _optix_get_transform_list_size
//CHECK_PTX-DAG: _optix_get_transform_list_handle

struct TestPayload
{
    TRAVERSABLE_TRANSFORM_TYPE transformType;
    uint instanceId;
    float3x4 transform;
    float3x4 inverseTransform;
};

[shader("raygeneration")]
void rayGen()
{
    TestPayload payload;

    // Test all the new multi-level traversal intrinsics
    // Note: Using uint64_t for traversable handles (maps to OptixTraversableHandle in CUDA)
    uint64_t handle = 0;

    // Test 1: Get transform type
    payload.transformType = GetTraversableTransformType(handle);

    // Test 2: Get instance traversable from IAS
    uint64_t instanceHandle = GetInstanceTraversable(handle, 0);

    // Test 3: Get instance ID
    payload.instanceId = GetTraversableInstanceId(instanceHandle);

    // Test 4: Get child handle
    uint64_t childHandle = GetTraversableChild(instanceHandle);

    // Test 5 & 6: Get transforms (returned as float3x4 matrices)
    payload.transform = GetTraversableObjectToWorld(instanceHandle);
    payload.inverseTransform = GetTraversableWorldToObject(instanceHandle);

    // Use the child handle to make sure all intrinsics are called and not optimized out
    payload.instanceId += GetTraversableInstanceId(childHandle);

    // Write results to output buffer to prevent optimization
    outputBuffer[0] = payload.transformType;
    outputBuffer[1] = payload.instanceId;
    // Write first element of each transform matrix
    outputBuffer[2] = asuint(payload.transform[0][0]);
    outputBuffer[3] = asuint(payload.inverseTransform[0][0]);
}

struct HitPayload
{
    uint transformListSize;
    uint64_t firstHandle;
};

[shader("closesthit")]
void closestHit(inout HitPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    // Test 7: Get transform list size (context-dependent, only valid in hit shaders)
    payload.transformListSize = GetTransformListSize();

    // Test 8: Get transform list handle (context-dependent, only valid in hit shaders)
    // Get the root handle (index 0)
    if (payload.transformListSize > 0)
    {
        payload.firstHandle = GetTransformListHandle(0);
    }
}
