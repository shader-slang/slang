//TEST:SIMPLE(filecheck=CHECK-COMPUTE): -target spirv -stage compute -entry testCompute
//TEST:SIMPLE(filecheck=CHECK-VERTEX): -target spirv -stage vertex -entry testVertex
//TEST:SIMPLE(filecheck=CHECK-FRAGMENT): -target spirv -stage fragment -entry testFragment

// Comprehensive test for ALL SV_* semantics in wrong stages
// This test systematically checks that each semantic is properly restricted

// CHECK-COMPUTE: error 40015
// CHECK-VERTEX: error 40015
// CHECK-FRAGMENT: error 40015

// ============================================================================
// Fragment-only semantics that should error in compute
// ============================================================================

[shader("compute")]
[numthreads(1,1,1)]
void testCompute(
    // Fragment-only semantics - should all error in compute
    float2 pointCoord : SV_PointCoord,           // CHECK-COMPUTE: error
    uint sampleIndex : SV_SampleIndex,           // CHECK-COMPUTE: error
    float2 samplePos : SV_VulkanSamplePosition,  // CHECK-COMPUTE: error
    int fragInvCount : SV_FragInvocationCount,   // CHECK-COMPUTE: error
    int2 fragSize : SV_FragSize                  // CHECK-COMPUTE: error
) { }

// ============================================================================
// Vertex-only input semantics that should error in fragment
// ============================================================================

[shader("fragment")]
float4 testFragment(
    // Vertex-only input semantics - should all error in fragment
    uint vertexID : SV_VertexID,                       // CHECK-FRAGMENT: error
    uint instanceID : SV_InstanceID,                   // CHECK-FRAGMENT: error
    uint vulkanVertexID : SV_VulkanVertexID,           // CHECK-FRAGMENT: error
    uint vulkanInstanceID : SV_VulkanInstanceID,       // CHECK-FRAGMENT: error
    int startVertex : SV_StartVertexLocation,          // CHECK-FRAGMENT: error
    int startInstance : SV_StartInstanceLocation,      // CHECK-FRAGMENT: error
    int drawIndex : SV_DrawIndex                       // CHECK-FRAGMENT: error
) : SV_Target
{
    return float4(0, 0, 0, 1);
}

// ============================================================================
// Compute-only semantics that should error in vertex
// ============================================================================

[shader("vertex")]
float4 testVertex(
    // Compute-only semantics - should all error in vertex
    uint3 dispatchThreadID : SV_DispatchThreadID,  // CHECK-VERTEX: error
    uint3 groupID : SV_GroupID,                    // CHECK-VERTEX: error
    uint3 groupThreadID : SV_GroupThreadID,        // CHECK-VERTEX: error
    uint groupIndex : SV_GroupIndex                // CHECK-VERTEX: error
) : SV_Position
{
    return float4(0, 0, 0, 1);
}

