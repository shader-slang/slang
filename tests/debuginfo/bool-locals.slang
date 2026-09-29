// Tests DebugValue emission for bool local variables.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input,
          uniform RWStructuredBuffer<uint> output)
{
    uint val = input[0];
    bool flag = (val > 3u);
    output[0] = flag ? 1u : 0u;
}

// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break bool-locals.slang 10
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: step
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}bool-locals.slang:10
// SPVDB: local: val = 7
// SPVDB: stopped: Step
// SPVDB: location: {{.*}}bool-locals.slang:11
// SPVDB: local: flag = true
// SPVDB: local: val = 7
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [1]
