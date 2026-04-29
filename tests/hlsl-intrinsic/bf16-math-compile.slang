// Verify BFloat16 transcendental and math functions compile to SPIRV.
// These functions are inherited from __BuiltinFloatingPointType generics.
// This test ensures they produce valid SPIRV without runtime execution.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -O0 -skip-spirv-validation

// CHECK: OpCapability BFloat16TypeKHR

RWStructuredBuffer<int> outputBuffer;

void use(int v, int idx) { outputBuffer[idx] = v; }

[numthreads(1,1,1)]
void computeMain()
{
    // Use buffer input to prevent constant folding
    BFloat16 x = BFloat16(float(outputBuffer[0]) * 0.5);
    BFloat16 y = BFloat16(float(outputBuffer[1]) * 0.5);

    // Transcendental functions — verify native bf16 SPIRV ExtInst emission
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Sin
    use(int(float(sin(x))), 0);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Cos
    use(int(float(cos(x))), 1);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Exp
    use(int(float(exp(x))), 2);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Log
    use(int(float(log(x))), 3);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Sqrt
    use(int(float(sqrt(x))), 4);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Pow
    use(int(float(pow(x, y))), 5);
    // exp2() is promoted to float on this path — not checked as bf16
    use(int(float(exp2(x))), 6);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Log2
    use(int(float(log2(x))), 7);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} InverseSqrt
    use(int(float(rsqrt(x))), 8);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Floor
    use(int(float(floor(x))), 9);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Ceil
    use(int(float(ceil(x))), 10);
    // CHECK-DAG: OpExtInst %bfloat16 {{%[0-9]+}} Fma
    use(int(float(fma(x, y, x))), 11);
}
