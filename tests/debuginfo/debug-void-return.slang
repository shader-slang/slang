// Regression test for 1cb3004b5 / #9237 and f72a2a618 / #9749:
// Void-returning functions produced an invalid DebugFunction (wrong/empty
// return type), causing a crash during SPIR-V emission.  The test verifies
// compilation succeeds, a backtrace inside the void function is valid, and
// execution completes without panicking.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[__NoInline]
void increment(RWStructuredBuffer<uint> buf)
{
    buf[0] += 1u;   // line 11
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> output)
{
    output[0] = 0u;
    increment(output);   // line 19
    increment(output);   // line 20
}

// SPVDB-CMD: descriptor 0 0 [0]
// SPVDB-CMD: break debug-void-return.slang 11
// SPVDB-CMD: run
// SPVDB-CMD: backtrace
// SPVDB-CMD: run
// SPVDB-CMD: run
// SPVDB-CMD: read 0 0

// The shader calls increment(output) twice, so the breakpoint fires twice.
// SPVDB: stopped: Breakpoint
// SPVDB: frame[0]: increment
// SPVDB: stopped: Breakpoint
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][0]: [2]
