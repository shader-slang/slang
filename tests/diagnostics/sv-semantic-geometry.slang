//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1): -target spirv -entry vsInvalidGSInstanceID -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2): -target spirv -entry fsInvalidGSInstanceID -stage fragment

// Test geometry shader specific semantic validation.
// SV_GSInstanceID is only valid as input in geometry shaders.

// Test 1: SV_GSInstanceID in vertex shader (wrong stage)
float4 vsInvalidGSInstanceID(uint gsInstanceId : SV_GSInstanceID) : SV_Position
//CHECK1:                         ^^^^^^^^^^^^ system value semantic 'SV_GSInstanceID' cannot be used as input in 'vertex' shader stage
{
    return float4(float(gsInstanceId), 0, 0, 1);
}

// Test 2: SV_GSInstanceID in fragment shader (wrong stage)
float4 fsInvalidGSInstanceID(float4 pos : SV_Position, uint gsInstanceId : SV_GSInstanceID) : SV_Target
//CHECK2:                                                   ^^^^^^^^^^^^ system value semantic 'SV_GSInstanceID' cannot be used as input in 'pixel' shader stage
{
    return pos * gsInstanceId;
}
