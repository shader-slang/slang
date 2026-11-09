// Tests that a breakpoint inside a while-loop fires for each iteration and
// that stepping out of the loop body correctly advances to the output line.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input,
          uniform RWStructuredBuffer<uint> output)
{
    uint n = input[0];
    uint acc = 1u;
    while (n > 0u)
    {
        acc *= 2u;   // line 14
        n--;
    }
    output[0] = acc;
}

// SPVDB-CMD: descriptor 0 0 [3]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break while-loop.slang 14
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// First iteration: acc=1 before doubling, n=3
// SPVDB: stopped: Breakpoint
// SPVDB: local: acc = 1
// SPVDB: local: n = 3
// Second iteration: acc=2, n=2
// SPVDB: stopped: Breakpoint
// SPVDB: local: acc = 2
// SPVDB: local: n = 2
// Third iteration: acc=4, n=1
// SPVDB: stopped: Breakpoint
// SPVDB: local: acc = 4
// SPVDB: local: n = 1
// Loop exits (n=0 after last n--), entry finishes
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [8]
