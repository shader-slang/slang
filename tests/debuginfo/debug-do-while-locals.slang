// Regression test for a2a201e32 / GitHub #5404:
// In a do-while loop, a variable updated at the END of the loop body must
// show the UPDATED value at the START of subsequent iterations.
// Before the fix, DebugValue pointed to an SSA value that went out of scope
// after the loop back-edge, causing the debugger to show the initial value
// on iterations 2 and beyond.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input, uniform RWStructuredBuffer<uint> output)
{
    uint step = input[0];
    uint pos = 0u;
    uint iter = 0u;
    do
    {
        output[iter] = pos;   // line 18 — break here
        iter++;
        pos += step;          // update at END of body; must be visible next iteration
    } while (iter < 3u);
    output[3] = pos;
}

// SPVDB-CMD: descriptor 0 0 [3]
// SPVDB-CMD: descriptor 0 1 [0, 0, 0, 0]
// SPVDB-CMD: break debug-do-while-locals.slang 18
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// Iteration 1: pos=0 before any update, iter=0
// SPVDB: stopped: Breakpoint
// SPVDB: local: iter = 0
// SPVDB: local: pos = 0
// Iteration 2: pos must be 3 (updated at end of iter 1); before the fix it showed 0
// SPVDB: stopped: Breakpoint
// SPVDB: local: iter = 1
// SPVDB: local: pos = 3
// Iteration 3: pos must be 6
// SPVDB: stopped: Breakpoint
// SPVDB: local: iter = 2
// SPVDB: local: pos = 6
// Loop exits (iter==3 fails condition), entry finishes
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [0, 3, 6, 9]
