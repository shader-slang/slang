// Check that generated constructors retain source locations and can be stepped into.
// FlagArtificial supplies provenance; the debugger decides whether to skip it.
// This test uses spvdb's ordinary step-in behavior with no artificial filtering.
//
// Constructor entry has a location on the struct declaration.
// Stepping into it must retain that location and execute the same shader.
// User-facing debuggers may choose a different policy.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute -O0

struct Foo
{
    uint x;
    uint y;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input, uniform RWStructuredBuffer<uint> output)
{
    Foo f = {input[0], input[1]};   // line 20 — synthesized init; break here
    output[0] = f.x + f.y;          // line 21 — ordinary caller statement
}

// SPVDB-CMD: descriptor 0 0 [5, 9]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break debug-step-into-ctor.slang 20
// SPVDB-CMD: run
// SPVDB-CMD: step
// SPVDB-CMD: location
// SPVDB-CMD: run
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// The first synthesized location is the struct declaration.
// Source stepping remains available even though the function is artificial.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}debug-step-into-ctor.slang:10
// Returning encounters the call-site breakpoint again; continue to finish.
// SPVDB: stopped: Breakpoint
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [14]
