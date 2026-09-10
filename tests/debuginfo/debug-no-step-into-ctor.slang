// Regression test for b9fd870d4 / GitHub #11550:
// Compiler-synthesized struct constructors were emitted with debug info,
// causing a single 'step' to descend into generated constructor code and
// report a bizarre (often zero) line number instead of the next user source line.
//
// After the fix, stepping over the struct initializer must land on the
// immediately following user statement.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

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
    output[0] = f.x + f.y;          // line 21 — step must land here, not in ctor
}

// SPVDB-CMD: descriptor 0 0 [5, 9]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break debug-no-step-into-ctor.slang 20
// SPVDB-CMD: run
// SPVDB-CMD: step
// SPVDB-CMD: location
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// If the bug regresses, 'step' enters the synthesized constructor and
// 'location' reports line 0 or an internal line, not line 21.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}debug-no-step-into-ctor.slang:21
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [14]
