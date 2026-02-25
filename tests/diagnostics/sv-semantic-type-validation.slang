//TEST:SIMPLE(filecheck=CHECK1): -target spirv -entry vsWrongVectorSize -stage vertex
//TEST:SIMPLE(filecheck=CHECK2): -target spirv -entry vsWrongVectorSize2 -stage vertex
//TEST:SIMPLE(filecheck=CHECK3): -target spirv -entry vsConditionalWrongType -stage vertex
//TEST:SIMPLE(filecheck=CHECK4): -target spirv -entry vsConditionalCorrectType -stage vertex
//TEST:SIMPLE(filecheck=CHECK5): -target spirv -entry vsConditionalOutputWrongType -stage vertex
//TEST:SIMPLE(filecheck=CHECK6): -target spirv -entry vsConditionalOutputCorrectType -stage vertex

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

// Test 3: Conditional<T, hasValue> should unwrap to T for type validation (input)
// Conditional<uint2, true> unwraps to uint2, which is wrong for SV_VertexID
// The error message shows the unwrapped type (what's actually validated)
// CHECK3: ([[# @LINE+1]]): error 30701: type 'vector<uint,2>' is not valid for system value semantic 'SV_VertexID'; expected 'uint'
float4 vsConditionalWrongType(Conditional<uint2, true> vertexId : SV_VertexID) : SV_Position
{
    return float4(0, 0, 0, 1);
}

// Test 4: Conditional<uint, true> unwraps to uint, which is correct for SV_VertexID (input)
// CHECK4-NOT: error 30701
float4 vsConditionalCorrectType(Conditional<uint, true> vertexId : SV_VertexID) : SV_Position
{
    return float4(0, 0, 0, 1);
}

// Test 5: Conditional<T, hasValue> should unwrap to T for type validation (output)
// Conditional<float2, true> unwraps to float2, which is wrong for SV_Position
// CHECK5: ([[# @LINE+1]]): error 30701: type 'vector<float,2>' is not valid for system value semantic 'SV_Position'; expected 'vector<float,4>'
Conditional<float2, true> vsConditionalOutputWrongType(uint vertexId : SV_VertexID) : SV_Position
{
    return float2(0, 0);
}

// Test 6: Conditional<float4, true> unwraps to float4, which is correct for SV_Position (output)
// CHECK6-NOT: error 30701
Conditional<float4, true> vsConditionalOutputCorrectType(uint vertexId : SV_VertexID) : SV_Position
{
    return float4(0, 0, 0, 1);
}
