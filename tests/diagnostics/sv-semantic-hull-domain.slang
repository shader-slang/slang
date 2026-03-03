//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1): -target spirv -entry vsInvalidTessFactor -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2): -target spirv -entry vsInvalidDomainLocation -stage vertex

// Test hull/domain shader specific semantic validation.
// SV_TessFactor, SV_InsideTessFactor, SV_DomainLocation, and SV_OutputControlPointID
// are only valid in hull/domain shader stages.

// Test 1: SV_TessFactor in wrong stage (vertex shader)
// SV_TessFactor is only valid as output in hull shader or input in domain shader
float4 vsInvalidTessFactor(uint id : SV_VertexID, out float tessFactor : SV_TessFactor) : SV_Position
//CHECK1:                                                   ^^^^^^^^^^ system value semantic 'SV_TessFactor' cannot be used as output in 'vertex' shader stage
{
    tessFactor = 1.0;
    return float4(0, 0, 0, 1);
}

// Test 2: SV_DomainLocation in wrong stage (vertex shader)
// SV_DomainLocation is only valid as input in domain shader
float4 vsInvalidDomainLocation(float2 domainLoc : SV_DomainLocation) : SV_Position
//CHECK2:                             ^^^^^^^^^ system value semantic 'SV_DomainLocation' cannot be used as input in 'vertex' shader stage
{
    return float4(domainLoc, 0, 1);
}
