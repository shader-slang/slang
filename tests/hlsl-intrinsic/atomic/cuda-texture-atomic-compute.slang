// Runtime numeric check for the CUDA/PTX `sured` surface-reduction lowering of
// result-discarding `RWTexture` atomics (shader-slang/slang#12636). Compile
// coverage lives in `cuda-texture-atomic.slang`; this test proves the actual
// reduction semantics, the byte-addressed x coordinate, and the vector-texel
// channel offset by reading the affected texels back after the atomics.
//
// Each op runs at the *nonzero* coordinate (1, 2) so a dropped `* stride` term
// (which would leave x at the texel index rather than the byte address) or a
// missing channel offset would corrupt a different texel/channel than the one
// we assert, and we read neighboring texels/channels to confirm they are
// untouched.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type

// Scalar targets, initialized to a known content so the reduction result is
// predictable. `content = one` fills every texel with 1.
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content = one, mipMaps = 1):name texAdd
RWTexture2D<uint> texAdd;
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content = one, mipMaps = 1):name texMin
RWTexture2D<uint> texMin;
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content = one, mipMaps = 1):name texMax
RWTexture2D<uint> texMax;
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content = one, mipMaps = 1):name texAnd
RWTexture2D<uint> texAnd;
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content = one, mipMaps = 1):name texOr
RWTexture2D<uint> texOr;

// Vector texel: exercise the `.y` channel offset at a nonzero x. `content = one`
// fills all four channels with 1.
//TEST_INPUT: RWTexture2D(format=RGBA32Uint, size=4, content = one, mipMaps = 1):name texVec
RWTexture2D<uint4> texVec;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    uint2 c = uint2(1, 2);

    // add: 1 (initial) + 10 = 11
    InterlockedAdd(texAdd[c], 10u);
    // min: min(1, 5) = 1
    InterlockedMin(texMin[c], 5u);
    // max: max(1, 5) = 5
    InterlockedMax(texMax[c], 5u);
    // and: 1 & 3 = 1
    InterlockedAnd(texAnd[c], 3u);
    // or: 1 | 4 = 5
    InterlockedOr(texOr[c], 4u);

    // Vector-component: add 100 to the .y channel only. Initial .y = 1 → 101;
    // .x/.z/.w must stay 1.
    InterlockedAdd(texVec[c].y, 100u);

    // Read the affected texel of each scalar target.
    outputBuffer[0] = texAdd[c]; // 11
    outputBuffer[1] = texMin[c]; // 1
    outputBuffer[2] = texMax[c]; // 5
    outputBuffer[3] = texAnd[c]; // 1
    outputBuffer[4] = texOr[c];  // 5

    // A neighboring texel of texAdd must be unchanged (byte-addressing check):
    // if x were not scaled by the format stride, the add could have hit (0, 2)
    // instead of (1, 2).
    outputBuffer[5] = texAdd[uint2(0, 2)]; // 1 (untouched)
    outputBuffer[6] = texAdd[uint2(2, 2)]; // 1 (untouched)

    // Vector texel channels: only .y changed.
    uint4 v = texVec[c];
    outputBuffer[7] = v.x;  // 1
    outputBuffer[8] = v.y;  // 101
    outputBuffer[9] = v.z;  // 1
    outputBuffer[10] = v.w; // 1
    // A neighbor of the vector texel is fully untouched.
    outputBuffer[11] = texVec[uint2(0, 2)].y; // 1
}

// BUF: 11
// BUF-NEXT: 1
// BUF-NEXT: 5
// BUF-NEXT: 1
// BUF-NEXT: 5
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 101
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 1
