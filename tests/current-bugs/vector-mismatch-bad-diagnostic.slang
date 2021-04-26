// vector-mismatch-bad-diagnostic.slang

//DIAGNOSTIC_TEST:SIMPLE:-target hlsl -stage compute -entry computeMain

/* 
This test produces the diagnostic, claiming it's ambiguous, but none of the options appear to be a possible
match. That being the case it's not ambiguous it's that there isn't a match.

tests/current-bugs/vector-mismatch-bad-diagnostic.slang(20): error 39999: ambiguous call to 'float4' with arguments of type (float, float, float)
    float3 v = float4(1.0, 2.0, 1.0);
                     ^
core.meta.slang(446): note 39999: candidate: vector<T,4>.init(float, float, vector<float,2>)
core.meta.slang(445): note 39999: candidate: vector<T,4>.init(float, vector<float,2>, float)
core.meta.slang(444): note 39999: candidate: vector<T,4>.init(vector<float,2>, float, float)
*/

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3 v = float4(1.0, 2.0, 1.0);
}
