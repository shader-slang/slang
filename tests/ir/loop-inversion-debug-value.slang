//TEST:SIMPLE(filecheck=CHECK):-entry computeMain -stage compute -target spirv-asm -emit-spirv-directly -O0 -loop-inversion -g2
//TEST:SIMPLE(filecheck=CHECK):-entry computeMain -stage compute -target spirv-asm -emit-spirv-directly -O0 -loop-inversion -g3

// These loops differ only by a variable observed through debug information and its update
// before the break. Without the update, the break branch is trivial and inversion moves
// the condition after the body. With the update, inversion currently rejects the loop: treating
// DebugValue as an ignorable location or scope marker would erase debugState = 7.
// Check both loop shapes and the surviving update at each debug level.
// CHECK-DAG: [[STATE_NAME:%[A-Za-z0-9_]+]] = OpString "debugState"
// CHECK-DAG: [[STATE:%[A-Za-z0-9_]+]] = OpExtInst %void {{%[A-Za-z0-9_]+}} DebugLocalVariable [[STATE_NAME]]
// CHECK-DAG: [[SEVEN:%[A-Za-z0-9_]+]] = OpConstant %int 7

// The positive loop executes j += i and i++ before testing whether to continue.
// Anchor the function labels so they cannot match this shader's embedded debug source.
// CHECK-LABEL: {{^}}%sumWithoutDebugUpdate = OpFunction %int
// CHECK: OpLoopMerge
// CHECK-NOT: OpBranchConditional
// CHECK: OpIAdd %int
// CHECK: OpIAdd %int
// CHECK: OpBranchConditional
// CHECK: OpFunctionEnd

// The negative loop still tests the condition before executing the body, and its break
// branch retains the assignment even though DebugValue has no result users.
// This ordering reflects the current pass's inability to move DebugValue safely.
// If that becomes supported, update the loop-shape checks and verify that debugState = 7
// remains on the correct exit path.
// CHECK-LABEL: {{^}}%sumWithDebugUpdate = OpFunction %int
// CHECK: OpLoopMerge
// CHECK-NOT: OpIAdd
// CHECK: OpBranchConditional
// CHECK: OpExtInst %void {{%[A-Za-z0-9_]+}} DebugValue [[STATE]] [[SEVEN]]
// CHECK: OpFunctionEnd

RWStructuredBuffer<int> outputBuffer;

// This loop has a trivial break branch, so it should be inverted.
int sumWithoutDebugUpdate()
{
    int j = 0;
    int i = 0;
    do
    {
        if (i < 10)
            {}
        else
            break;
        j += i;
        i++;
    } while (true);
    return j;
}

int sumWithDebugUpdate()
{
    int j = 0;
    int i = 0;
    int debugState = 0;
    do
    {
        if (i < 10)
            {}
        else
        {
            debugState = 7;
            break;
        }
        j += i;
        i++;
    } while (true);
    return j;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = sumWithoutDebugUpdate() + sumWithDebugUpdate();
}
