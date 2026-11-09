// Using SV_Barycentrics must surface its fragmentshaderbarycentric requirement, so a profile
// lacking that capability is diagnosed as implicitly upgraded rather than compiling silently.

// Direct parameter, SPIR-V.
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

// Struct field, SPIR-V.
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

// Nested struct field — exercises the recursive descent.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NESTED):-stage fragment -entry fragmentMainNested -target spirv -profile spirv_1_6

struct Inner
{
    float3 barycentrics : SV_Barycentrics;
};

struct Outer
{
    float4 position : SV_Position;
    Inner inner;
};

[shader("fragment")]
float4 fragmentMainNested(Outer input) : SV_Target
/*CHECK_NESTED:
       ^^^^^^^^^^^^^^^^^^ profile implicitly upgraded
       ^^^^^^^^^^^^^^^^^^ entry point 'fragmentMainNested' uses additional capabilities that are not part of the specified profile 'spirv_1_6'. The profile setting is automatically updated to include these capabilities: 'spvFragmentBarycentricKHR'
*/
{
    return float4(input.inner.barycentrics, 1.0);
}

// Negative case: capability present in the profile, so no warning.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_HASCAP):-stage fragment -entry fragmentMainWithCap -target spirv -profile spirv_1_6+spvFragmentBarycentricKHR

[shader("fragment")]
float4 fragmentMainWithCap(float3 barycentrics : SV_Barycentrics) : SV_Target
{
    return float4(barycentrics, 1.0);
}

// HLSL path: the same requirement maps to sm_6_1 rather than the SPIR-V extension.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_HLSL):-stage fragment -entry fragmentMainHlsl -target hlsl -profile sm_6_0

[shader("fragment")]
float4 fragmentMainHlsl(float3 barycentrics : SV_Barycentrics) : SV_Target
/*CHECK_HLSL:
       ^^^^^^^^^^^^^^^^ profile implicitly upgraded
       ^^^^^^^^^^^^^^^^ entry point 'fragmentMainHlsl' uses additional capabilities that are not part of the specified profile 'sm_6_0'. The profile setting is automatically updated to include these capabilities: 'sm_6_1'
*/
{
    return float4(barycentrics, 1.0);
}

// Wrong field type: the accessor is selected by type, so a mismatched `int` matches no accessor
// and contributes no capability. Only the type-mismatch error fires — the exhaustive check would
// fail if a spurious barycentric profile-upgrade warning were also emitted.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MISMATCH):-stage fragment -entry fragmentMainBadType -target spirv -profile spirv_1_6

[shader("fragment")]
float4 fragmentMainBadType(int barycentrics : SV_Barycentrics) : SV_Target
//CHECK_MISMATCH:              ^^^^^^^^^^^^ type 'int' is not valid for system value semantic 'SV_Barycentrics'; expected 'vector<float,3>'
{
    return float4(0.0, 0.0, 0.0, 1.0);
}
