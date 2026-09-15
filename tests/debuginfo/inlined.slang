// Tests inlined function call-stack reconstruction via DebugInlinedAt.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

uint add_values(uint x, uint y) { return x + y; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> inputA,
          uniform RWStructuredBuffer<uint> inputB,
          uniform RWStructuredBuffer<uint> output)
{
    uint a = inputA[0];
    uint b = inputB[0];
    output[0] = add_values(a, b);
}

// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [5]
// SPVDB-CMD: descriptor 0 2 [0]
// SPVDB-CMD: break inlined.slang 4
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: backtrace
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 2

// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}inlined.slang:4
// SPVDB: frame[0]: add_values at {{.*}}inlined.slang:4
// SPVDB: frame[1]: main at {{.*}}inlined.slang:{{[0-9]+}}
// SPVDB: local: x = 7
// SPVDB: local: y = 5
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][2]: [12]
