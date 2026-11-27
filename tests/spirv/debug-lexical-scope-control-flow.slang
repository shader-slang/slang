//TEST:SIMPLE(filecheck=SWITCH): -target spirv -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=EXPR): -target spirv -g2 -O0 -validate-ir

// Switch traversal must preserve both the switch braces and the distinct case scopes.
// SWITCH-DAG: [[NAME:%[0-9]+]] = OpString "main"
// SWITCH-DAG: [[SHADOW_NAME:%[0-9]+]] = OpString "shadow"
// SWITCH-DAG: [[FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// SWITCH-DAG: [[SWITCH:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE:%[A-Za-z_0-9]+]] %uint_105 %uint_5 [[FUNC]]
// SWITCH-DAG: [[FIRST:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_107 %uint_13 [[SWITCH]]
// SWITCH-DAG: [[SECOND:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_114 %uint_13 [[SWITCH]]
// SWITCH-DAG: {{.*}} DebugLocalVariable [[SHADOW_NAME]] {{.*}} [[SOURCE]] %uint_107 {{%uint_[0-9]+}} [[FIRST]]
// SWITCH-DAG: {{.*}} DebugLocalVariable [[SHADOW_NAME]] {{.*}} [[SOURCE]] %uint_114 {{%uint_[0-9]+}} [[SECOND]]
// SWITCH: DebugScope [[SECOND]]{{$}}
// SWITCH-NOT: Debug{{(No)?Scope}}
// SWITCH: OpStore {{.*}} %int_202
// SWITCH: DebugScope [[FIRST]]{{$}}
// SWITCH-NOT: Debug{{(No)?Scope}}
// SWITCH: OpStore {{.*}} %int_201
// SWITCH: DebugScope [[FUNC]]{{$}}
// Repeated function-scope assignments at block entry are harmless. None of the
// scopes active in the switch may remain active at the following store.
// SWITCH-NOT: DebugNoScope
// SWITCH-NOT: DebugScope [[SWITCH]]
// SWITCH-NOT: DebugScope [[FIRST]]
// SWITCH-NOT: DebugScope [[SECOND]]
// SWITCH: OpStore {{.*}} %int_203

// Expression lowering creates its own CFG blocks. Check the effective scope at
// actual stores in the short-circuit RHS, both conditional arms, and their merge.
// EXPR-DAG: [[NAME:%[0-9]+]] = OpString "main"
// EXPR-DAG: [[FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// EXPR-DAG: [[BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock {{%[A-Za-z_0-9]+}} %uint_122 %uint_9 [[FUNC]]
// EXPR: OpBranchConditional
// EXPR: DebugScope [[BLOCK]]{{$}}
// EXPR-NOT: Debug{{(No)?Scope}}
// EXPR: OpStore {{.*}} %int_301
// EXPR: OpBranchConditional
// EXPR: DebugScope [[BLOCK]]{{$}}
// EXPR-NOT: Debug{{(No)?Scope}}
// EXPR: OpStore {{.*}} %int_302
// EXPR: DebugScope [[BLOCK]]{{$}}
// EXPR-NOT: Debug{{(No)?Scope}}
// EXPR: OpStore {{.*}} %int_303
// EXPR: DebugScope [[BLOCK]]{{$}}
// EXPR-NOT: Debug{{(No)?Scope}}
// EXPR: OpStore {{.*}} %int_304

RWStructuredBuffer<int> outputBuffer;
[shader("compute")]
#line 99 "lexical-control-flow.slang"
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int key = int(tid.x);
    switch (key)
    {
    case 0:
        {
            int shadow = key + 1;
            outputBuffer[0] = shadow;
            outputBuffer[1] = 201;
            break;
        }
    default:
        {
            int shadow = key + 2;
            outputBuffer[2] = shadow;
            outputBuffer[3] = 202;
            break;
        }
    }
    outputBuffer[4] = 203;
    {
        bool p = tid.x != 0 && ((outputBuffer[5] = 301) > 0);
        int v = tid.y != 0 ? (outputBuffer[6] = 302) : (outputBuffer[7] = 303);
        outputBuffer[8] = 304;
        outputBuffer[9] = int(p) + v;
    }
}
