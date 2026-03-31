//TEST:SIMPLE(filecheck=SPV): -target spirv -emit-spirv-directly -O0

// Verify BFloat16 arithmetic emits correct SPIR-V float opcodes.

// SPV-DAG: OpCapability BFloat16TypeKHR
// SPV-DAG: OpExtension "SPV_KHR_bfloat16"

// SPV-DAG: OpFAdd %bfloat16
// SPV-DAG: OpFNegate %bfloat16

// SPV-DAG: OpFOrdEqual %bool
// SPV-DAG: OpFUnordNotEqual %bool
// SPV-DAG: OpFOrdLessThan %bool
// SPV-DAG: OpFOrdGreaterThan %bool

RWStructuredBuffer<int> outputBuffer;

void use(int v, int idx) { outputBuffer[idx] = v; }

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(float(outputBuffer[0]));
    BFloat16 b = BFloat16(float(outputBuffer[1]));

    BFloat16 sum = a + b;
    BFloat16 neg = -a;

    bool eq = (a == b);
    bool neq = (a != b);
    bool lt = (a < b);
    bool gt = (a > b);

    use(int(float(sum)), 0);
    use(int(float(neg)), 1);
    use(eq ? 1 : 0, 2);
    use(neq ? 1 : 0, 3);
    use(lt ? 1 : 0, 4);
    use(gt ? 1 : 0, 5);
}
