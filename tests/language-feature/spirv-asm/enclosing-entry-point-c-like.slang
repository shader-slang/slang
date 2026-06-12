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

struct AsmResult
{
    int value;
};

RWStructuredBuffer<AsmResult> outputBuffer;

AsmResult typedHelper()
{
    __target_switch
    {
    case hlsl:
        AsmResult result;
        result.value = 1;
        return result;
    case spirv:
        return spirv_asm
        {
            OpName $computeMain "computeMain";
            result: $$AsmResult = OpConstantNull $$AsmResult;
        };
    }
}

// CHECK: struct AsmResult
// CHECK: [numthreads(1, 1, 1)]
// CHECK: void computeMain()
[numthreads(1, 1, 1)]
void computeMain()
{
    int value = helper();
    outputBuffer[0] = typedHelper();
}
