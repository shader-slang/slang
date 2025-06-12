//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-via-glsl

// This test is to test that PartlyDiffable.Differential is synthesized
// with correct scope. If not, the constructor for it will be wrong, and
// slang will generate wrong code, the downstream compiler will fail to
// compile.

// CHECK: OpEntryPoint Fragment

struct PartlyDiffable : IDifferentiable
{
    int i;
    float q;
}


func breaker(x:PartlyDiffable.Differential) {
    return;
}


[shader("fragment")]
float4 fragment(float4 in: SV_Position)
    : SV_Target
{
    if (PartlyDiffable.Differential(0).q == 0) {
        return float4(0);
    } else {
        return float4(1);
    }
}
