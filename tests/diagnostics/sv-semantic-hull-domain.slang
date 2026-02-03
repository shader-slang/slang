//TEST:SIMPLE(filecheck=CHECK1): -target spirv -entry vsInvalidTessFactor -stage vertex
//TEST:SIMPLE(filecheck=CHECK2): -target spirv -entry vsInvalidDomainLocation -stage vertex

// Test hull/domain shader specific semantic validation.
// SV_TessFactor, SV_InsideTessFactor, SV_DomainLocation, and SV_OutputControlPointID
// are only valid in hull/domain shader stages.

// Test 1: SV_TessFactor in wrong stage (vertex shader)
// SV_TessFactor is only valid as output in hull shader or input in domain shader
// CHECK1: ([[# @LINE+1]]): error 30702: system value semantic 'SV_TessFactor' cannot be used as output in 'vertex' shader stage
float4 vsInvalidTessFactor(uint id : SV_VertexID, out float tessFactor : SV_TessFactor) : SV_Position
{
    tessFactor = 1.0;
    return float4(0, 0, 0, 1);
}

// Test 2: SV_DomainLocation in wrong stage (vertex shader)
// SV_DomainLocation is only valid as input in domain shader
// CHECK2: ([[# @LINE+1]]): error 30702: system value semantic 'SV_DomainLocation' cannot be used as input in 'vertex' shader stage
float4 vsInvalidDomainLocation(float2 domainLoc : SV_DomainLocation) : SV_Position
{
    return float4(domainLoc, 0, 1);
}
