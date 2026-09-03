// Tests 'inputloc' for fragment shader interpolated inputs, and 'outputs' to
// inspect the written fragment color.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage fragment

struct PS_Out
{
    float4 col : SV_Target;
};

[shader("fragment")]
PS_Out main(float2 uv : TEXCOORD0)
{
    float lum = uv.x + uv.y;   // line 13
    PS_Out o;
    o.col = float4(lum, lum, lum, 1.0);
    return o;
}

// SPVDB-CMD: inputloc 0 [0.25, 0.75]
// SPVDB-CMD: break fragment-shader.slang 15
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: outputs

// Break at line 15 so 'lum' (computed on line 13) is in the DebugValue stream.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}fragment-shader.slang:15
// SPVDB: local: lum = 1
// SPVDB: stopped: EntryFinished
// SPVDB: output: {{.*}} = (1, 1, 1, 1)
