//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage miss -g2 -emit-spirv-directly

struct TestType
{
    float4 color;
    float3 normal;
};

void main(inout TestType t)
{
    t.color = float4(1, 1, 1, 1);
    t.normal = float3(1, 0, 0);
}

// CHECK: OpEntryPoint MissKHR %main "main" %t_0
// CHECK: OpExtInst %void %2 DebugGlobalVariable {{.*}} %t_0
