//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O1 -validate-ir

// The unused expression gives case 0 its own block during lowering. Once it is
// eliminated, only scope markers remain before the fall-through to case 1.
// Scope markers must not stop CFG simplification from forwarding both case
// labels to the same block.
// The surviving stores use the switch scope until the merge restores main's scope.
// CHECK-DAG: [[NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK-DAG: [[SWITCH:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock {{%[A-Za-z_0-9]+}} %uint_103 %uint_5 [[FUNC]]
// CHECK: = OpFunction
// CHECK: OpSelectionMerge [[MERGE:%[A-Za-z_0-9]+]] None
// CHECK: OpSwitch {{%[A-Za-z_0-9]+}} [[DEFAULT:%[A-Za-z_0-9]+]] 0 [[CASE:%[A-Za-z_0-9]+]] 1 [[CASE]] 2 [[OTHER:%[A-Za-z_0-9]+]]
// CHECK: [[DEFAULT]] = OpLabel
// CHECK: DebugScope [[SWITCH]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{%[A-Za-z_0-9]+}} %int_103{{$}}
// CHECK: [[CASE]] = OpLabel
// CHECK: DebugScope [[SWITCH]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{%[A-Za-z_0-9]+}} %int_101{{$}}
// CHECK: [[OTHER]] = OpLabel
// CHECK: DebugScope [[SWITCH]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{%[A-Za-z_0-9]+}} %int_102{{$}}
// CHECK: [[MERGE]] = OpLabel
// CHECK: DebugScope [[FUNC]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{%[A-Za-z_0-9]+}} %int_104{{$}}

RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
#line 100 "switch-fallthrough.slang"
{
    switch (tid.x)
    {
    case 0:
        { tid.y; }
    case 1:
        outputBuffer[0] = 101;
        break;
    case 2:
        outputBuffer[0] = 102;
        break;
    default:
        outputBuffer[0] = 103;
        break;
    }
    outputBuffer[1] = 104;
}
