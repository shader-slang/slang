//TEST:SIMPLE(filecheck=SPV): -target spirv -emit-spirv-directly -O0

// Verify BFloat16 operations emit correct SPIR-V opcodes.

// SPV-DAG: OpCapability BFloat16TypeKHR
// SPV-DAG: OpExtension "SPV_KHR_bfloat16"

// Arithmetic
// SPV-DAG: OpFAdd %bfloat16
// SPV-DAG: OpFSub %bfloat16
// SPV-DAG: OpFMul %bfloat16
// SPV-DAG: OpFDiv %bfloat16
// SPV-DAG: OpFNegate %bfloat16

// Comparisons
// SPV-DAG: OpFOrdEqual %bool
// SPV-DAG: OpFUnordNotEqual %bool
// SPV-DAG: OpFOrdLessThan %bool
// SPV-DAG: OpFOrdGreaterThan %bool
// SPV-DAG: OpFOrdLessThanEqual %bool
// SPV-DAG: OpFOrdGreaterThanEqual %bool

// Math functions (GLSL.std.450 extended instructions)
// SPV-DAG: OpExtInst %bfloat16 {{%[0-9]+}} FAbs
// SPV-DAG: OpExtInst %bfloat16 {{%[0-9]+}} FMin
// SPV-DAG: OpExtInst %bfloat16 {{%[0-9]+}} FMax
// SPV-DAG: OpExtInst %bfloat16 {{%[0-9]+}} FClamp

RWStructuredBuffer<int> outputBuffer;

void use(int v, int idx) { outputBuffer[idx] = v; }

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(float(outputBuffer[0]));
    BFloat16 b = BFloat16(float(outputBuffer[1]));

    // Arithmetic
    BFloat16 sum = a + b;
    BFloat16 diff = a - b;
    BFloat16 prod = a * b;
    BFloat16 quot = a / b;
    BFloat16 neg = -a;

    // Comparisons
    bool eq = (a == b);
    bool neq = (a != b);
    bool lt = (a < b);
    bool gt = (a > b);
    bool le = (a <= b);
    bool ge = (a >= b);

    // Math functions
    BFloat16 absVal = abs(a);
    BFloat16 minVal = min(a, b);
    BFloat16 maxVal = max(a, b);
    BFloat16 clamped = clamp(a, b, BFloat16(float(outputBuffer[2])));

    use(int(float(sum)), 0);
    use(int(float(diff)), 1);
    use(int(float(prod)), 2);
    use(int(float(quot)), 3);
    use(int(float(neg)), 4);
    use(eq ? 1 : 0, 5);
    use(neq ? 1 : 0, 6);
    use(lt ? 1 : 0, 7);
    use(gt ? 1 : 0, 8);
    use(le ? 1 : 0, 9);
    use(ge ? 1 : 0, 10);
    use(int(float(absVal)), 11);
    use(int(float(minVal)), 12);
    use(int(float(maxVal)), 13);
    use(int(float(clamped)), 14);
}
