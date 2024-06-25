//TEST:SIMPLE(filecheck=CHECK): -entry main -target hlsl -stage fragment
//TEST:SIMPLE(filecheck=GLSL): -entry main -target glsl -stage fragment
//TEST:SIMPLE(filecheck=METAL): -entry main -target metal -stage fragment
//TEST:SIMPLE(filecheck=SPIRV): -entry main -target spirv -stage fragment

float4 main() : SV_Target
{
    return float4(IsHelperLane());
}
