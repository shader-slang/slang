// Identical constants written in `spirv_asm` share one id, but a constant that its block
// decorates keeps its own id, so the decoration does not spread to the other constants.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=COUNT): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DEC): -target spirv -entry computeMain -stage compute

// CHECK-DAG: OpDecorate %[[RELAXED:[A-Za-z0-9_]+]] RelaxedPrecision
// CHECK-DAG: %[[RELAXED]] = OpConstant %int 12345
// CHECK-DAG: %[[PLAIN:[A-Za-z0-9_]+]] = OpConstant %int 12345{{ *$}}

// Two constants: the decorated one, and one shared by the two undecorated blocks.
// COUNT-COUNT-2: OpConstant %int 12345
// COUNT-NOT: OpConstant %int 12345

// Only the decorated constant is relaxed.
// DEC-COUNT-1: OpDecorate {{.*}}RelaxedPrecision
// DEC-NOT: OpDecorate {{.*}}RelaxedPrecision

RWStructuredBuffer<int> outputBuffer;

int relaxedValue()
{
    return spirv_asm
    {
        %c = OpConstant $$int 12345;
        OpDecorate %c RelaxedPrecision;
        result:$$int = OpCopyObject %c
    };
}

int plainValue()
{
    return spirv_asm
    {
        %c = OpConstant $$int 12345;
        result:$$int = OpCopyObject %c
    };
}

int plainValueAgain()
{
    return spirv_asm
    {
        %c = OpConstant $$int 12345;
        result:$$int = OpCopyObject %c
    };
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = relaxedValue();
    outputBuffer[1] = plainValue();
    outputBuffer[2] = plainValueAgain();
}
