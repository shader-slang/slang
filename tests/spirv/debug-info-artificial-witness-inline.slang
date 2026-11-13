//TEST:SIMPLE(filecheck=FORCE): -target spirv-asm -g2 -O0 -DINLINE_USER=1 -validate-ir
//TEST:SIMPLE(filecheck=FORCE): -target spirv-asm -g2 -O0 -DINLINE_USER=2 -validate-ir
//TEST:SIMPLE(filecheck=OPT): -target spirv-asm -g2 -O1 -DINLINE_USER=0 -validate-ir
//TEST:SIMPLE(filecheck=MINIMAL_FORCE): -target spirv-asm -g1 -O0 -DINLINE_USER=1 -validate-ir
//TEST:SIMPLE(filecheck=MINIMAL_FORCE): -target spirv-asm -g1 -O0 -DINLINE_USER=2 -validate-ir
//TEST:SIMPLE(filecheck=MINIMAL_OPT): -target spirv-asm -g1 -O1 -DINLINE_USER=0 -validate-ir

// Check that authored lines and unflagged scopes survive inlining into an artificial witness.
// Optimized inlining also moves the witness and authored method into the unflagged caller.
//FORCE-DAG: [[WRAPPER_NAME:%[0-9]+]] = OpString "MyStruct.$__syn_method"
//FORCE-DAG: [[USER_NAME:%[0-9]+]] = OpString "MyStruct.method"
//FORCE: [[ARTIFICIAL:%[a-zA-Z0-9_]+]] = OpConstant %uint 32
//FORCE-DAG: [[WRAPPER:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[WRAPPER_NAME]] {{.*}} [[WRAPPER_NAME]] [[ARTIFICIAL]]
//FORCE-DAG: [[USER:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[USER_NAME]] {{.*}} [[USER_NAME]] %uint_0
//FORCE: %MyStruct____syn_method = OpFunction %int
//FORCE-NOT: OpFunctionEnd
//FORCE: DebugFunctionDefinition [[WRAPPER]] %MyStruct____syn_method
//FORCE-NOT: OpFunctionEnd
//FORCE: DebugScope [[USER]]
//FORCE-NOT: {{OpFunctionEnd|DebugScope}}

//OPT-DAG: [[WRAPPER_NAME:%[0-9]+]] = OpString "MyStruct.$__syn_method"
//OPT-DAG: [[USER_NAME:%[0-9]+]] = OpString "MyStruct.method"
//OPT-DAG: [[CALLER_NAME:%[0-9]+]] = OpString "invoke"
//OPT: [[ARTIFICIAL:%[a-zA-Z0-9_]+]] = OpConstant %uint 32
//OPT-DAG: [[CALLER:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[CALLER_NAME]] {{.*}} [[CALLER_NAME]] %uint_0
//OPT-DAG: [[WRAPPER:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[WRAPPER_NAME]] {{.*}} [[WRAPPER_NAME]] [[ARTIFICIAL]]
//OPT-DAG: [[USER:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[USER_NAME]] {{.*}} [[USER_NAME]] %uint_0
//OPT-DAG: [[WRAPPER_SITE:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugInlinedAt {{%uint_[0-9]+}} [[CALLER]]
//OPT-DAG: [[USER_SITE:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugInlinedAt {{%uint_[0-9]+}} [[WRAPPER]] [[WRAPPER_SITE]]
//OPT: %invoke = OpFunction %int DontInline
//OPT-NOT: OpFunctionEnd
//OPT: DebugScope [[WRAPPER]] [[WRAPPER_SITE]]
//OPT-NOT: OpFunctionEnd
//OPT: DebugScope [[USER]] [[USER_SITE]]
//OPT-NOT: {{OpFunctionEnd|DebugScope}}

//MINIMAL_FORCE: %MyStruct____syn_method = OpFunction %int
//MINIMAL_FORCE-NOT: OpFunctionEnd
//MINIMAL_OPT: %invoke = OpFunction %int DontInline
//MINIMAL_OPT-NOT: OpFunctionEnd

interface IFoo
{
    static int method(This value, int input);
}

struct MyStruct : IFoo
{
#if INLINE_USER == 1
    [ForceInline]
#elif INLINE_USER == 2
    [__unsafeForceInlineEarly]
#elif INLINE_USER == 3
    [noinline]
#endif
    int method(int input)
    {
        int adjusted = input + 1;
        //FORCE: DebugLine {{.*}} %uint_[[#@LINE-1]]
        //FORCE-NOT: {{OpFunctionEnd|DebugScope}}
        //FORCE: OpIAdd
        //OPT: DebugLine {{.*}} %uint_[[#@LINE-4]]
        //OPT-NOT: {{OpFunctionEnd|DebugScope}}
        //OPT: OpIAdd
        //MINIMAL_FORCE: OpLine {{%[0-9]+}} [[#@LINE-7]] {{[0-9]+}}
        //MINIMAL_FORCE-NOT: OpFunctionEnd
        //MINIMAL_FORCE: OpIAdd
        //MINIMAL_OPT: OpLine {{%[0-9]+}} [[#@LINE-10]] {{[0-9]+}}
        //MINIMAL_OPT-NOT: OpFunctionEnd
        //MINIMAL_OPT: OpIAdd
        return adjusted * 3;
    }
}

[noinline]
int invoke<T : IFoo>(T value, int input)
{
    return T.method(value, input);
}

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    MyStruct value = {};
    outputBuffer[tid.x] = invoke(value, int(tid.x));
}
