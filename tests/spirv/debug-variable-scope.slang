//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly

Texture2D    testTex      :   register(t0);
SamplerState testSampler  :   register(s0);

struct PSIn
{
   float4 pos      : SV_Position;
};

float4 main(PSIn input) : SV_TARGET
{
    uint4 testPos = (uint4)input.pos;
    float bias = -1.0;
    float2 tc = testPos.xy / 32.0;
    float4 colVal = testTex.SampleBias(testSampler, tc, bias);
    return float4(colVal.xyz, 1.0);
}

// CHECK: %[[COMPILATION_UNIT_ID:[0-9]+]] = OpExtInst %void {{.*}} DebugCompilationUnit
// CHECK: %[[FUNC_ID:[0-9]+]] = OpExtInst %void {{.*}} DebugFunction %{{[0-9]+}}

// CHECK: %input = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %{{[^ ]+}} %{{[^ ]+}} %[[FUNC_ID]]
// CHECK: %input_pos = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %{{[^ ]+}} %{{[^ ]+}} %[[FUNC_ID]]
// CHECK: %testPos = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %{{[^ ]+}} %{{[^ ]+}} %[[FUNC_ID]]

// CHECK: DebugGlobalVariable %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %{{.*}} %{{.*}} %[[COMPILATION_UNIT_ID]] %{{[0-9]+}} %testTex
// CHECK: DebugGlobalVariable %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %{{.*}} %{{.*}} %[[COMPILATION_UNIT_ID]] %{{[0-9]+}} %testSampler

// CHECK: = OpExtInst %void %{{[0-9]+}} DebugValue %input_pos %gl_FragCoord
