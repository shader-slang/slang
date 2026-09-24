//TEST:SIMPLE(filecheck=CHECK):-entry computeMain -stage compute -target spirv-asm -emit-spirv-directly -O0 -loop-inversion -g2
//TEST:SIMPLE(filecheck=CHECK):-entry computeMain -stage compute -target spirv-asm -emit-spirv-directly -O0 -loop-inversion -g3

// The break block updates a variable that is only observed through debug information.
// A DebugValue has no result users, but erasing it would lose the assignment to debugState.
// CHECK-DAG: [[STATE_NAME:%[A-Za-z0-9_]+]] = OpString "debugState"
// CHECK-DAG: [[STATE:%[A-Za-z0-9_]+]] = OpExtInst %void {{%[A-Za-z0-9_]+}} DebugLocalVariable [[STATE_NAME]]
// CHECK-DAG: [[SEVEN:%[A-Za-z0-9_]+]] = OpConstant %int 7
// CHECK: OpExtInst %void {{%[A-Za-z0-9_]+}} DebugValue [[STATE]] [[SEVEN]]

RWStructuredBuffer<int> outputBuffer;

int sum()
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
    outputBuffer[tid.x] = sum();
}
