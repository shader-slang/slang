// Debugger support is temporarily non-functional while debug-scope tracking is redesigned.
//DISABLE_TEST:SIMPLE(filecheck=CHECK,SCOPE_COUNT): -target hgbc -entry main -g2 -validate-ir

[host]
void main()
{
    for (uint index = 0; index < 1; ++index)
    {
        uint bodyLocal = index;
        print(bodyLocal);
    }
    print(index);
}

// CHECK: Section("META")
// CHECK-DAG: [[FUNCTION:%[A-Za-z0-9_]+]] = OpDebugFunction %strMain
// CHECK-DAG: [[BODY:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[FUNCTION]]
// CHECK-DAG: [[FOR_BODY:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[BODY]]
// CHECK: Section("FUNC")
// CHECK-DAG: OpDebugVariable %strIndex {{.*}} [[BODY]]
// CHECK-DAG: OpDebugVariable %strBodylocal {{.*}} [[FOR_BODY]]

// SCOPE_COUNT: Section("META")
// SCOPE_COUNT-COUNT-2: OpDebugLexicalBlock
// SCOPE_COUNT-NOT: OpDebugLexicalBlock
