//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=STRIP): -target spirv -g0 -O0 -validate-ir
//TEST:SIMPLE(filecheck=SEPARATE): -target spirv -g2 -O0 -separate-debug-info -separate-debug-info-output debug-lexical-scope-restore.dbg.spv

// Scope markers set the current scope. Each ordered check reaches an actual store
// without another scope marker, so a transient restore cannot satisfy the test.
// CHECK-DAG: [[NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[INDEX_NAME:%[0-9]+]] = OpString "index"
// CHECK-DAG: [[SHADOW_NAME:%[0-9]+]] = OpString "shadow"
// CHECK-DAG: [[FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK-DAG: [[FIRST:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE:%[A-Za-z_0-9]+]] %uint_104 %uint_9 [[FUNC]]
// CHECK-DAG: [[INNER:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_107 %uint_13 [[FIRST]]
// CHECK-DAG: [[SECOND:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_115 %uint_9 [[FUNC]]
// CHECK-DAG: [[LOOP:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_120 %uint_5 [[FUNC]]
// CHECK-DAG: [[BODY:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_122 %uint_9 [[LOOP]]
// CHECK-DAG: [[EARLY_RETURN:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_130 %uint_9 [[FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[INDEX_NAME]] {{.*}} [[SOURCE]] %uint_102 {{%uint_[0-9]+}} [[FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[INDEX_NAME]] {{.*}} [[SOURCE]] %uint_120 {{%uint_[0-9]+}} [[LOOP]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[SHADOW_NAME]] {{.*}} [[SOURCE]] %uint_104 {{%uint_[0-9]+}} [[FIRST]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[SHADOW_NAME]] {{.*}} [[SOURCE]] %uint_107 {{%uint_[0-9]+}} [[INNER]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[SHADOW_NAME]] {{.*}} [[SOURCE]] %uint_115 {{%uint_[0-9]+}} [[SECOND]]
// CHECK: DebugScope [[INNER]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_102
// CHECK: DebugScope [[FIRST]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_103
// CHECK: DebugScope [[FUNC]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_104
// CHECK: DebugScope [[SECOND]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_105
// CHECK: DebugScope [[FUNC]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_106
// CHECK: OpLoopMerge [[MERGE:%[A-Za-z_0-9]+]]
// CHECK: DebugScope [[BODY]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_107
// CHECK: [[MERGE]] = OpLabel
// CHECK: DebugScope [[LOOP]]{{$}}
// CHECK: DebugScope [[FUNC]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_108
// CHECK: DebugScope [[EARLY_RETURN]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_109
// CHECK: OpReturn
// CHECK: DebugScope [[FUNC]]{{$}}
// CHECK-NOT: DebugNoScope
// CHECK-NOT: DebugScope [[EARLY_RETURN]]
// CHECK: OpStore {{.*}} %int_110
// STRIP-NOT: DebugLexicalBlock
// STRIP-NOT: DebugScope
// SEPARATE: DebugBuildIdentifier
// SEPARATE-NOT: DebugLexicalBlock
// SEPARATE-NOT: DebugScope

RWStructuredBuffer<int> outputBuffer;
[shader("compute")]
#line 99 "lexical-restore.slang"
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int index = int(tid.x);
    {
        int shadow = index + 1;
        outputBuffer[0] = shadow;
        {
            int shadow = index + 2;
            outputBuffer[1] = shadow;
            outputBuffer[2] = 102;
        }
        outputBuffer[3] = 103;
    }
    outputBuffer[4] = 104;
    {
        int shadow = index + 3;
        outputBuffer[5] = shadow;
        outputBuffer[6] = 105;
    }
    outputBuffer[7] = 106;
    for (int index = 0; index < int(tid.y); ++index)
    {
        outputBuffer[8] = 107;
        if (index == 2) continue;
        if (index == 3) break;
    }
    outputBuffer[9] = 108;
    outputBuffer[10] = index;
    if (index == 4)
    {
        outputBuffer[11] = 109;
        return;
    }
    outputBuffer[12] = 110;
}
