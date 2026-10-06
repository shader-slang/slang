// Regression test for ab001e556 / GitHub #10541:
// Under -O2 the struct backing store is optimized away, but Slang now emits
// per-member DebugValue instructions so the struct remains visible in locals.
// Without the fix the struct was completely absent from 'locals' under -O2.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute -O2

struct Pair
{
    uint a;
    uint b;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input, uniform RWStructuredBuffer<uint> output)
{
    Pair s;
    s.a = input[0];            // line 18
    s.b = input[1];            // line 19
    output[0] = s.a + s.b;    // line 20 — break here, both members must be visible
}

// SPVDB-CMD: descriptor 0 0 [3, 7]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break debug-struct-opt.slang 20
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// Under -O2 the struct is tracked via per-member DebugValue (SSA path), not a
// memory-backed variable.  Before the fix, 's' would not appear in locals at all.
// DebugValue partial updates now correctly accumulate: a=3, b=7 → (3, 7).
// SPVDB: local: s = (3, 7)
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [10]
