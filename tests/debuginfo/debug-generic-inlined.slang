// Regression test for d1fbce396 / GitHub #8978:
// Generic functions that were specialized and inlined caused a null dereference
// crash in emitCalleeDebugInlinedAt because IRDebugLocationDecoration was
// absent on the call site.  The test verifies compilation succeeds and the
// shader executes correctly.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

T doubled<T : __BuiltinIntegerType>(T x) { return x + x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input, uniform RWStructuredBuffer<uint> output)
{
    uint a = doubled<uint>(input[0]);   // line 14 — specialized + inlined generic
    output[0] = a;                       // line 15
}

// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [14]
