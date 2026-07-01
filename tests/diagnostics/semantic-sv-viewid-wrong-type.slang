// Regression test for https://github.com/shader-slang/slang/issues/6428
// SV_ViewID requires an integer type; using float should be a diagnostic error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage vertex -entry vertexShader

[shader("vertex")]
float4 vertexShader(float id: SV_ViewID) : SV_Position
//CHECK: E30701
{
    return float4(id, 0, 0, 1);
}
