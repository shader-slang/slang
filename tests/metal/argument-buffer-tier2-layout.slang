// argument-buffer-tier2-layout.slang

// Metal measures a parameter block's members in argument buffer element slots, so a
// consumer wanting byte offsets cannot get them from the element's own layout. The
// element's byte layout is attached to the parameter group layout for that purpose.
// Nothing in the compiler reads it yet, so the IR dump is the only place to observe it.
//
// D3D measures the element in bytes already, so nothing is attached there.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry main -dump-ir
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry main -dump-ir

struct PB
{
    float4 a;
    float4 b;
}

ParameterBlock<PB> block;

RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = block.b;
}

// METAL: parameterGroupTypeLayout
// METAL: metalArgumentBufferTier2Layout(
// METAL-NEXT: structTypeLayout(

// HLSL-NOT: metalArgumentBufferTier2Layout
