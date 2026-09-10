// Tests that function parameters appear in locals at function entry via
// DebugLocalVariable with arg_index.  Uses a non-inlined function with
// three parameters so all three must be visible before any body locals.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[__NoInline]
uint clamp_add(uint a, uint b, uint limit)
{
    uint sum = a + b;          // line 9
    return (sum < limit) ? sum : limit;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> inputA,
          uniform RWStructuredBuffer<uint> inputB,
          uniform RWStructuredBuffer<uint> inputC,
          uniform RWStructuredBuffer<uint> output)
{
    output[0] = clamp_add(inputA[0], inputB[0], inputC[0]);
}

// SPVDB-CMD: descriptor 0 0 [5]
// SPVDB-CMD: descriptor 0 1 [8]
// SPVDB-CMD: descriptor 0 2 [10]
// SPVDB-CMD: descriptor 0 3 [0]
// SPVDB-CMD: break function-parameters.slang 9
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 3

// At the first line of the callee, all three parameters are already visible.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}function-parameters.slang:9
// SPVDB: local: a = 5
// SPVDB: local: b = 8
// SPVDB: local: limit = 10
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][3]: [10]
