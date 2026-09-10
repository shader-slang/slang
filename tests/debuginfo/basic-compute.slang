// Basic compute shader: tests run-to-finish and output value.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [5]
// SPVDB-CMD: descriptor 0 2 [0]
// SPVDB-CMD: run
// SPVDB-CMD: read 0 2

// SPVDB: stopped: EntryFinished
// SPVDB: read[0][2]: [12]

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
