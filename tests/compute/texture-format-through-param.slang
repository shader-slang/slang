// shader-slang/slang#12737: on CUDA a writable texture's `[format]` backing size must be
// recovered when the texture is accessed through a function parameter, not just directly.
// `r16f` is a 16-bit backing format accessed as 32-bit `float`, so the size mismatch forces
// the converting surface intrinsics; an unrecovered format instead emits the plain
// `surf2Dwrite`/`surf2Dread` scaled by the 4-byte element size. The converting write is
// coordinate-unscaled (`* 1`) because `surf2Dwrite_convert` handles the packing itself, whereas
// the converting read (`surf2Dread_convert`) delegates to a byte-addressed load and so scales by
// the 2-byte backing size (`* 2`) — the two multipliers differ for the same format for that reason.
// `r16f`/`float` is used (rather than an integer format) because the CUDA prelude only defines the
// converting-read helper for float element types.

//TEST:SIMPLE(filecheck=CUDA,READ,NEG): -target cuda -entry computeMain -stage compute

// Both writes (through the parameter and direct) recover the format and convert with `* 1`.
//CUDA-COUNT-2: surf2Dwrite_convert<{{.*}}>({{.*}}, {{.*}}, ({{.*}}).x * 1, {{.*}})

// The read through the parameter recovers the format and converts with the 2-byte stride.
// (Separate prefix: the read interleaves between the two writes, and FileCheck matches in
// order within a prefix.)
//READ: surf2Dread_convert<{{.*}}>({{.*}}, ({{.*}}).x * 2, {{.*}})

// The plain, non-converting call forms appear only at an access site, never in the prelude
// declaration, so their presence means an access failed to recover `[format]`. A dedicated
// prefix makes this a whole-input check rather than a trailing `-NOT` that only scans past
// the last positive match. The trailing `<` is load-bearing: it matches the plain generic form
// `surf2Dwrite<...>` but NOT the required `surf2Dwrite_convert<...>` (no `<` right after the
// name) — dropping it would wrongly reject the converting calls the test asserts above.
//NEG-NOT: surf2Dwrite<
//NEG-NOT: surf2Dread<

void writeThroughParam(RWTexture2D<float> t)
{
    t[uint2(1, 0)] = 5.0f;
}

float readThroughParam(RWTexture2D<float> t)
{
    return t[uint2(1, 0)];
}

[format("r16f")]
RWTexture2D<float> tex;

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    writeThroughParam(tex);
    outBuf[0] = readThroughParam(tex);
    tex[uint2(2, 0)] = 7.0f;
}
