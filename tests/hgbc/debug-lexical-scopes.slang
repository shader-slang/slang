// Debugger support is temporarily non-functional while debug-scope tracking is redesigned.
//DISABLE_TEST:SIMPLE(filecheck=CHECK,SCOPE_COUNT): -target hgbc -entry main -g2 -validate-ir

[host]
void main()
{
    uint outer = 0;
    while (outer < 1)
    {
        uint whileLocal = outer;
        outer++;
    }

    do
        outer++;
    while (outer < 1);

    for (uint index = 0; index < 1; ++index)
    {
        uint forLocal = index;
        print(forLocal);
    }
}

// CHECK: Section("META")
// CHECK-DAG: [[FUNCTION:%[A-Za-z0-9_]+]] = OpDebugFunction %strMain
// CHECK-DAG: [[BODY:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[FUNCTION]] {{.*}} 6 5 22 1
// CHECK-DAG: [[WHILE_BODY:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[BODY]] {{.*}} 9 9 11 5
// CHECK-DAG: [[FOR_SCOPE:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[BODY]] {{.*}} 17 5 22 1
// CHECK-DAG: [[FOR_BODY:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[FOR_SCOPE]] {{.*}} 19 9 21 5
// CHECK: Section("FUNC")
// CHECK-DAG: OpDebugVariable %strOuter {{.*}} [[BODY]]
// CHECK-DAG: OpDebugVariable %strWhilelocal {{.*}} [[WHILE_BODY]]
// CHECK-DAG: OpDebugVariable %strIndex {{.*}} [[FOR_SCOPE]]
// CHECK-DAG: OpDebugVariable %strForlocal {{.*}} [[FOR_BODY]]

// SCOPE_COUNT: Section("META")
// SCOPE_COUNT-COUNT-4: OpDebugLexicalBlock
// SCOPE_COUNT-NOT: OpDebugLexicalBlock
