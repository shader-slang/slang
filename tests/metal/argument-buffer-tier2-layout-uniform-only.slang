// argument-buffer-tier2-layout-uniform-only.slang

// A parameter block with no resource still gets the byte layout, so the attribute tracks
// how the element is measured, not whether the block holds a resource.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry main -dump-ir -o /dev/null

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
    outBuf[0] = block.a + block.b;
}

// METAL: parameterGroupTypeLayout
// METAL: metalArgumentBufferTier2Layout([[BYTELAYOUT:%[0-9]+]])
// METAL-DAG: [[BYTELAYOUT]]{{.*}}= structTypeLayout([[BYTESIZE:%[0-9]+]],
// METAL-DAG: [[BYTESIZE]]{{.*}}= size(8 : Int, 32 : Int)
