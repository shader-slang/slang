//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib

struct NestedReturn
{
    float4 debug;
};
struct NestedReturn2
{
    float4 debugAux1;
    float4 debugAux2;
};

// Semantics are supposed to ignore uppercase/lowercase differences
struct Output
{
    float4 Diffuse : SV_TarGet0;
    NestedReturn debug1 : SV_Target1;
    float4 Material : SV_TArgeT2;
    NestedReturn2 debug2 : SV_TaRget3;
}

// CHECK-ASM: define {{.*}} @fragmentMain
// CHECK: color(0)
// CHECK-DAG: color(1)
// CHECK-DAG: color(2)
// CHECK-DAG: color(3)
// CHECK-DAG: color(4)

[shader("fragment")]
Output fragmentMain()
{
    return { float4(1), {float4(2)}, float4(3) };
}