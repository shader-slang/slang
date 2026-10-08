// Regression test for shader-slang/slang#11659: compiling a graphics-stage
// entry point to a CPU host target used to crash the compiler. The CPU varying
// legalizer only models compute system values, so a graphics varying falls to
// the "unsupported varying" diagnostic, which dereferenced `m_param`. The
// entry-point result is legalized before `m_param` is set, so that deref hit
// null. Each case below crashed on the unfixed compiler; a matched diagnostic
// proves the process ran to completion.

// Bare SV_Position/SV_Target result -> system-value path.
//TEST:SIMPLE(filecheck=GFX_VS): -target cpp -entry vertexMain
//TEST:SIMPLE(filecheck=GFX_VS): -target hpp -entry vertexMain
//TEST:SIMPLE(filecheck=GFX_VS): -target host-callable -entry vertexMain
//TEST:SIMPLE(filecheck=GFX_FS): -target cpp -entry fragmentMain

// Struct result (the original repro) -> user-varying path.
//TEST:SIMPLE(filecheck=GFX_USER): -target cpp -entry structOutMain

// Two entry points: the compute entry point sets and frees `m_param`, then the
// vertex entry point's result legalization must not read it. Exercises the
// per-entry-point reset in `processEntryPoint`.
//TEST:SIMPLE(filecheck=GFX_VS): -target cpp -entry computeMain -stage compute -entry vertexMain -stage vertex

// GFX_VS: this target doesn't support this system-defined varying parameter
// GFX_VS: vertexMain

// GFX_FS: this target doesn't support this system-defined varying parameter
// GFX_FS: fragmentMain

// GFX_USER: this target doesn't support this user-defined varying parameter
// GFX_USER: structOutMain

[shader("vertex")]
float4 vertexMain() : SV_Position
{
    return float4(0, 0, 0, 1);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(1, 0, 0, 1);
}

struct VertexOut
{
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOut structOutMain()
{
    VertexOut o;
    o.position = float4(0, 0, 0, 1);
    return o;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
}
