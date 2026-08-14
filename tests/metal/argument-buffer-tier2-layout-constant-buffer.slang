// argument-buffer-tier2-layout-constant-buffer.slang

// A constant buffer's element is not measured in argument buffer element slots, so no byte
// layout is attached. Both this file and argument-buffer-tier2-layout.slang compile for
// Metal, so a rule keyed on the target rather than on the element's layout would pass there
// and fail here.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry main -dump-ir -o /dev/null

struct PB
{
    float4 a;
    float4 b;
}

ConstantBuffer<PB> cb;

RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = cb.b;
}

// METAL: parameterGroupTypeLayout
// METAL-NOT: metalArgumentBufferTier2Layout
