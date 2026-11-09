// Tests the 'builtin' command: sets GlobalInvocationID before execution so a
// shader that uses SV_DispatchThreadID produces thread-specific output.
// The shader writes the thread index itself into the output buffer.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID,
          uniform RWStructuredBuffer<uint> output)
{
    uint idx = tid.x;          // line 11
    output[0] = idx * idx;     // line 12: square of the thread index
}

// SPVDB-CMD: descriptor 0 0 [0]
// SPVDB-CMD: builtin GlobalInvocationID 5 0 0
// SPVDB-CMD: break builtin-thread-id.slang 12
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 0

// With GlobalInvocationID = (5,0,0), tid.x = 5.  Break at line 12 so that idx
// (assigned on line 11) is already in the DebugValue stream.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}builtin-thread-id.slang:12
// SPVDB: local: idx = 5
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][0]: [25]
