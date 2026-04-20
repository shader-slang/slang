// Verifies that -trace-coverage on a SPIR-V target emits real
// OpAtomicIAdd instructions on the synthesized counter buffer. This
// is the check that shader coverage actually lowers to native atomics
// on GPU targets (not just the stopgap used during earlier prototypes).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = 0;
    for (uint i = 0; i < 4; ++i)
    {
        if ((i & 1u) == 0u)
            accum += i;
        else
            accum += i * 2u;
    }
    outputBuffer[0] = accum;
}

// At least one OpAtomicIAdd must appear in the output — this is the
// native SPIR-V lowering of kIROp_AtomicAdd for our counter increment.
//CHECK: OpAtomicIAdd

// The synthesized coverage buffer should also show up in the module's
// debug name table so that a host can discover it via reflection.
//CHECK: __slang_coverage
