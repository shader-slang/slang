// Test that accessing .value on both Optional and Conditional that are always 'none' produces errors
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

[shader("fragment")]
float4 main() : SV_Target
{
    Optional<float4> opt = none;
    // CHECK-DAG: ([[# @LINE+1]]): error 41027
    float4 a = opt.value;

    Conditional<float4, false> cond;
    // CHECK-DAG: ([[# @LINE+1]]): error 41027
    float4 b = cond.get().value;

    return a + b;
}
