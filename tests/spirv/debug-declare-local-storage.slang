// A local variable that keeps its storage through SSA construction is described
// by one DebugDeclare that names its OpVariable, the same way an `inout`
// parameter of a non-inlined function is described by a DebugDeclare that names
// its OpFunctionParameter. Neither gets a `_dbgvar_` shadow variable, and no
// store to the storage is mirrored into a DebugValue.
//
// The patterns below avoid spelling the shadow-variable names literally because
// -g2 embeds this source file in an OpSource string.

//TEST:SIMPLE(filecheck=SLOCAL): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=ARRLOCAL): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=VPARAM): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=NOSHADOW): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir

struct S
{
    float a;
    int b;
};

RWStructuredBuffer<float> outputBuffer;

[noinline]
void modify(inout S v)
{
    v.a += 1.0;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // `s` is stored to field by field, so SSA construction keeps its storage.
    S s;
    s.a = 1.0;
    s.b = 2;
    modify(s);

    // `arr` is indexed dynamically, so SSA construction keeps its storage.
    float arr[4];
    for (int i = 0; i < 4; i++)
        arr[i] = float(i);
    arr[tid.x & 3] = 5.0;

    outputBuffer[0] = s.a + float(s.b) + arr[tid.y & 3];
}

// SLOCAL: [[S_STR:%[0-9]+]] = OpString "s"
// SLOCAL: [[S_DBG:%[A-Za-z0-9_]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[S_STR]]
// SLOCAL: [[S_VAR:%s[_0-9]*]] = OpVariable %_ptr_Function_S Function
// SLOCAL: DebugDeclare [[S_DBG]] [[S_VAR]]
// SLOCAL-NOT: DebugValue [[S_DBG]]

// ARRLOCAL: [[ARR_STR:%[0-9]+]] = OpString "arr"
// ARRLOCAL: [[ARR_DBG:%[A-Za-z0-9_]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ARR_STR]]
// ARRLOCAL: [[ARR_VAR:%arr[_0-9]*]] = OpVariable %_ptr_Function__arr_float_int_4 Function
// ARRLOCAL: DebugDeclare [[ARR_DBG]] [[ARR_VAR]]
// ARRLOCAL-NOT: DebugValue [[ARR_DBG]]

// VPARAM: [[V_STR:%[0-9]+]] = OpString "v"
// VPARAM: [[V_DBG:%[A-Za-z0-9_]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[V_STR]]
// VPARAM: [[V_PARAM:%v[_0-9]*]] = OpFunctionParameter %_ptr_Function_S
// VPARAM: DebugDeclare [[V_DBG]] [[V_PARAM]]
// VPARAM-NOT: DebugValue [[V_DBG]]

// NOSHADOW-NOT: %{{_}}dbgvar_s
// NOSHADOW-NOT: %{{_}}dbgvar_arr
// NOSHADOW-NOT: %{{_}}dbgvar_v
