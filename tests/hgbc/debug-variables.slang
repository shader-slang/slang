// Debugger support is temporarily non-functional while debug-scope tracking is redesigned.
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -g2

[noinline]
uint addOne(uint input)
{
    uint local = input + 1;
    return local;
}

[host]
void main()
{
    uint value = 4;
    uint result = addOne(value);
    print(result);
}

// CHECK: Section("META")
// CHECK-DAG: [[MAIN_FUNC:%[A-Za-z0-9_]+]] = OpDebugFunction %strMain
// CHECK-DAG: [[MAIN_SCOPE:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[MAIN_FUNC]]
// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK: Section("FUNC")
// CHECK: OpDebugScope [[MAIN_SCOPE]]
// CHECK-DAG: [[VALUE_VAR:%[A-Za-z0-9_]+]] = OpDebugVariable %strValue [[UINT]]
// CHECK-DAG: OpDebugValue [[VALUE_VAR]]
// CHECK-DAG: [[RESULT_VAR:%[A-Za-z0-9_]+]] = OpDebugVariable %strResult [[UINT]]
// CHECK-DAG: OpDebugValue [[RESULT_VAR]]
// CHECK-DAG: [[INPUT_VAR:%[A-Za-z0-9_]+]] = OpDebugVariable %strInput [[UINT]]
// CHECK-DAG: OpDebugValue [[INPUT_VAR]]
// CHECK-DAG: [[LOCAL_VAR:%[A-Za-z0-9_]+]] = OpDebugVariable %strLocal [[UINT]]
// CHECK-DAG: OpDebugValue [[LOCAL_VAR]]
