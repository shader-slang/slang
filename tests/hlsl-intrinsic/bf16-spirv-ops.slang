//TEST:SIMPLE(filecheck=SPV): -target spirv -emit-spirv-directly -O0

// Verify BFloat16 arithmetic emits correct SPIR-V float opcodes.

// SPV-DAG: OpCapability BFloat16TypeKHR
// SPV-DAG: OpExtension "SPV_KHR_bfloat16"

// SPV-DAG: OpFAdd %bfloat16
// SPV-DAG: OpFSub %bfloat16
// SPV-DAG: OpFMul %bfloat16
// SPV-DAG: OpFDiv %bfloat16
// SPV-DAG: OpFNegate %bfloat16

// SPV-DAG: OpFOrdEqual %bool
// SPV-DAG: OpFUnordNotEqual %bool
// SPV-DAG: OpFOrdLessThan %bool
// SPV-DAG: OpFOrdGreaterThan %bool

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(3.0);
    BFloat16 b = BFloat16(2.0);

    BFloat16 sum = a + b;
    BFloat16 diff = a - b;
    BFloat16 prod = a * b;
    BFloat16 quot = a / b;
    BFloat16 neg = -a;

    bool eq = a == b;
    bool neq = a != b;
    bool lt = a < b;
    bool gt = a > b;

    outputBuffer[0] = int(float(sum));
    outputBuffer[1] = int(eq);
    outputBuffer[2] = int(float(neg));
}
