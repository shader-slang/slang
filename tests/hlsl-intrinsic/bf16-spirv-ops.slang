//TEST:SIMPLE(filecheck=SPV): -target spirv -emit-spirv-directly -O0 -skip-spirv-validation

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
// SPV-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Floor
// SPV-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Ceil
// SPV-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Sqrt

// Conversions
// SPV-DAG: OpFConvert %float %{{.*}}

// Vector arithmetic
// SPV-DAG: OpFAdd %v4bfloat16
// SPV-DAG: OpFSub %v4bfloat16
// SPV-DAG: OpFMul %v4bfloat16
// SPV-DAG: OpFDiv %v4bfloat16
// SPV-DAG: OpFNegate %v4bfloat16

// Vector comparisons
// SPV-DAG: OpFOrdEqual %v2bool
// SPV-DAG: OpFUnordNotEqual %v2bool
// SPV-DAG: OpFOrdLessThan %v2bool
// SPV-DAG: OpFOrdGreaterThan %v2bool
// SPV-DAG: OpFOrdLessThanEqual %v2bool
// SPV-DAG: OpFOrdGreaterThanEqual %v2bool

// Vector math functions
// SPV-DAG: OpExtInst %v2bfloat16 {{%[0-9]+}} FAbs
// SPV-DAG: OpExtInst %v2bfloat16 {{%[0-9]+}} FMin
// SPV-DAG: OpExtInst %v2bfloat16 {{%[0-9]+}} FMax
// SPV-DAG: OpExtInst %v2bfloat16 {{%[0-9]+}} FClamp

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
    BFloat16 floorVal = floor(a);
    BFloat16 ceilVal = ceil(a);
    BFloat16 sqrtVal = sqrt(a);

    // Conversion: BFloat16 -> float
    float converted = float(a);

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
    use(int(float(floorVal)), 15);
    use(int(float(ceilVal)), 16);
    use(int(float(sqrtVal)), 17);
    use(int(converted), 18);

    // Vector4 arithmetic
    vector<BFloat16, 4> v4a = vector<BFloat16, 4>(a, b, a, b);
    vector<BFloat16, 4> v4b = vector<BFloat16, 4>(b, a, b, a);

    let vsum = v4a + v4b;
    let vdiff = v4a - v4b;
    let vprod = v4a * v4b;
    let vquot = v4a / v4b;
    let vneg = -v4a;

    use(int(float(vsum[0])), 19);
    use(int(float(vdiff[1])), 20);
    use(int(float(vprod[2])), 21);
    use(int(float(vquot[3])), 22);
    use(int(float(vneg[3])), 23);

    // Vector2 comparisons
    vector<BFloat16, 2> v2a = vector<BFloat16, 2>(a, b);
    vector<BFloat16, 2> v2b = vector<BFloat16, 2>(b, a);

    let veq = (v2a == v2b);
    let vneq = (v2a != v2b);
    let vlt = (v2a < v2b);
    let vgt = (v2a > v2b);
    let vle = (v2a <= v2b);
    let vge = (v2a >= v2b);

    use(int(veq[0]) + int(vneq[0]) + int(vlt[0]) + int(vgt[0]) + int(vle[0]) + int(vge[0]), 24);

    // Vector2 math functions
    vector<BFloat16, 2> v2neg = vector<BFloat16, 2>(BFloat16(-float(a)), BFloat16(-float(b)));
    let vabs = abs(v2neg);
    let vmin = min(v2a, v2b);
    let vmax = max(v2a, v2b);
    let vclamped = clamp(v2a, v2b, vector<BFloat16, 2>(BFloat16(float(outputBuffer[2])), BFloat16(float(outputBuffer[2]))));

    use(int(float(vabs[0])), 25);
    use(int(float(vmin[1])), 26);
    use(int(float(vmax[0])), 27);
    use(int(float(vclamped[1])), 28);
}
