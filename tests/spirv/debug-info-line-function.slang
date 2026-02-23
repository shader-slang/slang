//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -preserve-embedded-source
// Tests generation of DebugFunction and DebugLine with #line preprocessor directive specifying different files

// Get the IDs for the Source argument in DebugFunction/DebugLine
//CHECK-DAG: [[DEBUGINFOLINE_FILE:%[0-9]+]] = OpString {{.*}}debug-info-line-function.slang"
//CHECK-DAG: [[B_FILE:%[0-9]+]] = OpString {{.*}}b.slang"
//CHECK-DAG: [[D_FILE:%[0-9]+]] = OpString {{.*}}d.slang"
//CHECK-DAG: [[DEBUGINFOLINE_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[DEBUGINFOLINE_FILE]]
//CHECK-DAG: [[B_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[B_FILE]]
//CHECK-DAG: [[D_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[D_FILE]]

// DebugFunctions are declared together in the "; Types, variables and constants" section
// Assume the integers for the line numbers are in the form %uint_{linenumber}
//CHECK: DebugFunction %{{[0-9]+}} %{{[0-9]+}} [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+9]]
//CHECK: DebugFunction %{{[0-9]+}} %{{[0-9]+}} [[B_SOURCE]] %uint_99
//CHECK: DebugFunction %{{[0-9]+}} %{{[0-9]+}} [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+20]]
//CHECK: DebugFunction %{{[0-9]+}} %{{[0-9]+}} [[D_SOURCE]] %uint_603
//CHECK: DebugFunction %{{[0-9]+}} %{{[0-9]+}} [[D_SOURCE]] %uint_40
//CHECK: DebugFunction %{{[0-9]+}} %{{[0-9]+}} [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+38]]

//CHECK:; Function a
//CHECK: DebugLine [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+1]]
int a() { return 0; }

//CHECK:; Function b
//CHECK: DebugLine [[B_SOURCE]] %uint_99
#line 99 "b.slang"
int b()
{
    return 0;
}

//CHECK:; Function c
//CHECK: DebugLine [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+2]]
#line default
int c()
{
    return 0;
}

//CHECK:; Function d
//CHECK: DebugLine [[D_SOURCE]] %uint_603
#line 603 "d.slang"
int d() { return 0; }

//CHECK:; Function e
//CHECK: DebugLine [[D_SOURCE]] %uint_40
#line 40
int e()
{
    return 0;
}

//CHECK:; Function f
//CHECK: DebugLine [[DEBUGINFOLINE_SOURCE]] %uint_[[#@LINE+2]]
#line
int f() { return 0; }

RWStructuredBuffer<int> out;
[shader("compute")]
void main()
{
    out[0] = a() + b() + c() + d() + e() + f();
}
