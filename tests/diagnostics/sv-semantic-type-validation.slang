//TEST:SIMPLE(filecheck=CHECK1): -target spirv -entry vsWrongVectorSize -stage vertex
//TEST:SIMPLE(filecheck=CHECK2): -target spirv -entry vsWrongVectorSize2 -stage vertex

// Test type validation for system value semantics.
// Each SV_ semantic has specific type requirements - shape mismatches (scalar vs vector,
// wrong vector sizes) produce errors, while scalar type coercions are allowed.

// Test 1: Wrong vector size for SV_VertexID
// SV_VertexID requires uint (scalar), not uint2 (vector)
// CHECK1: ([[# @LINE+1]]): error 30701: type 'vector<uint,2>' is not valid for system value semantic 'SV_VertexID'; expected 'uint'
float4 vsWrongVectorSize(uint2 vertexId : SV_VertexID) : SV_Position
{
    return float4(float(vertexId.x), 0, 0, 1);
}

// Test 2: Wrong vector size for SV_Position output
// SV_Position requires float4, not float2
// CHECK2: ([[# @LINE+1]]): error 30701: type 'vector<float,2>' is not valid for system value semantic 'SV_Position'; expected 'vector<float,4>'
float2 vsWrongVectorSize2(uint vertexId : SV_VertexID) : SV_Position
{
    return float2(float(vertexId), 0);
}
