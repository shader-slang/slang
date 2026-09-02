// Tests that a real OpFunctionCall (not inlined) produces a correct two-level
// backtrace and that stepping over the call with 'next' returns to the caller.
// The [__NoInline] attribute prevents the compiler from inlining the helper.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[__NoInline]
uint multiply(uint x, uint y) { return x * y; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> inputA,
          uniform RWStructuredBuffer<uint> inputB,
          uniform RWStructuredBuffer<uint> output)
{
    uint a = inputA[0];
    uint b = inputB[0];
    uint result = multiply(a, b);   // line 17: call site
    output[0] = result;
}

// SPVDB-CMD: descriptor 0 0 [6]
// SPVDB-CMD: descriptor 0 1 [7]
// SPVDB-CMD: descriptor 0 2 [0]
// SPVDB-CMD: break non-inlined-call.slang 7
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: backtrace
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 2

// Breakpoint inside the callee: backtrace shows multiply then main.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}non-inlined-call.slang:7
// SPVDB: frame[0]: multiply at {{.*}}non-inlined-call.slang:7
// SPVDB: frame[1]: main at {{.*}}non-inlined-call.slang:{{[0-9]+}}
// SPVDB: local: x = 6
// SPVDB: local: y = 7
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][2]: [42]
