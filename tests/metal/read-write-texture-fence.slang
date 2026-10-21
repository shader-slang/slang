// MSL orders a thread's own accesses to an `access::read_write` texture only across a `fence()`
// (MSL 4.1 section 6.13.17). Every write to such a texture, including the ones Slang creates when
// it lowers a partial texel store, has a fence before it (so it cannot pass an earlier read) and
// after it (so later reads see it). Write-only textures do not declare `fence()` and get none.
// (shader-slang/slang#13771)

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=ARRAY): -target metal -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=LOOP): -target metal -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=WAR): -target metal -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=ROV): -target metal -stage fragment -entry fragMain -line-directive-mode none
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB_ROV): -target metallib -stage fragment -entry fragMain

// METALLIB: define void @computeMain
// METALLIB_ROV: define {{.*}} @fragMain

RWTexture2D<float4> img;
RWTexture2DArray<float> layers;
RWBuffer<float> texels;
RWBuffer<uint2> pairs;
RWTexture2D<uint> counters;
RWTexture2D<float4> channels;
RWTexture2D<float4> tiles[4];
RWTexture2D<uint> flags;
WTexture2D<float4> writeOnly;
Texture2D<float4> readOnly;
RWStructuredBuffer<float4> outputBuffer;

struct Targets
{
    RWTexture2D<float4> color;
}
ParameterBlock<Targets> targets;

// A write in a callee is fenced in the callee, so a read in the caller sees it.
// CHECK-LABEL: void writeLayer
// CHECK: _slang_textureFence({{.*}}layers
// CHECK-NEXT: layers{{.*}}.write(
// CHECK-NEXT: _slang_textureFence({{.*}}layers
[noinline]
void writeLayer(uint2 p)
{
    layers[uint3(p, 1)] = 2.0;
}

// Each iteration reads the texel the previous iteration wrote, so the fence stays in the loop
// body around the write rather than moving before the loop or after it.
// LOOP-LABEL: void accumulate
// LOOP: for
// LOOP: img{{.*}}.read(
// LOOP: _slang_textureFence({{.*}}img
// LOOP-NEXT: img{{.*}}.write(
// LOOP-NEXT: _slang_textureFence({{.*}}img
// LOOP-NEXT: i_{{[0-9]+}} = i_{{[0-9]+}} + 1U;
// LOOP-NEXT: }
[noinline]
void accumulate(uint2 p, uint count)
{
    for (uint i = 0; i < count; i++)
        img[p] = img[p] + float(i);
}

// The fence on an element of a texture array names the same element as the write, so the array
// of handles stays in the kernel context instead of being copied into a local.
// ARRAY-LABEL: void writeTile
// ARRAY-NOT: array<texture2d
// ARRAY: _slang_textureFence({{.*}}->tiles_{{[0-9]+}}[i_{{[0-9]+}}])
// ARRAY-NEXT: ->tiles_{{[0-9]+}}[i_{{[0-9]+}}].write(
// ARRAY-NEXT: _slang_textureFence({{.*}}->tiles_{{[0-9]+}}[i_{{[0-9]+}}])
[noinline]
void writeTile(uint i, uint2 p)
{
    tiles[i][p] = float4(6.0);
}

// Write-after-read on one texel: without a fence between them, Metal may move the write ahead of
// the read, and `seen` could then come back as 42. Dawn hit this on Apple M1 (crbug.com/417296309).
// WAR-LABEL: uint markSeen
// WAR: flags{{.*}}.read(
// WAR: _slang_textureFence({{.*}}flags
// WAR-NEXT: flags{{.*}}.write(
// WAR-NEXT: _slang_textureFence({{.*}}flags
[noinline]
uint markSeen(uint2 p)
{
    uint seen = flags[p];
    flags[p] = 42;
    return seen;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    uint2 p = id.xy;

    accumulate(p, id.z);
    writeTile(id.z & 3, p);
    uint seen = markSeen(p);

    // The read-modify-write from the issue: the second read must see the write.
    // CHECK-LABEL: void computeMain
    // CHECK: img{{.*}}.read(
    // CHECK: _slang_textureFence({{.*}}img
    // CHECK-NEXT: img{{.*}}.write(
    // CHECK-NEXT: _slang_textureFence({{.*}}img
    img[p] = img[p] * 0.5 + 1.0;
    outputBuffer[p.y * 64 + p.x] = img[p];

    writeLayer(p);

    // `RWBuffer` lowers to `texture_buffer<T, access::read_write>`, whose `write` takes a
    // 4-component value (MSL 4.1 section 6.13.16), so the stored value is widened.
    // CHECK: [[TEXEL:_S[0-9]+]] = float4(3.0f,
    // CHECK-NEXT: _slang_textureFence({{.*}}texels
    // CHECK-NEXT: texels{{.*}}.write([[TEXEL]],
    // CHECK-NEXT: _slang_textureFence({{.*}}texels
    texels[p.x] = 3.0;
    // CHECK: [[PAIR:_S[0-9]+]] = uint4(1U, 2U,
    // CHECK-NEXT: _slang_textureFence({{.*}}pairs
    // CHECK-NEXT: pairs{{.*}}.write([[PAIR]],
    pairs[p.x] = uint2(1, 2);

    // A later ordinary texture read must see the atomic write.
    // CHECK: _slang_textureFence({{.*}}counters
    // CHECK-NEXT: counters{{.*}}.atomic_fetch_add(
    // CHECK-NEXT: _slang_textureFence({{.*}}counters
    uint previous;
    InterlockedAdd(counters[p], 1, previous);

    // A compare-exchange is emitted as a block, and the fences surround the block.
    // CHECK: _slang_textureFence({{.*}}counters
    // CHECK: counters{{.*}}.atomic_compare_exchange_weak(
    // CHECK: }
    // CHECK-NEXT: _slang_textureFence({{.*}}counters
    InterlockedCompareExchange(counters[p], 1, 2, previous);

    // Storing one channel becomes a read, a swizzle and a write of the whole texel.
    // CHECK: channels{{.*}}.read(
    // CHECK: _slang_textureFence({{.*}}channels
    // CHECK-NEXT: channels{{.*}}.write(
    // CHECK-NEXT: _slang_textureFence({{.*}}channels
    channels[p].x = 1.0;

    // A texture in a parameter block is reached through a `constant` pointer, so the fence
    // takes the texture by value rather than calling `fence()` on the const member.
    // CHECK: [[COLOR:_S[0-9]+]] = {{.*}}->color
    // CHECK-NEXT: _slang_textureFence([[COLOR]])
    // CHECK-NEXT: [[COLOR]].write(
    // CHECK-NEXT: _slang_textureFence([[COLOR]])
    targets.color[p] = float4(4.0);

    // A write-only texture has no `fence()`, and nothing else is written from here to the end of
    // the kernel, so no fence of any texture may appear between these two lines.
    // CHECK: writeOnly{{.*}}.write(
    // CHECK-NOT: _slang_textureFence
    // CHECK: readOnly{{.*}}.read(
    writeOnly.Store(p, float4(5.0));

    outputBuffer[0] = float4(layers[uint3(p, 1)], texels[p.x], float(counters[p]), channels[p].y)
                      + targets.color[p] + float(seen) + readOnly.Load(int3(p, 0));
}

// Rasterizer-ordered textures are `access::read_write` too.
// ROV: _slang_textureFence({{.*}}rov
// ROV-NEXT: rov{{.*}}.write(
// ROV-NEXT: _slang_textureFence({{.*}}rov
RasterizerOrderedTexture2D<float4> rov;

[shader("fragment")]
float4 fragMain(float4 position: SV_Position) : SV_Target
{
    uint2 p = uint2(position.xy);
    rov[p] = rov[p] + 1.0;
    return rov[p];
}
