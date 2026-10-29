// Regression test for 40ae503b6 / GitHub #9371:
// An if-statement with no else branch left the successor SPIR-V block without
// a DebugLine, so stepping over a skipped (false) if would report line 0.
// The same bug affected short-circuit && / || expressions.  This test covers
// both: a bare if (condition false, body skipped) followed by a && expression.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input, uniform RWStructuredBuffer<uint> output)
{
    uint v = input[0];                           // line 12
    if (v > 100u)                                // line 13 — no else; false with input=1
        v = 999u;                                // line 14 — never taken
    uint ok = (v > 0u && v < 50u) ? 1u : 0u;   // line 15 — step must land here, not :0
    output[0] = ok;                              // line 16
}

// SPVDB-CMD: descriptor 0 0 [1]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break debug-no-else-if.slang 13
// SPVDB-CMD: run
// SPVDB-CMD: step
// SPVDB-CMD: location
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// After stepping past the skipped if-body the debugger must land on line 15
// (the conditional expression), not report :0.
// SPVDB: stopped: Breakpoint
// SPVDB: stopped: Step
// SPVDB: location: {{.*}}debug-no-else-if.slang:15
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [1]
