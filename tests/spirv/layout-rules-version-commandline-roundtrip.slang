// With `-g2`, SPIR-V debug info embeds the command line that `writeCommandLineArgs` rebuilds from
// the compile options. `-layout-rules-version` changes buffer layouts, so it has to survive that
// round trip for a tool to rebuild the shader with the same layout.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -g2 -fvk-use-scalar-layout -layout-rules-version 202c

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = (float)tid.x;
}

// CHECK: -layout-rules-version 202c
