// Regression test for https://github.com/shader-slang/slang/issues/13841
// When a source variable keeps its own Function-storage `OpVariable` (or arrives as an
// `inout`/`out` pointer parameter), its `DebugDeclare` names that storage instead of a
// separate `_dbgvar_*` copy. Variables without such storage keep the copy.
//
// Note: -g2 embeds the entire source as one OpString, so the CHECK lines anchor on the real
// `= OpString "<name>"` and on `OpName %id "<name>"` (the source blob escapes quotes as \").

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -g2 -O0
//TEST:SIMPLE(filecheck=OPT): -target spirv-asm -entry computeMain -stage compute -g2 -O1
//TEST:SIMPLE(filecheck=NODEBUG): -target spirv-asm -entry computeMain -stage compute -g0 -O0
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -xslang -g2 -xslang -O0

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct S
{
    float a;
    float b;
};

[noinline]
void bump(inout S s, inout float f)
{
    s.a += 1;
    f *= 2;
}

[noinline]
void produce(out float r)
{
    r = outputBuffer[1];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float acc[4];
    for (int i = 0; i < 4; i++)
        acc[i] = outputBuffer[i];
    S st = { outputBuffer[4], 2 };
    float f = outputBuffer[5];
    bump(st, f);
    float r;
    produce(r);
    outputBuffer[tid.x] = acc[tid.x & 3] + st.a + f + r;
}

// acc[0] + (st.a + 1) + f * 2 + r = 1 + 6 + 12 + 2.
// BUF: 21.0
// BUF-NEXT: 2.0

// CHECK-DAG: [[ACC_STR:%[0-9]+]] = OpString "acc"
// CHECK-DAG: [[ST_STR:%[0-9]+]] = OpString "st"
// CHECK-DAG: [[F_STR:%[0-9]+]] = OpString "f"
// CHECK-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK-DAG: [[R_STR:%[0-9]+]] = OpString "r"

// `tid` (an entry-point input) and the loop counter `i` (promoted to SSA) have no storage of
// their own, so they keep the debug-only copy. Variables that do have storage get no copy.
// CHECK-DAG: OpName %_dbgvar_tid "_dbgvar_tid"
// CHECK-DAG: OpName %_dbgvar_i "_dbgvar_i"
// CHECK-NOT: OpName {{%[A-Za-z_0-9]+}} "_dbgvar_{{(acc|st|f|s|r)}}"

// CHECK-DAG: [[ACC_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ACC_STR]]
// CHECK-DAG: [[ST_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ST_STR]]
// CHECK-DAG: [[F_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[F_STR]] {{%[0-9]+}} {{%[0-9]+}} %uint_43 %uint_11
// CHECK-DAG: [[R_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[R_STR]] {{%[0-9]+}} {{%[0-9]+}} %uint_45 %uint_11
// CHECK-DAG: [[S_PARAM_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[S_STR]]
// CHECK-DAG: [[F_PARAM_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[F_STR]] {{%[0-9]+}} {{%[0-9]+}} %uint_24 %uint_6
// CHECK-DAG: [[R_PARAM_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[R_STR]] {{%[0-9]+}} {{%[0-9]+}} %uint_31 %uint_6

// CHECK-LABEL: %computeMain = OpFunction
// CHECK-DAG: DebugDeclare [[ACC_DBG]] %acc {{%[0-9]+}}{{$}}
// CHECK-DAG: DebugDeclare [[ST_DBG]] %st {{%[0-9]+}}{{$}}
// CHECK-DAG: DebugDeclare [[F_DBG]] %f {{%[0-9]+}}{{$}}
// CHECK-DAG: DebugDeclare [[R_DBG]] %r {{%[0-9]+}}{{$}}
// CHECK-DAG: OpFunctionCall %void %bump %st %f

// CHECK-LABEL: %bump = OpFunction
// CHECK: [[S_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_S
// CHECK: [[F_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK-DAG: DebugDeclare [[S_PARAM_DBG]] [[S_PARAM]] {{%[0-9]+}}{{$}}
// CHECK-DAG: DebugDeclare [[F_PARAM_DBG]] [[F_PARAM]] {{%[0-9]+}}{{$}}

// CHECK-LABEL: %produce = OpFunction
// CHECK: [[R_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK: DebugDeclare [[R_PARAM_DBG]] [[R_PARAM]] {{%[0-9]+}}{{$}}

// spirv-opt deletes a store-only debug copy together with its DebugDeclare, but keeps a
// DebugDeclare on storage that the program reads.
// OPT-DAG: [[ACC_STR:%[0-9]+]] = OpString "acc"
// OPT-DAG: [[ST_STR:%[0-9]+]] = OpString "st"
// OPT-DAG: [[F_STR:%[0-9]+]] = OpString "f"
// OPT-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// OPT-DAG: [[ACC_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ACC_STR]]
// OPT-DAG: [[ST_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ST_STR]]
// OPT-DAG: [[F_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[F_STR]] {{%[0-9]+}} {{%[0-9]+}} %uint_43 %uint_11
// OPT-DAG: [[S_PARAM_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[S_STR]]
// OPT-DAG: [[F_PARAM_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[F_STR]] {{%[0-9]+}} {{%[0-9]+}} %uint_24 %uint_6
// OPT-LABEL: %computeMain = OpFunction
// OPT-DAG: DebugDeclare [[ACC_DBG]] %acc {{%[0-9]+}}{{$}}
// OPT-DAG: DebugDeclare [[ST_DBG]] %st {{%[0-9]+}}{{$}}
// OPT-DAG: DebugDeclare [[F_DBG]] %f {{%[0-9]+}}{{$}}
// OPT-LABEL: %bump = OpFunction
// OPT-DAG: DebugDeclare [[S_PARAM_DBG]] %s {{%[0-9]+}}{{$}}
// OPT-DAG: DebugDeclare [[F_PARAM_DBG]] %f_1 {{%[0-9]+}}{{$}}

// Without debug info there are no debug variables, and so no declarations or copies.
// NODEBUG-NOT: DebugDeclare
// NODEBUG-NOT: _dbgvar_
// NODEBUG: OpFunctionCall %void %bump
