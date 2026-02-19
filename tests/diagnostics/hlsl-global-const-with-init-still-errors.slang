//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target hlsl -entry main

// Test to ensure HLSL mode still produces the diagnostic for global const with initializers
// This verifies our fix doesn't break existing HLSL behavior

// This should trigger error 31224 in HLSL mode
const float globalConstWithInit = 1.0f;
/*diag:
            ^^^^^^^^^^^^^^^^^^^ global const requires static
            ^^^^^^^^^^^^^^^^^^^ global const variable with initializer must be declared static: 'globalConstWithInit'
*/

// This should also trigger error 31224 in HLSL mode
uniform float uniformWithInit = 2.0f;
/*diag:
              ^^^^^^^^^^^^^^^ global const requires static
              ^^^^^^^^^^^^^^^ global const variable with initializer must be declared static: 'uniformWithInit'
*/

[shader("vertex")]
float4 main() : SV_Position
{
    return float4(1, 0, 0, 1);
}
