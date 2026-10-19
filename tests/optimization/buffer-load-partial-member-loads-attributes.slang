//TEST:SIMPLE(filecheck=COHERENT): -target spirv-asm -entry coherentMain -stage compute -capability vk_mem_model
//TEST:SIMPLE(filecheck=ALIGNED): -target spirv-asm -entry alignedMain -stage compute
//TEST:SIMPLE(filecheck=PLAIN): -target spirv-asm -entry plainMain -stage compute

// A load of a composite through a pointer whose value is only taken apart is replaced by loads of
// the members that are used (see buffer-load-partial-member-loads.slang). A load with an alignment
// or memory-scope attribute (`loadAligned`, `loadCoherent`) is not replaced, so the attribute is
// not lost: the load that reaches SPIR-V (the whole record, or the member when a later step
// narrows it) still carries `Aligned 16` and, for the coherent load,
// `MakePointerVisible|NonPrivatePointer` (#13732). Replaced by member loads here, it would be a
// plain `Aligned 4` load.

struct Record
{
    float4 first;
    float4 second;
    float other;
};

uniform Record* gRecord;
RWStructuredBuffer<float> outputBuffer;

// COHERENT: OpLoad %{{Record_natural|float}} %{{[0-9]+}} Aligned|MakePointerVisible|NonPrivatePointer 16
[shader("compute")]
[numthreads(1, 1, 1)]
void coherentMain()
{
    outputBuffer[0] = loadCoherent<16, MemoryScope::Device>(gRecord).other;
}

// ALIGNED: OpLoad %{{Record_natural|float}} %{{[0-9]+}} Aligned 16
[shader("compute")]
[numthreads(1, 1, 1)]
void alignedMain()
{
    outputBuffer[0] = loadAligned<16>(gRecord).other;
}

// Without an attribute, the used member is loaded with its own alignment.
// PLAIN: OpLoad %float %{{[0-9]+}} Aligned 4
[shader("compute")]
[numthreads(1, 1, 1)]
void plainMain()
{
    outputBuffer[0] = (*gRecord).other;
}
