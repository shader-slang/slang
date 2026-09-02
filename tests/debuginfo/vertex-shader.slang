// Tests 'inputloc' for vertex shader attribute inputs.
// Sets a float2 vertex attribute at location 0, breaks inside the entry point,
// and inspects the computed local variable derived from the input.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage vertex

struct VS_Out
{
    float4 pos : SV_Position;
};

[shader("vertex")]
VS_Out main(float2 inPos : POSITION0)
{
    float2 scaled = inPos * 2.0;   // line 14
    VS_Out o;
    o.pos = float4(scaled, 0.0, 1.0);
    return o;
}

// SPVDB-CMD: inputloc 0 [1.5, 2.5]
// SPVDB-CMD: break vertex-shader.slang 16
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run

// Break at line 16 so 'scaled' (computed on line 14) is in the DebugValue stream.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}vertex-shader.slang:16
// SPVDB: local: scaled = (3, 5)
// SPVDB: stopped: EntryFinished
