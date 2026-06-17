// Verifies boolean coverage mode (`-trace-coverage-boolean`, #11509).
//
// In boolean mode each coverage counter is written with a plain non-atomic
// store of 1 (CoverageCounterMode::Boolean) instead of an atomic add, so it
// records whether an entry executed (0 / non-zero) rather than an exact count.
// This removes all atomic contention. Off by default — the default count mode
// still uses atomic adds (asserted by the COUNT run below).

// Default (count) mode: coverage uses atomic adds.
//TEST:SIMPLE(filecheck=COUNT):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-branch-coverage

// Boolean mode: 32-bit counters, no atomics.
// Pin counter width to 32-bit so the stored constant is %uint_1 (not %ulong_1
// which the 64-bit default would produce), making the HITMISS check below exact.
//TEST:SIMPLE(filecheck=HITMISS):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-branch-coverage -trace-coverage-boolean -trace-coverage-counter-width 32

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Use output values 2 and 3 (not 1) so the only OpStore of the constant 1
    // in the SPIR-V is the coverage counter write, making the HITMISS check
    // below unambiguous.
    if ((tid.x & 1u) == 0u)
        outputBuffer[tid.x] = 2;
    else
        outputBuffer[tid.x] = 3;
}

// Count mode increments counters atomically.
//COUNT: OpAtomicIAdd

// Boolean mode emits a plain store of 1 (not an atomic add). The coverage
// variable is named __slang_coverage; checking OpName ensures the variable
// is present and OpStore %uint_1 is unambiguous because the shader writes
// 2 and 3 to outputBuffer, not 1.
//HITMISS: OpName {{.*}} "__slang_coverage"
//HITMISS: OpStore {{.*}} %uint_1
//HITMISS-NOT: OpAtomicIAdd
