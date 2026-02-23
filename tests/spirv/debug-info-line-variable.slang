//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -preserve-embedded-source
// Tests generation of DebugLocalVariable and DebugDeclare with #line preprocessor directive specifying different files

// Get the IDs for the Source argument in DebugLine/DebugFunction/DebugLocalVariable
//CHECK-DAG: [[DEBUGINFOLINE_FILE:%[0-9]+]] = OpString {{.*}}debug-info-line-variable.slang"
//CHECK-DAG: [[B_FILE:%[0-9]+]] = OpString {{.*}}b.slang"
//CHECK-DAG: [[DEBUGINFOLINE_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[DEBUGINFOLINE_FILE]]
//CHECK-DAG: [[B_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[B_FILE]]

// Get the IDs for the variable strings
//CHECK-DAG: [[T_STR:%[0-9]+]] = OpString "t"
//CHECK-DAG: [[U_STR:%[0-9]+]] = OpString "u"
//CHECK-DAG: [[V_STR:%[0-9]+]] = OpString "v"
//CHECK-DAG: [[I_STR:%[0-9]+]] = OpString "i"
//CHECK-DAG: [[J_STR:%[0-9]+]] = OpString "j"
//CHECK-DAG: [[K_STR:%[0-9]+]] = OpString "k"

// Assume the integers for the line numbers are in the form %uint_{linenumber}
// Check for DebugLocalVariable's Name, Source, and Line arguments
//CHECK: DebugFunction %{{[0-9]+}} %{{[0-9]+}} [[B_SOURCE]] %uint_[[#%u,B_FUNC_LINE:]]
//CHECK: [[T_VAR:%[a-zA-Z0-9_]+]] = {{.*}} DebugLocalVariable [[T_STR]] %{{[0-9]+}} [[B_SOURCE]] %uint_[[#B_FUNC_LINE]]
//CHECK: [[U_VAR:%[a-zA-Z0-9_]+]] = {{.*}} DebugLocalVariable [[U_STR]] %{{[0-9]+}} [[B_SOURCE]] %uint_[[#B_FUNC_LINE+4]]
//CHECK: [[V_VAR:%[a-zA-Z0-9_]+]] = {{.*}} DebugLocalVariable [[V_STR]] %{{[0-9]+}} [[B_SOURCE]] %uint_[[#B_FUNC_LINE+7]]

//CHECK: DebugFunction %{{[0-9]+}} %{{[0-9]+}} [[DEBUGINFOLINE_SOURCE]] %uint_[[#%u,C_FUNC_LINE:]]
//CHECK: [[I_VAR:%[a-zA-Z0-9_]+]] = {{.*}} DebugLocalVariable [[I_STR]] %{{[0-9]+}} [[DEBUGINFOLINE_SOURCE]] %uint_[[#C_FUNC_LINE]]
//CHECK: [[J_VAR:%[a-zA-Z0-9_]+]] = {{.*}} DebugLocalVariable [[J_STR]] %{{[0-9]+}} [[DEBUGINFOLINE_SOURCE]] %uint_[[#C_FUNC_LINE+4]]
//CHECK: [[K_VAR:%[a-zA-Z0-9_]+]] = {{.*}} DebugLocalVariable [[K_STR]] %{{[0-9]+}} [[DEBUGINFOLINE_SOURCE]] %uint_[[#C_FUNC_LINE+7]]

//CHECK:; Function b
//CHECK: DebugLine [[B_SOURCE]] %uint_[[#B_FUNC_LINE: == 99]]
//CHECK: DebugDeclare [[T_VAR]]
#line 99 "b.slang"
int b(int t)
{
//CHECK: DebugLine [[B_SOURCE]] %uint_[[#99+4]]
//CHECK: DebugDeclare [[U_VAR]]
    int u = 2;
//CHECK: DebugLine [[B_SOURCE]] %uint_[[#99+7]]
//CHECK: DebugDeclare [[V_VAR]]
    let v = 3;
//CHECK: DebugLine [[B_SOURCE]] %uint_[[#99+9]]
    return t - u - v;
}

//CHECK:; Function c
//CHECK: DebugLine [[DEBUGINFOLINE_SOURCE]] %uint_[[#C_FUNC_LINE: == @LINE+3]]
//CHECK: DebugDeclare [[I_VAR]]
#line default
int c(int i)
{
//CHECK: DebugLine [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+2]]
//CHECK: DebugDeclare [[J_VAR]]
    int j = 0;
//CHECK: DebugLine [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+2]]
//CHECK: DebugDeclare [[K_VAR]]
    let k = 1;
//CHECK: DebugLine [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+1]]
    return i + j + k;
}

RWStructuredBuffer<int> out;
[shader("compute")]
void main()
{
    out[0] = b(10) + c(-1);
}
