//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir

struct Counter
{
    int value;
};

extension Counter
{
    [ForceInline]
    int advance(int amount)
    {
        {
            int local = value + amount;
            return local;
        }
    }
}

RWStructuredBuffer<int> output;
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Counter counter = { int(tid.x) };
    {
        output[0] = counter.advance(1);
        output[1] = 101;
    }
    {
        output[2] = counter.advance(2);
        output[3] = 102;
    }
}

// The receiver and explicit parameter retain method ownership in both inline instances.
// Check the nested local's active inline scope and restoration at observable caller stores.
// clang-format off
// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[METHOD_NAME:%[0-9]+]] = OpString "Counter.advance"
// CHECK-DAG: [[THIS_NAME:%[0-9]+]] = OpString "this"
// CHECK-DAG: [[AMOUNT_NAME:%[0-9]+]] = OpString "amount"
// CHECK-DAG: [[LOCAL_NAME:%[0-9]+]] = OpString "local"
// CHECK-DAG: [[MAIN:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]]
// CHECK-DAG: [[METHOD:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[METHOD_NAME]]
// CHECK-DAG: [[CALLER_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE:%[A-Za-z_0-9]+]] %uint_27 %uint_9 [[MAIN]]
// CHECK-DAG: [[CALLER_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_31 %uint_9 [[MAIN]]
// CHECK-DAG: [[AT_A:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt %uint_27 [[CALLER_A]]{{$}}
// CHECK-DAG: [[AT_B:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt %uint_31 [[CALLER_B]]{{$}}
// CHECK-DAG: [[BODY:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_14 %uint_13 [[METHOD]]
// CHECK-DAG: [[THIS_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[THIS_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_11 %uint_9 [[METHOD]] %uint_0 %uint_1
// CHECK-DAG: [[THIS_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[THIS_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_11 %uint_9 [[METHOD]] %uint_0 %uint_1
// CHECK-DAG: [[ARG_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[AMOUNT_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_11 %uint_9 [[METHOD]] %uint_0 %uint_2
// CHECK-DAG: [[ARG_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[AMOUNT_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_11 %uint_9 [[METHOD]] %uint_0 %uint_2
// CHECK-DAG: [[LOCAL_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[LOCAL_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_14 %uint_17 [[BODY]] %uint_0
// CHECK-DAG: [[LOCAL_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLocalVariable [[LOCAL_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] %uint_14 %uint_17 [[BODY]] %uint_0
// CHECK: DebugScope [[METHOD]] [[AT_A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[ARG_A]]
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[THIS_A]]
// CHECK: DebugScope [[BODY]] [[AT_A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[LOCAL_A]]
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: [[SUM_A:%[A-Za-z_0-9]+]] = OpIAdd {{.*}} %int_1
// CHECK: DebugScope [[CALLER_A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} [[SUM_A]]
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_101
// CHECK: DebugScope [[METHOD]] [[AT_B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[ARG_B]]
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[THIS_B]]
// CHECK: DebugScope [[BODY]] [[AT_B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugDeclare [[LOCAL_B]]
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: [[SUM_B:%[A-Za-z_0-9]+]] = OpIAdd {{.*}} %int_2
// CHECK: DebugScope [[CALLER_B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} [[SUM_B]]
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_102
// clang-format on
