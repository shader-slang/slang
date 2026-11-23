// Two sampling locations on one varying conflict (an interpolation mode and a sampling location
// combine: interpolation-mode-and-sampling.slang).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage fragment -entry fsMain

struct BadIn
{
    float4 pos : SV_Position;
    centroid sample float a : TEXCOORD0;
//CHECK:     ^^^^^^ duplicate modifier
};

float4 fsMain(BadIn i) : SV_Target
{
    return float4(i.a, 0, 0, 1);
}
