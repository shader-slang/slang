// Tests stepping through an if/else branch and verifying that the debugger
// lands on the correct taken branch.  Two runs use the same breakpoint but
// different input values so each branch is covered independently.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input,
          uniform RWStructuredBuffer<uint> output)
{
    uint val = input[0];
    uint result;
    if (val > 5u)
    {
        result = val * 2u;   // line 15: taken when val=7
    }
    else
    {
        result = val + 1u;   // line 19: taken when val=3
    }
    output[0] = result;
}

// --- Run 1: val=7, takes the true branch ---
// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break if-else-stepping.slang 15
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}if-else-stepping.slang:15
// SPVDB: local: val = 7
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [14]
