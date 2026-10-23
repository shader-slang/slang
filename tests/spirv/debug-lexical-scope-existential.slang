// Each existential specialization needs a distinct debug function and local scope.
//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir

interface IValue
{
    int get();
}

struct First : IValue
{
    int value;
    int get() { return value + 1; }
}

struct Second : IValue
{
    int value;
    int get() { return value + 2; }
}

[noinline]
int evaluate(IValue value)
{
    {
        int scopedValue = value.get();
        return scopedValue + 3;
    }
}

RWStructuredBuffer<int> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    First first = { int(tid.x) };
    Second second = { int(tid.y) };
    outputBuffer[0] = evaluate(first);
    outputBuffer[1] = evaluate(second);
}

// CHECK-DAG: [[NAME:%[0-9]+]] = OpString "evaluate"
// CHECK-DAG: [[LOCAL_NAME:%[0-9]+]] = OpString "scopedValue"
// CHECK-DAG: [[FUNC_A:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK-DAG: [[FUNC_B:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK-DAG: [[BLOCK_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE:%[A-Za-z_0-9]+]] %uint_25 %uint_9 [[FUNC_A]]
// CHECK-DAG: [[BLOCK_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_25 %uint_9 [[FUNC_B]]
// CHECK-DAG: [[LOCAL_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[LOCAL_NAME]] {{.*}} [[SOURCE]] %uint_25 %uint_13 [[BLOCK_A]]
// CHECK-DAG: [[LOCAL_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[LOCAL_NAME]] {{.*}} [[SOURCE]] %uint_25 %uint_13 [[BLOCK_B]]
// CHECK: [[IMPL_A:%[A-Za-z_0-9]+]] = OpFunction %int DontInline
// CHECK-NOT: OpFunctionEnd
// CHECK: DebugFunctionDefinition [[FUNC_A]] [[IMPL_A]]{{$}}
// CHECK: DebugScope [[BLOCK_A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: Debug{{(Value|Declare)}} [[LOCAL_A]]
// CHECK: OpFunctionEnd
// CHECK: [[IMPL_B:%[A-Za-z_0-9]+]] = OpFunction %int DontInline
// CHECK-NOT: OpFunctionEnd
// CHECK: DebugFunctionDefinition [[FUNC_B]] [[IMPL_B]]{{$}}
// CHECK: DebugScope [[BLOCK_B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: Debug{{(Value|Declare)}} [[LOCAL_B]]
