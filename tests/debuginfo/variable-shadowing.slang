// Tests that an inner declaration shadows an outer one: after stepping into the
// inner block, locals should show the inner 'x', not the outer one.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input,
          uniform RWStructuredBuffer<uint> output)
{
    uint x = input[0];          // outer x = 10
    {
        uint x = input[0] * 3u; // inner x = 30 (shadows outer)
        output[0] = x;          // line 13: uses inner x
    }
}

// SPVDB-CMD: descriptor 0 0 [10]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break variable-shadowing.slang 13
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// At line 13 the inner x=30 is in scope and shadows the outer x=10.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}variable-shadowing.slang:13
// SPVDB: local: x = 30
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [30]
