//TEST:SIMPLE(diag=CHK):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=FC):-target hlsl -stage compute -entry computeMain

// Test that diagnostics pointing to built-in modules (where source text is
// unavailable) don't render empty source sections.

Texture2D<int4> t2D;
SamplerState s;

RWStructuredBuffer<int4> outputBuffer;

void computeMain()
{
    outputBuffer[0] = t2D.Sample(s, float2(0.0, 0.0));
//CHK: static assertion failed
//CHK: HLSL supports only float and half type textures
//CHK: see call to 'Sample'
}

// FC: error[E41400]: static assertion failed
// FC-NEXT: --> hlsl.meta.slang:{{[0-9]+}}:{{[0-9]+}}
// FC-NEXT:  | static assertion failed, HLSL supports only float and half type textures
// FC-NEXT: -'
