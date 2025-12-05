//TEST:SIMPLE(filecheck=VK_CHECK):-target spirv -entry main -stage fragment -gdwarf -g2 -O0 -line-directive-mode glsl
//TEST:SIMPLE(filecheck=VK_CHECK_MINIMAL):-target spirv -entry main -stage fragment -gdwarf -g1 -O0 -line-directive-mode glsl

// VK_CHECK_MINIMAL-NOT: DebugLexicalBlock

// VK_CHECK: %[[fname0:[0-9]+]] = OpString "main"
// VK_CHECK: %[[fname1:[0-9]+]] = OpString "func"

// VK_CHECK: %[[func0:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[fname0]] {{.*}}
// VK_CHECK: %[[block0:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugLexicalBlock {{.*}} %[[func0]]
// VK_CHECK: %ret{{.*}} = OpExtInst %void {{.*}} DebugLocalVariable {{.*}} %[[func0]] {{.*}}
// VK_CHECK: %lv{{.*}} = OpExtInst %void {{.*}} DebugLocalVariable {{.*}} %[[func0]] {{.*}}
// VK_CHECK: %lv{{.*}} = OpExtInst %void {{.*}} DebugLocalVariable {{.*}} %[[block0]] {{.*}}

// VK_CHECK: %[[func1:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[fname1]] {{.*}}
// func should not have DebugLexicalBlock
// VK_CHECK-NOT: DebugLexicalBlock

cbuffer CB0
{
    float4 a;
    float4 b;
} cb0;

float4 func(float4 cp)
{
    return cp + float4(0.01);
};

[shader("fragment")]
float4 main() : SV_Target
{
    float4 ret = float4(0);
    float4 lv = float4(0.01);
    {
        if (cb0.a.x > 0)
            ret -= float4(0.01);
        if (cb0.a.x > 1)
            lv += float4(0.01);
        float4 lv = float4(0.01);
        ret += func(lv);
    }
    return ret;
}