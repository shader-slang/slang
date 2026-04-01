//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1): -target spirv -entry vsWrongVectorSize -stage vertex
//TEST:SIMPLE(filecheck=CHECK2): -target spirv -entry vsPositionFloat2 -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK3): -target spirv -entry vsConditionalWrongType -stage vertex
//TEST:SIMPLE(filecheck=CHECK4): -target spirv -entry vsConditionalCorrectType -stage vertex
//TEST:SIMPLE(filecheck=CHECK5): -target spirv -entry vsConditionalOutputFloat2 -stage vertex
//TEST:SIMPLE(filecheck=CHECK6): -target spirv -entry vsConditionalOutputCorrectType -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK7): -target spirv -entry vsConditionalOutputWrongType -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK8): -target spirv -entry vsPositionScalar -stage vertex
//TEST:SIMPLE(filecheck=CHECK9): -target spirv -entry vsPositionFloat3 -stage vertex
//TEST:SIMPLE(filecheck=CHECK10): -target hlsl -entry vsPositionFloat2 -stage vertex

// Test type validation for system value semantics.
// Each SV_ semantic declares its accepted types. Shape mismatches (e.g., vector for a
// scalar semantic) produce errors, while scalar type coercions (e.g., int to uint) are allowed.

// Test 1: Wrong vector size for SV_VertexID
// SV_VertexID requires uint (scalar), not uint2 (vector)
float4 vsWrongVectorSize(uint2 vertexId : SV_VertexID) : SV_Position
//CHECK1:                      ^^^^^^^^ type 'vector<uint,2>' is not valid for system value semantic 'SV_VertexID'; expected 'uint'
{
    return float4(float(vertexId.x), 0, 0, 1);
}

// Test 2: float2 is an accepted type for SV_Position output (HLSL allows fewer components)
// CHECK2-NOT: error 30701
float2 vsPositionFloat2(uint vertexId : SV_VertexID) : SV_Position
{
    return float2(float(vertexId), 0);
}

// Test 3: Conditional<T, hasValue> should unwrap to T for type validation (input)
// Conditional<uint2, true> unwraps to uint2, which is wrong for SV_VertexID
// The error message shows the unwrapped type (what's actually validated)
float4 vsConditionalWrongType(Conditional<uint2, true> vertexId : SV_VertexID) : SV_Position
//CHECK3:                                              ^^^^^^^^ type 'vector<uint,2>' is not valid for system value semantic 'SV_VertexID'; expected 'uint'
{
    return float4(0, 0, 0, 1);
}

// Test 4: Conditional<uint, true> unwraps to uint, which is correct for SV_VertexID (input)
// CHECK4-NOT: error 30701
float4 vsConditionalCorrectType(Conditional<uint, true> vertexId : SV_VertexID) : SV_Position
{
    return float4(0, 0, 0, 1);
}

// Test 5: Conditional<float2, true> unwraps to float2, which is now accepted for SV_Position
// CHECK5-NOT: error 30701
Conditional<float2, true> vsConditionalOutputFloat2(uint vertexId : SV_VertexID) : SV_Position
{
    return float2(0, 0);
}

// Test 6: Conditional<float4, true> unwraps to float4, which is correct for SV_Position (output)
// CHECK6-NOT: error 30701
Conditional<float4, true> vsConditionalOutputCorrectType(uint vertexId : SV_VertexID) : SV_Position
{
    return float4(0, 0, 0, 1);
}

// Test 7: Conditional<float, true> unwraps to float (scalar), which is wrong for SV_Position
Conditional<float, true> vsConditionalOutputWrongType(uint vertexId : SV_VertexID) : SV_Position
//CHECK7:                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ type 'float' is not valid for system value semantic 'SV_Position'; expected 'vector<float,4>' or 'vector<float,3>' or 'vector<float,2>'
{
    return float(vertexId);
}

// Test 8: Scalar type is rejected for SV_Position (only float2/float3/float4 accepted)
float vsPositionScalar(uint vertexId : SV_VertexID) : SV_Position
/*CHECK8:
      ^^^^^^^^^^^^^^^^ type 'float' is not valid for system value semantic 'SV_Position'; expected 'vector<float,4>' or 'vector<float,3>' or 'vector<float,2>'
*/
{
    return float(vertexId);
}

// Test 9: float3 is an accepted type for SV_Position
// CHECK9-NOT: error 30701
float3 vsPositionFloat3(uint vertexId : SV_VertexID) : SV_Position
{
    return float3(float(vertexId), 0, 0);
}

// Test 10: float2 for SV_Position works for HLSL target too
// CHECK10-NOT: error 30701
// (uses vsPositionFloat2 entry point from Test 2, targeting HLSL)
