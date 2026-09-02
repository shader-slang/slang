// Tests that a source breakpoint inside a for-loop fires once per iteration
// and that the loop variable and accumulator reflect the correct values at
// each iteration boundary.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input,
          uniform RWStructuredBuffer<uint> output)
{
    uint base = input[0];
    uint sum = 0u;
    for (uint i = 0u; i < 3u; i++)
    {
        sum += base;   // line 15
    }
    output[0] = sum;
}

// SPVDB-CMD: descriptor 0 0 [4]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break loop-stepping.slang 15
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// First iteration breakpoint: sum not yet accumulated, i=0
// SPVDB: stopped: Breakpoint
// SPVDB: local: base = 4
// SPVDB: local: i = 0
// SPVDB: local: sum = 0
// Second iteration: sum accumulated once
// SPVDB: stopped: Breakpoint
// SPVDB: local: base = 4
// SPVDB: local: i = 1
// SPVDB: local: sum = 4
// Third iteration: sum accumulated twice
// SPVDB: stopped: Breakpoint
// SPVDB: local: base = 4
// SPVDB: local: i = 2
// SPVDB: local: sum = 8
// Loop exits, entry finishes
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [12]
