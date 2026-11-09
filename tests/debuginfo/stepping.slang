// Tests stepping through source lines and reading local variables via DebugValue.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> inputA,
          uniform RWStructuredBuffer<uint> inputB,
          uniform RWStructuredBuffer<uint> output)
{
    uint a = inputA[0];
    uint b = inputB[0];
    output[0] = a + b;
}

// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [5]
// SPVDB-CMD: descriptor 0 2 [0]
// SPVDB-CMD: break stepping.slang 11
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: step
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 2

// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}stepping.slang:11
// SPVDB: local: a = 7
// SPVDB: location: {{.*}}stepping.slang:12
// SPVDB: local: a = 7
// SPVDB: local: b = 5
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][2]: [12]
