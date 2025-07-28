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

// CHECK-DAG: %input = OpExtInst %void %{{[0-9]+}} DebugLocalVariable {{.+}} %[[FUNC_ID]]
// CHECK-DAG: %input_pos = OpExtInst %void %{{[0-9]+}} DebugLocalVariable {{.+}} %[[FUNC_ID]]
// CHECK-DAG: %testPos = OpExtInst %void %{{[0-9]+}} DebugLocalVariable {{.+}} %[[FUNC_ID]]

// CHECK-DAG: DebugGlobalVariable %{{.+}} %[[COMPILATION_UNIT_ID]] %{{[0-9]+}} %testTex
// CHECK-DAG: DebugGlobalVariable %{{.+}} %[[COMPILATION_UNIT_ID]] %{{[0-9]+}} %testSampler

// CHECK: %main = OpFunction %void None

// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugDeclare %input_pos %[[input_pos:[0-9]+]]
// CHECK-DAG: OpStore %[[input_pos]] %[[gl_FragCoord_input_pos:[0-9]+]]
// CHECK-DAG: %[[gl_FragCoord_input_pos]] = OpLoad %v4float %gl_FragCoord

// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %input %[[input:[0-9]+]]
// CHECK-DAG: OpStore %[[input]] %[[gl_FragCoord_input:[0-9]+]]
// CHECK-DAG: %[[gl_FragCoord_input]] = OpLoad %v4float %gl_FragCoord
