// WaveActiveAllTrue / WaveActiveAnyTrue functional test.
//
// Wave-uniform reasoning primitives — given a per-thread predicate,
// reduce across the wave to a single boolean. WaveActiveAllTrue is
// the conjunction; WaveActiveAnyTrue is the disjunction. Both are
// wave-tangled and must operate over the active threads on the
// current convergent path.
//
// 4 threads, indices 0..3:
//   - Predicate "idx >= 0": all true → WaveActiveAllTrue = true,
//     WaveActiveAnyTrue = true.
//   - Predicate "idx > 5":  all false → WaveActiveAllTrue = false,
//     WaveActiveAnyTrue = false.
//   - Predicate "idx == 0": one true → WaveActiveAllTrue = false,
//     WaveActiveAnyTrue = true.
//   - Predicate "idx < 3":  three true / one false → AllTrue = false,
//     AnyTrue = true.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);

    // Each lane writes the same wave-reduced value, so any one thread's
    // observation is sufficient. Use idx 0 to write the eight values
    // (one per predicate × {All, Any}). Wave operations are still
    // wave-tangled; every thread participates in the reduction.
    bool allGE0 = WaveActiveAllTrue(idx >= 0);
    bool anyGE0 = WaveActiveAnyTrue(idx >= 0);
    bool allGT5 = WaveActiveAllTrue(idx > 5);
    bool anyGT5 = WaveActiveAnyTrue(idx > 5);
    bool allEQ0 = WaveActiveAllTrue(idx == 0);
    bool anyEQ0 = WaveActiveAnyTrue(idx == 0);
    bool allLT3 = WaveActiveAllTrue(idx < 3);
    bool anyLT3 = WaveActiveAnyTrue(idx < 3);

    if (idx == 0)
    {
        outputBuffer[0] = allGE0 ? 1 : 0;
        outputBuffer[1] = anyGE0 ? 1 : 0;
        outputBuffer[2] = allGT5 ? 1 : 0;
        outputBuffer[3] = anyGT5 ? 1 : 0;
        outputBuffer[4] = allEQ0 ? 1 : 0;
        outputBuffer[5] = anyEQ0 ? 1 : 0;
        outputBuffer[6] = allLT3 ? 1 : 0;
        outputBuffer[7] = anyLT3 ? 1 : 0;
    }
}

// CHECK: 1
// CHECK: 1
// CHECK: 0
// CHECK: 0
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 1
