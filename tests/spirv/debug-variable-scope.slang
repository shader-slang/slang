// Regressed with SPIRV update. Tracking on github issue #8522
//DISABLE_TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -g2 -O0 -emit-spirv-directly

Texture2D    testTex      :   register(t0);
SamplerState testSampler  :   register(s0);

struct PSIn
{
   float4 pos      : SV_Position;
   float4 color    : COLOR;
};

float4 main(PSIn input) : SV_TARGET
{
    uint4 testPos = (uint4)input.pos;
    float bias = -1.0;
    float2 tc = testPos.xy / 32.0;
    float4 colVal = testTex.SampleBias(testSampler, tc, bias) + input.color;
    return float4(colVal.xyz, 1.0);
}

// Make sure "pos" and "color" is reported as member variables
// CHECK-DAG: %[[StrPos:[0-9]+]] = OpString "pos"
// CHECK-DAG: %[[StrColor:[0-9]+]] = OpString "color"
// CHECK-DAG: DebugTypeMember %[[StrPos]]
// CHECK-DAG: DebugTypeMember %[[StrColor]]

// Global variables should be reported as DebugGlobalVariable
// CHECK-DAG: %[[COMPILATION_UNIT_ID:[0-9]+]] = OpExtInst %void {{.*}} DebugCompilationUnit
// CHECK-DAG: DebugGlobalVariable %{{.+}} %[[COMPILATION_UNIT_ID]] %{{[0-9]+}} %testTex
// CHECK-DAG: DebugGlobalVariable %{{.+}} %[[COMPILATION_UNIT_ID]] %{{[0-9]+}} %testSampler

// Entry point parameter is reported as DebugLocalVariable
// CHECK-DAG: %[[StrInput:[0-9]+]] = OpString "input"
// CHECK-DAG: %[[input:[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable %[[StrInput]]

// Funciton local variable should be reported as DebugLocalVariable
// CHECK-DAG: %[[StrTestPos:[0-9]+]] = OpString "testPos"
// CHECK-DAG: DebugLocalVariable %[[StrTestPos]]

// CHECK: %main = OpFunction %void None

// "input.pos" is reported with DebugValue as a first member
// And its value comes from `gl_FragCoord`
// CHECK-DAG: DebugValue %[[input]] %[[input_pos:[0-9]+]] %{{[0-9]+}} %int_0
// CHECK-DAG: OpStore %[[input_pos]] %[[gl_FragCoord_input_pos:[0-9]+]]
// CHECK-DAG: %[[gl_FragCoord_input_pos]] = OpLoad %v4float %gl_FragCoord

// "input.color" is reported with DebugValue as a second member
// And its value comes from a global varying input
// CHECK-DAG: DebugValue %[[input]] %[[input_color:[0-9]+]] %{{[0-9]+}} %int_1
// CHECK-DAG: OpStore %[[input_color]] %[[varying_color:[0-9]+]]
// CHECK-DAG: %[[varying_color]] = OpLoad %v4float %input_color
