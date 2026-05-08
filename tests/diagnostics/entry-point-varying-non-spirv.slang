// Verify that the SPIR-V-only varying rule for `vector<bool>` does NOT
// diagnose on non-SPIR-V targets. A future change that accidentally makes
// this rule universal would otherwise silently break valid HLSL code.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage vertex

[shader("vertex")]
float4 main(vector<bool, 4> b : A) : SV_Position
{
    return float4(0);
}

// HLSL compilation should succeed; the SPIR-V-only varying-type error codes
// must not appear anywhere in the output.
// HLSL-NOT: E38050
// HLSL-NOT: E38051
// HLSL-NOT: invalid-entry-point-varying-type
// Positive check that real output was produced:
// HLSL: float4 main(
