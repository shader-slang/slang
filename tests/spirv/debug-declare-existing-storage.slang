// Regression test for https://github.com/shader-slang/slang/issues/13841
// When a source variable has program storage, its `DebugDeclare` names that storage instead of a
// separate `_dbgvar_*` copy. Program storage is a local's own Function-storage `OpVariable`, a
// pointer parameter (`inout`, `out`, `__constref`), or the proxy variable of a by-value
// parameter that the function modifies. Variables without such storage keep the copy.
//
// Note: -g2 embeds the entire source as one OpString, so the CHECK lines anchor on the real
// `= OpString "<name>"` and on `OpName %id "<name>"` (the source blob escapes quotes as \").
// Each variable has its own name, so that its OpString identifies it.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir
// NOCOPY repeats the CHECK compile with a prefix that has only -NOT lines after one anchor, so that
// it scans the whole module rather than the gaps between CHECK matches.
//TEST:SIMPLE(filecheck=NOCOPY): -target spirv-asm -entry computeMain -stage compute -g2 -O0
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
void bump(inout S s, inout float scale)
{
    s.a += 1;
    scale *= 2;
}

[noinline]
void produce(out float result)
{
    result = outputBuffer[1];
}

[noinline]
float overrideOne(float vals[4], int k)
{
    vals[k] = 7;
    return vals[0] + vals[1];
}

[noinline]
float readHeld(__constref S held)
{
    return held.b;
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
    int k = int(tid.x & 3);
    outputBuffer[tid.x] = acc[k] + st.a + f + r + overrideOne(acc, k) + readHeld(st);
}

// acc[0] + (st.a + 1) + f * 2 + r + (7 + acc[1]) + st.b = 1 + 6 + 12 + 2 + 9 + 2.
// The single thread writes only outputBuffer[0]; outputBuffer[1] keeps its input.
// BUF: 32.0
// BUF-NEXT: 2.0

// CHECK-DAG: [[ACC_STR:%[0-9]+]] = OpString "acc"
// CHECK-DAG: [[ST_STR:%[0-9]+]] = OpString "st"
// CHECK-DAG: [[F_STR:%[0-9]+]] = OpString "f"
// CHECK-DAG: [[R_STR:%[0-9]+]] = OpString "r"
// CHECK-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// CHECK-DAG: [[SCALE_STR:%[0-9]+]] = OpString "scale"
// CHECK-DAG: [[RESULT_STR:%[0-9]+]] = OpString "result"
// CHECK-DAG: [[VALS_STR:%[0-9]+]] = OpString "vals"
// CHECK-DAG: [[HELD_STR:%[0-9]+]] = OpString "held"

// `tid` (an entry-point input) and the loop counter `i` (promoted to SSA) have no program
// storage, so they keep the `_dbgvar_` copy.
// CHECK-DAG: OpName %_dbgvar_tid "_dbgvar_tid"
// CHECK-DAG: OpName %_dbgvar_i "_dbgvar_i"

// CHECK-DAG: [[ACC_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ACC_STR]]
// CHECK-DAG: [[ST_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ST_STR]]
// CHECK-DAG: [[F_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[F_STR]]
// CHECK-DAG: [[R_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[R_STR]]
// CHECK-DAG: [[S_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[S_STR]]
// CHECK-DAG: [[SCALE_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[SCALE_STR]]
// CHECK-DAG: [[RESULT_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[RESULT_STR]]
// CHECK-DAG: [[VALS_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[VALS_STR]]
// CHECK-DAG: [[HELD_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[HELD_STR]]

// CHECK-LABEL: %computeMain = OpFunction
// CHECK-DAG: DebugDeclare [[ACC_DBG]] %acc {{%[0-9]+}}{{$}}
// CHECK-DAG: DebugDeclare [[ST_DBG]] %st {{%[0-9]+}}{{$}}
// CHECK-DAG: DebugDeclare [[F_DBG]] %f {{%[0-9]+}}{{$}}
// CHECK-DAG: DebugDeclare [[R_DBG]] %r {{%[0-9]+}}{{$}}
// CHECK-DAG: OpFunctionCall %void %bump %st %f

// CHECK-LABEL: %bump = OpFunction
// CHECK: [[S_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_S
// CHECK: [[SCALE_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK-DAG: DebugDeclare [[S_DBG]] [[S_PARAM]] {{%[0-9]+}}{{$}}
// CHECK-DAG: DebugDeclare [[SCALE_DBG]] [[SCALE_PARAM]] {{%[0-9]+}}{{$}}
// `s.a += 1` is described by a DebugValue of the member, with no access chain into the debug
// variable.
// CHECK: OpFAdd
// CHECK-NOT: OpAccessChain {{%[A-Za-z_0-9]+}} [[S_DBG]]
// CHECK: DebugValue [[S_DBG]] {{%[0-9]+}} {{%[0-9]+}} %int_0

// CHECK-LABEL: %produce = OpFunction
// CHECK: [[RESULT_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK: DebugDeclare [[RESULT_DBG]] [[RESULT_PARAM]] {{%[0-9]+}}{{$}}

// `vals` is modified, so it lives in a proxy variable that the function copies it into.
// CHECK-LABEL: %overrideOne = OpFunction
// CHECK: [[VALS_PROXY:%[A-Za-z_0-9]+]] = OpVariable %_ptr_Function__arr_float_int_4 Function
// CHECK: DebugDeclare [[VALS_DBG]] [[VALS_PROXY]] {{%[0-9]+}}{{$}}

// CHECK-LABEL: %readHeld = OpFunction
// CHECK: [[HELD_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_S
// CHECK: DebugDeclare [[HELD_DBG]] [[HELD_PARAM]] {{%[0-9]+}}{{$}}

// No variable with program storage has a `_dbgvar_` copy anywhere in the module.
// NOCOPY: OpEntryPoint
// NOCOPY-NOT: %_dbgvar_{{(acc|st|f|r|s|scale|result|vals|held)(_[0-9]+)?[^A-Za-z_0-9]}}

// spirv-opt deletes a store-only `_dbgvar_` copy together with its DebugDeclare, but keeps a
// DebugDeclare on storage that the program reads.
// OPT-DAG: [[ACC_STR:%[0-9]+]] = OpString "acc"
// OPT-DAG: [[ST_STR:%[0-9]+]] = OpString "st"
// OPT-DAG: [[F_STR:%[0-9]+]] = OpString "f"
// OPT-DAG: [[R_STR:%[0-9]+]] = OpString "r"
// OPT-DAG: [[S_STR:%[0-9]+]] = OpString "s"
// OPT-DAG: [[SCALE_STR:%[0-9]+]] = OpString "scale"
// OPT-DAG: [[RESULT_STR:%[0-9]+]] = OpString "result"
// OPT-DAG: [[VALS_STR:%[0-9]+]] = OpString "vals"
// OPT-DAG: [[HELD_STR:%[0-9]+]] = OpString "held"
// OPT-DAG: [[ACC_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ACC_STR]]
// OPT-DAG: [[ST_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[ST_STR]]
// OPT-DAG: [[F_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[F_STR]]
// OPT-DAG: [[R_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[R_STR]]
// OPT-DAG: [[S_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[S_STR]]
// OPT-DAG: [[SCALE_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[SCALE_STR]]
// OPT-DAG: [[RESULT_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[RESULT_STR]]
// OPT-DAG: [[VALS_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[VALS_STR]]
// OPT-DAG: [[HELD_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[HELD_STR]]
// OPT-LABEL: %computeMain = OpFunction
// OPT-DAG: DebugDeclare [[ACC_DBG]] %acc {{%[0-9]+}}{{$}}
// OPT-DAG: DebugDeclare [[ST_DBG]] %st {{%[0-9]+}}{{$}}
// OPT-DAG: DebugDeclare [[F_DBG]] %f {{%[0-9]+}}{{$}}
// OPT-DAG: DebugDeclare [[R_DBG]] %r {{%[0-9]+}}{{$}}
// OPT-LABEL: %bump = OpFunction
// OPT: [[S_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_S
// OPT: [[SCALE_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// OPT-DAG: DebugDeclare [[S_DBG]] [[S_PARAM]] {{%[0-9]+}}{{$}}
// OPT-DAG: DebugDeclare [[SCALE_DBG]] [[SCALE_PARAM]] {{%[0-9]+}}{{$}}
// OPT-LABEL: %produce = OpFunction
// OPT: [[RESULT_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// OPT: DebugDeclare [[RESULT_DBG]] [[RESULT_PARAM]] {{%[0-9]+}}{{$}}
// OPT-LABEL: %overrideOne = OpFunction
// OPT: [[VALS_PROXY:%[A-Za-z_0-9]+]] = OpVariable %_ptr_Function__arr_float_int_4 Function
// OPT: DebugDeclare [[VALS_DBG]] [[VALS_PROXY]] {{%[0-9]+}}{{$}}
// OPT-LABEL: %readHeld = OpFunction
// OPT: [[HELD_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_S
// OPT: DebugDeclare [[HELD_DBG]] [[HELD_PARAM]] {{%[0-9]+}}{{$}}

// NODEBUG: OpEntryPoint
// NODEBUG-NOT: DebugDeclare
// NODEBUG-NOT: _dbgvar_
