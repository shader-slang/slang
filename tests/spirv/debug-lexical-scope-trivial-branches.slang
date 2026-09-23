//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O1 -validate-ir

// Empty arms still acquire lexical scope markers. They must not keep otherwise
// redundant control flow alive, and the following store belongs to main.
// CHECK-DAG: [[NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK: OpFunction
// CHECK-NOT: OpBranch
// CHECK-NOT: OpSwitch
// CHECK: DebugScope [[FUNC]]{{$}}
// CHECK-NOT: OpBranch
// CHECK-NOT: OpSwitch
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore
// CHECK-NOT: OpBranch
// CHECK-NOT: OpSwitch
// CHECK: OpFunctionEnd

RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x != 0)
    {
    }
    else
    {
    }
    switch (tid.y)
    {
    case 0:
        { break; }
    default:
        { break; }
    }
    outputBuffer[0] = 123;
}
