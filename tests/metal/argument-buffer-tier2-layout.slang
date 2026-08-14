// argument-buffer-tier2-layout.slang

// Metal measures a parameter block's members in argument buffer element slots, so a
// consumer wanting byte offsets cannot get them from the element's own layout. The
// element's byte layout is attached to the parameter group layout for that purpose.
// Nothing in the compiler reads it yet, so the IR dump is the only place to observe it.
//
// Only Metal measures an element in argument buffer element slots, so nothing is attached
// on the targets checked negatively below.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry main -dump-ir -o /dev/null
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry main -dump-ir -o /dev/null
//TEST:SIMPLE(filecheck=VK): -target spirv -stage compute -entry main -dump-ir -o /dev/null
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry main -dump-ir -o /dev/null

struct PB
{
    float4 a;
    Texture2D t;
    float4 b;
}

ParameterBlock<PB> block;

RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = block.a + block.b;
}

// Under tier 2 the element occupies 48 bytes with the texture at byte 16, while the
// element's own layout reports 3 argument buffer element slots instead. Pinning the byte
// size catches the wrong ruleset being attached, which a check for a struct layout alone
// would not.
// METAL: parameterGroupTypeLayout
// METAL: metalArgumentBufferTier2Layout([[BYTELAYOUT:%[0-9]+]])
// METAL-DAG: [[BYTELAYOUT]]{{.*}}= structTypeLayout([[BYTESIZE:%[0-9]+]],
// METAL-DAG: [[BYTESIZE]]{{.*}}= size(8 : Int, 48 : Int)

// The positive check keeps each negative honest, since a run that produced no layout, or
// failed outright, would otherwise satisfy it.
// HLSL: parameterGroupTypeLayout
// HLSL-NOT: metalArgumentBufferTier2Layout

// VK: parameterGroupTypeLayout
// VK-NOT: metalArgumentBufferTier2Layout

// WGSL: parameterGroupTypeLayout
// WGSL-NOT: metalArgumentBufferTier2Layout
