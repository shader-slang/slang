//TEST:SIMPLE(filecheck=CHECK1): -target spirv -entry vsInvalidGSInstanceID -stage vertex
//TEST:SIMPLE(filecheck=CHECK2): -target spirv -entry fsInvalidGSInstanceID -stage fragment

// Test geometry shader specific semantic validation.
// SV_GSInstanceID is only valid as input in geometry shaders.

// Test 1: SV_GSInstanceID in vertex shader (wrong stage)
// CHECK1: ([[# @LINE+1]]): error 30702: system value semantic 'SV_GSInstanceID' cannot be used as input in 'vertex' shader stage
float4 vsInvalidGSInstanceID(uint gsInstanceId : SV_GSInstanceID) : SV_Position
{
    return float4(float(gsInstanceId), 0, 0, 1);
}

// Test 2: SV_GSInstanceID in fragment shader (wrong stage)
// CHECK2: ([[# @LINE+1]]): error 30702: system value semantic 'SV_GSInstanceID' cannot be used as input in 'pixel' shader stage
float4 fsInvalidGSInstanceID(float4 pos : SV_Position, uint gsInstanceId : SV_GSInstanceID) : SV_Target
{
    return pos * gsInstanceId;
}
