//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir

// Address-space specialization copies the executable function. Each copy must retain
// its own debug function and lexical declaration tree, just like typeflow specialization.
// CHECK-DAG: [[NAME:%[0-9]+]] = OpString "modify"
// CHECK-DAG: [[LOCAL_NAME:%[0-9]+]] = OpString "value"
// CHECK-DAG: [[FUNC_A:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK-DAG: [[FUNC_B:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK-DAG: [[BLOCK_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock {{.*}} [[FUNC_A]]
// CHECK-DAG: [[BLOCK_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock {{.*}} [[FUNC_B]]
// CHECK-DAG: [[LOCAL_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[LOCAL_NAME]] {{.*}} [[BLOCK_A]]
// CHECK-DAG: [[LOCAL_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[LOCAL_NAME]] {{.*}} [[BLOCK_B]]
// CHECK: [[EXEC_A:%[A-Za-z_0-9]+]] = OpFunction {{.*}} DontInline
// CHECK-NOT: OpFunctionEnd
// CHECK: DebugFunctionDefinition [[FUNC_A]] [[EXEC_A]]
// CHECK: DebugScope [[BLOCK_A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: Debug{{(Declare|Value)}} [[LOCAL_A]]
// CHECK: OpFunctionEnd
// CHECK: [[EXEC_B:%[A-Za-z_0-9]+]] = OpFunction {{.*}} DontInline
// CHECK-NOT: OpFunctionEnd
// CHECK: DebugFunctionDefinition [[FUNC_B]] [[EXEC_B]]
// CHECK: DebugScope [[BLOCK_B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: Debug{{(Declare|Value)}} [[LOCAL_B]]

static int privateArray[2];
groupshared int sharedArray[2];
RWStructuredBuffer<int> outputBuffer;

[noinline]
void modify(inout int array[2])
{
    {
        int value = array[0] + 1;
        array[1] = value;
    }
}
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    modify(privateArray);
    modify(sharedArray);
    outputBuffer[0] = privateArray[1] + sharedArray[1];
}
