// Tests DebugValue emission for float scalar local variables.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<float> inputA,
          uniform RWStructuredBuffer<float> inputB,
          uniform RWStructuredBuffer<int>   output)
{
    float a = inputA[0];
    float b = inputB[0];
    output[0] = int(a + b);
}

// SPVDB-CMD: descriptor 0 0 [1.5]
// SPVDB-CMD: descriptor 0 1 [2.5]
// SPVDB-CMD: descriptor 0 2 [0]
// SPVDB-CMD: break float-locals.slang 11
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: step
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 2

// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}float-locals.slang:11
// SPVDB: local: a = 1.5
// SPVDB: stopped: Step
// SPVDB: location: {{.*}}float-locals.slang:12
// SPVDB: local: a = 1.5
// SPVDB: local: b = 2.5
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][2]: [4]
