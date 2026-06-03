//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

int helper()
{
    __target_switch
    {
    case hlsl:
        return 1;
    case spirv:
        // C-like emit ordering must not chase operands inside SPIR-V-only asm blocks.
        return spirv_asm
        {
            OpName $computeMain "computeMain";
            OpConstant $$int result 1;
        };
    }
}

// CHECK: [numthreads(1, 1, 1)]
// CHECK: void computeMain()
[numthreads(1, 1, 1)]
void computeMain()
{
    int value = helper();
}
