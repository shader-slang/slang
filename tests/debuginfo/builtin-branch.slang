// Tests that different GlobalInvocationID values cause the shader to take
// different branches, verifiable by inspecting output.  This exercises the
// 'builtin' command's effect on control flow without needing two test runs.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID,
          uniform RWStructuredBuffer<uint> output)
{
    if (tid.x == 0u)
        output[0] = 111u;
    else
        output[0] = 222u;
}

// SPVDB-CMD: descriptor 0 0 [0]
// SPVDB-CMD: builtin GlobalInvocationID 1 0 0
// SPVDB-CMD: run
// SPVDB-CMD: read 0 0

// With tid.x=1 the else branch runs.
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][0]: [222]
