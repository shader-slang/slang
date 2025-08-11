//TEST:SIMPLE(filecheck=CHK): -target hlsl -entry main

// Test to ensure HLSL mode still produces the diagnostic for global const with initializers
// This verifies our fix doesn't break existing HLSL behavior

// This should trigger error 31224 in HLSL mode
//CHK: ([[# @LINE + 1]]): error 31224: global const variable with initializer must be declared static: 'globalConstWithInit'
const float globalConstWithInit = 1.0f;

// This should also trigger error 31224 in HLSL mode  
//CHK: ([[# @LINE + 1]]): error 31224: global const variable with initializer must be declared static: 'uniformWithInit'
uniform float uniformWithInit = 2.0f;

[shader("vertex")]
float4 main() : SV_Position
{
    return float4(1, 0, 0, 1);
}