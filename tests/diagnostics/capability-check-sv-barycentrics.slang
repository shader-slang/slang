// Test that SV_Barycentrics correctly requires the fragmentshaderbarycentric capability.

// Test 1: Direct parameter
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-stage fragment -entry fragmentMain -target spirv -profile spirv_1_6

[shader("fragment")]
float4 fragmentMain(float3 barycentrics : SV_Barycentrics) : SV_Target
/*CHECK1:
       ^^^^^^^^^^^^ profile implicitly upgraded
       ^^^^^^^^^^^^ entry point 'fragmentMain' uses additional capabilities that are not part of the specified profile 'spirv_1_6'. The profile setting is automatically updated to include these capabilities: 'spvFragmentBarycentricKHR'
*/
{
    return float4(barycentrics, 1.0);
}

// Test 2: Struct field
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-stage fragment -entry fragmentMainStruct -target spirv -profile spirv_1_6

struct PSInput
{
    float3 barycentrics : SV_Barycentrics;
};

[shader("fragment")]
float4 fragmentMainStruct(PSInput input) : SV_Target
/*CHECK2:
       ^^^^^^^^^^^^^^^^^^ profile implicitly upgraded
       ^^^^^^^^^^^^^^^^^^ entry point 'fragmentMainStruct' uses additional capabilities that are not part of the specified profile 'spirv_1_6'. The profile setting is automatically updated to include these capabilities: 'spvFragmentBarycentricKHR'
*/
{
    return float4(input.barycentrics, 1.0);
}

// Test 3: No warning when the capability is already in the profile
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK3):-stage fragment -entry fragmentMainWithCap -target spirv -profile spirv_1_4+spvFragmentBarycentricKHR

[shader("fragment")]
float4 fragmentMainWithCap(float3 barycentrics : SV_Barycentrics) : SV_Target
{
    return float4(barycentrics, 1.0);
}
