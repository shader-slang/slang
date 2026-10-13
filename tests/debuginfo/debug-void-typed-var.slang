// Regression test for f3a61273b / GitHub #10061:
// The cleanUpVoidType IR pass could leave DebugVar/DebugValue instructions
// referencing void-typed values (e.g. an empty-struct parameter that was
// eliminated), causing a crash during SPIR-V emission.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

struct Empty {};

[__NoInline]
uint compute(uint x, Empty e)
{
    return x * 2u;   // line 12
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input, uniform RWStructuredBuffer<uint> output)
{
    Empty e;
    output[0] = compute(input[0], e);   // line 20
}

// SPVDB-CMD: descriptor 0 0 [7]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [14]
