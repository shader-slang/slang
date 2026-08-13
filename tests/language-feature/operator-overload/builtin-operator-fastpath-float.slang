// Builtin-operator fast-path coverage for floating-point element types, float matrices,
// scalar<->vector / scalar<->matrix broadcast, and mixed-type promotion. All operands are
// runtime values (read from the buffer) so the ops are genuinely fast-pathed (not folded).
// Exercises branches not covered by the integer test: float arithmetic + FRem (`%`), float
// comparison, float matrix arithmetic, vector*scalar broadcast (SPIR-V OpVectorTimesScalar),
// matrix+scalar broadcast, and mixed-type promotion (int+float, half+float, int3+float3).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-d3d12 -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = outputBuffer[0] + 7.0; // 7.0
    float b = outputBuffer[1] + 2.0; // 2.0

    // Float scalar arithmetic + FRem + comparison.
    outputBuffer[0] = a + b;            // 9.0
    outputBuffer[1] = a - b;            // 5.0
    outputBuffer[2] = a * b;            // 14.0
    outputBuffer[3] = a / b;            // 3.5
    outputBuffer[4] = a % b;            // 1.0  (FRem)
    outputBuffer[5] = (a > b) ? 1.0 : 0.0; // 1.0
    outputBuffer[6] = -a;               // -7.0

    // Float vector arithmetic + vector*scalar broadcast.
    float4 va = float4(a, b, a + b, a - b); // (7,2,9,5)
    float4 vb = va * 2.0;                    // (14,4,18,10)  vector*scalar broadcast
    float4 vs = va + vb;                     // (21,6,27,15)
    outputBuffer[7] = vs.x;             // 21.0
    outputBuffer[8] = vs.w;             // 15.0
    bool4 cmp = va < vb;                // all true
    outputBuffer[9] = cmp.x ? 1.0 : 0.0; // 1.0

    // Float matrix arithmetic + matrix+scalar broadcast.
    float2x2 ma = float2x2(a, b, b, a); // {{7,2},{2,7}}
    float2x2 mb = ma + 1.0;             // {{8,3},{3,8}}  matrix+scalar broadcast
    float2x2 ms = ma + mb;              // {{15,5},{5,15}}
    outputBuffer[10] = mb[0][0];        // 8.0
    outputBuffer[11] = ms[1][1];        // 15.0

    // Mixed-type promotion.
    int i = int(outputBuffer[12]) + 3;  // 3
    outputBuffer[12] = i + a;           // int+float -> float = 10.0
    half h = half(outputBuffer[13]) + half(2.0); // 2.0
    outputBuffer[13] = h + a;           // half+float -> float = 9.0
    int3 i3 = int3(1, 2, 3);
    float3 v3 = float3(a, b, a);        // (7,2,7)
    float3 mv = v3 + i3;                // int3->float3, = (8,4,10)
    outputBuffer[14] = mv.x;            // 8.0
    outputBuffer[15] = mv.z;            // 10.0

    // CHECK: 9.0
    // CHECK: 5.0
    // CHECK: 14.0
    // CHECK: 3.5
    // CHECK: 1.0
    // CHECK: 1.0
    // CHECK: -7.0
    // CHECK: 21.0
    // CHECK: 15.0
    // CHECK: 1.0
    // CHECK: 8.0
    // CHECK: 15.0
    // CHECK: 10.0
    // CHECK: 9.0
    // CHECK: 8.0
    // CHECK: 10.0
}
