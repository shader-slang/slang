//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib

struct NestedReturn
{
    float4 debug1;
    float4 debug2;
};

struct Output
{
    float4 Diffuse : SV_Target0;
    NestedReturn val : SV_Target1;
    float4 Material : SV_Target2;
}

// CHECK-ASM: define {{.*}} @fragmentMain
// CHECK: color(0)
// CHECK: color(1)
// CHECK-DAG: color(3)
// CHECK-DAG: color(2)

[shader("fragment")]
Output fragmentMain()
{
    return { float4(1), {float4(2), float4(2)}, float4(3) };
}