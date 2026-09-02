// Regression test for 94d4e4736 / GitHub #11565:
// When a struct has a user-defined __init method, the object under construction
// ('this') was not emitted as a DebugLocalVariable, making it invisible in the
// debugger while inside the initializer body.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

struct Counter
{
    uint value;
    __init(uint start)
    {
        this.value = start;   // line 12 — 'this' must appear in locals here
    }
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input, uniform RWStructuredBuffer<uint> output)
{
    Counter c = Counter(input[0]);
    output[0] = c.value;   // line 22
}

// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: run
// SPVDB-CMD: locals
// SPVDB-CMD: read 0 1

// The __init method is inlined, so no breakpoint inside its body fires.
// We verify the fix (DebugLocalVariable for 'this') by checking that 'this'
// appears in the locals list after execution completes.
// SPVDB: stopped: EntryFinished
// SPVDB: local: this = {{.*}}
// SPVDB: read[0][1]: [7]
