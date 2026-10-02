// A source line spanning several blocks counts one region visit. Suppressing
// another visit must suppress the atomic itself, not add zero to a hot slot.
//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -trace-coverage -trace-branch-coverage -trace-coverage-counter-width 32
//TEST:SIMPLE(filecheck=BOOL):-target cpp -entry computeMain -trace-coverage -trace-coverage-boolean -trace-coverage-counter-width 32
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry computeMain -trace-coverage -trace-branch-coverage -trace-coverage-counter-width 32

RWStructuredBuffer<uint> outputBuffer;
uint choose(uint t) { if (t == 0) return 5; return 7; }
[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = choose(tid.x);
    for (uint i = 0; i < tid.x; ++i) { if (i == 1) continue; value += i; }
    for (uint i = 0; i < 2; ++i)
    {
        for (uint j = 0; j < 3; ++j)
        {
            if (j == 1) break;
            value += 10;
        }
    }
    outputBuffer[tid.x] = value;
}

//CHECK-NOT: _slang_atomic_add_u32{{.*}}, 0U)
//CHECK: _slang_atomic_add_u32{{.*}}, 1U)
//CHECK-NOT: _slang_atomic_add_u32{{.*}}, 0U)

//SPIRV: OpLoopMerge

// The first line counter belongs to choose(). Its condition dominates both
// returns, so boolean recording must emit only one store to that counter.
//BOOL: _slang_coverage{{.*}}[int(0)]) = 1U;
//BOOL-NOT: _slang_coverage{{.*}}[int(0)]) = 1U;
