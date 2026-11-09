// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.compute.grid;
import slang.compute.buffer_utils;

public static const uint kTextureInitThreadsX = 16;
public static const uint kTextureInitThreadsY = 16;

public extension<T : ITexelElement> RWTexture2D<T>
{
    public void fill(T value)
    {
        uint width;
        uint height;
        this.GetDimensions(width, height);

        Grid(uint2(width, height))
            .dispatch<kTextureInitThreadsX, kTextureInitThreadsY>(
                (uint2 pixel) =>
                { this[pixel] = value; });
    }
}

public static const uint kTextureMapThreadsX = 8;
public static const uint kTextureMapThreadsY = 8;
public static const uint kCooperativeTextureMapSubgroupSize = 32;
public static const uint kCooperativeTextureMapThreadsY = 8;

public uint3 cooperativeTextureMapGroupCount<let subgroupCount : int>(uint2 dimensions)
{
    static_assert(subgroupCount > 0, "A cooperative texture map needs at least one subgroup.");
    static_assert(
        subgroupCount <= 32,
        "A cooperative texture map cannot exceed 1024 workgroup invocations.");
    static const uint threadsX =
        kCooperativeTextureMapSubgroupSize * subgroupCount / kCooperativeTextureMapThreadsY;
    uint groupCountX = dimensions.x == 0 ? 1 : (dimensions.x + (threadsX - 1)) / threadsX;
    uint groupCountY = dimensions.y == 0 ? 1
                                         : (dimensions.y + (kCooperativeTextureMapThreadsY - 1)) /
                                               kCooperativeTextureMapThreadsY;
    return uint3(groupCountX, groupCountY, 1);
}

/// Invokes a transform coherently across the requested number of 32-lane subgroups.
///
/// Padding lanes still invoke `transform` with `isValid` set to false, but do not access or write
/// the texture. This lets transforms safely use subgroup-cooperative neural operations.
[shader("compute")]
[WaveSize(kCooperativeTextureMapSubgroupSize)]
[numthreads(
    kCooperativeTextureMapSubgroupSize * subgroupCount / kCooperativeTextureMapThreadsY,
    kCooperativeTextureMapThreadsY,
    1)]
void mapTextureInPlaceCooperativeCS<
    let subgroupCount : int,
    T : ITexelElement,
    let Format : int,
    TTransform : IFunc<T, uint2, uint, bool, T>>(
    uint3 threadID: SV_DispatchThreadID,
    uint3 groupThreadID: SV_GroupThreadID,
    RWTexture2D<T, 0, Format> texture,
    uniform uint2 dimensions,
    uniform TTransform transform)
{
    bool isValid = all(threadID.xy < dimensions);
    T value = {};
    if (isValid)
        value = texture[threadID.xy];

    // TODO(WORKAROUND): A specialized generic entry point with an `SV_GroupIndex` parameter
    // currently asserts while converting entry-point parameters to borrow parameters. Derive the
    // equivalent linear index from `SV_GroupThreadID` until that compiler bug is fixed.
    uint groupIndex =
        groupThreadID.x + groupThreadID.y * (kCooperativeTextureMapSubgroupSize * subgroupCount /
                                             kCooperativeTextureMapThreadsY);
    T result = transform(threadID.xy, groupIndex, isValid, value);
    if (isValid)
        texture[threadID.xy] = result;
}

public extension<T : ITexelElement, let Format : int> RWTexture2D<T, 0, Format>
{
    /// Transforms the specified leading region of this texture in place.
    public void mapInPlace<TTransform : IFunc<T, T>>(uint2 dimensions, TTransform transform)
    {
        Grid(dimensions)
            .dispatch<kTextureMapThreadsX, kTextureMapThreadsY>(
                (uint2 pixel) =>
                { this[pixel] = transform(this[pixel]); });
    }

    /// Transforms every texel of this texture in place.
    public void mapInPlace<TTransform : IFunc<T, T>>(TTransform transform)
    {
        uint width;
        uint height;
        this.GetDimensions(width, height);
        this.mapInPlace(uint2(width, height), transform);
    }

    /// Transforms a leading region in place, passing each texel location to `transform`.
    public void mapInPlaceIndexed<TTransform : IFunc<T, uint2, T>>(
        uint2 dimensions,
        TTransform transform)
    {
        Grid(dimensions)
            .dispatch<kTextureMapThreadsX, kTextureMapThreadsY>(
                (uint2 pixel) =>
                { this[pixel] = transform(pixel, this[pixel]); });
    }

    /// Transforms every texel in place, passing each texel location to `transform`.
    public void mapInPlaceIndexed<TTransform : IFunc<T, uint2, T>>(TTransform transform)
    {
        uint width;
        uint height;
        this.GetDimensions(width, height);
        this.mapInPlaceIndexed(uint2(width, height), transform);
    }

    /// Transforms a leading region while keeping every dispatched subgroup lane active.
    public void
    mapInPlaceCooperative<let subgroupCount : int, TTransform : IFunc<T, uint2, uint, bool, T>>(
        uint2 dimensions,
        TTransform transform)
    {
        dispatch(
            mapTextureInPlaceCooperativeCS<subgroupCount, T, Format, TTransform>,
            cooperativeTextureMapGroupCount<subgroupCount>(
                dimensions))(this, dimensions, transform);
    }

    /// Transforms a leading region of this texture into `output`.
    public void
    mapTo<TOutput : ITexelElement, let OutputFormat : int, TTransform : IFunc<TOutput, T>>(
        RWTexture2D<TOutput, 0, OutputFormat> output,
        uint2 dimensions,
        TTransform transform)
    {
        Grid(dimensions)
            .dispatch<kTextureMapThreadsX, kTextureMapThreadsY>(
                (uint2 pixel) =>
                { output[pixel] = transform(this[pixel]); });
    }

    /// Transforms every texel of this texture into `output`.
    public void
    mapTo<TOutput : ITexelElement, let OutputFormat : int, TTransform : IFunc<TOutput, T>>(
        RWTexture2D<TOutput, 0, OutputFormat> output,
        TTransform transform)
    {
        uint width;
        uint height;
        this.GetDimensions(width, height);
        this.mapTo(output, uint2(width, height), transform);
    }

    /// Transforms a leading region into `output`, passing each location to `transform`.
    public void mapToIndexed<
        TOutput : ITexelElement,
        let OutputFormat : int,
        TTransform : IFunc<TOutput, uint2, T>>(
        RWTexture2D<TOutput, 0, OutputFormat> output,
        uint2 dimensions,
        TTransform transform)
    {
        Grid(dimensions)
            .dispatch<kTextureMapThreadsX, kTextureMapThreadsY>(
                (uint2 pixel) =>
                { output[pixel] = transform(pixel, this[pixel]); });
    }

    /// Transforms every texel into `output`, passing each location to `transform`.
    public void mapToIndexed<
        TOutput : ITexelElement,
        let OutputFormat : int,
        TTransform : IFunc<TOutput, uint2, T>>(
        RWTexture2D<TOutput, 0, OutputFormat> output,
        TTransform transform)
    {
        uint width;
        uint height;
        this.GetDimensions(width, height);
        this.mapToIndexed(output, uint2(width, height), transform);
    }
}

struct IdentityTexel<T> : IFunc<T, T>
{
    T operator()(T value) { return value; }
}

[shader("compute")]
[numthreads(kReduceThreadCount, 1, 1)]
void reduceTextureBlocksCS<
    TTexel : ITexelElement,
    TAccumulator,
    TMap : IFunc<TAccumulator, TTexel>,
    TCombine : IFunc<TAccumulator, TAccumulator, TAccumulator>,
    int format>(
    uint3 groupID: SV_GroupID,
    uint localIndex: SV_GroupIndex,
    RWTexture2D<TTexel, 0, format> input,
    RWStructuredBuffer<TAccumulator> output,
    uniform uint2 dimensions,
    uniform TMap mapTexel,
    uniform TCombine combine)
{
    static groupshared TAccumulator shared0[kReduceThreadCount];
    static groupshared TAccumulator shared1[kReduceThreadCount];

    uint count = dimensions.x * dimensions.y;
    uint blockStart = groupID.x * kReduceItemsPerGroup;
    uint firstIndex = blockStart + localIndex * 2;
    uint secondIndex = firstIndex + 1;
    uint blockCount = min(count - blockStart, kReduceItemsPerGroup);
    uint activeCount = (blockCount + 1) / 2;

    if (firstIndex < count)
    {
        uint2 firstLocation = uint2(firstIndex % dimensions.x, firstIndex / dimensions.x);
        TAccumulator value = mapTexel(input[firstLocation]);
        if (secondIndex < count)
        {
            uint2 secondLocation = uint2(secondIndex % dimensions.x, secondIndex / dimensions.x);
            value = combine(value, mapTexel(input[secondLocation]));
        }
        shared0[localIndex] = value;
    }

    GroupMemoryBarrierWithGroupSync();

    bool readShared0 = true;
    while (activeCount > 1)
    {
        uint nextCount = (activeCount + 1) / 2;
        if (localIndex < nextCount)
        {
            uint leftIndex = localIndex * 2;
            uint rightIndex = leftIndex + 1;
            if (readShared0)
            {
                TAccumulator value = shared0[leftIndex];
                if (rightIndex < activeCount)
                    value = combine(value, shared0[rightIndex]);
                shared1[localIndex] = value;
            }
            else
            {
                TAccumulator value = shared1[leftIndex];
                if (rightIndex < activeCount)
                    value = combine(value, shared1[rightIndex]);
                shared0[localIndex] = value;
            }
        }

        GroupMemoryBarrierWithGroupSync();
        activeCount = nextCount;
        readShared0 = !readShared0;
    }

    if (localIndex == 0)
        output[groupID.x] = readShared0 ? shared0[0] : shared1[0];
}

public extension<TTexel : ITexelElement, int format> RWTexture2D<TTexel, 0, format>
{
    /// Maps and reduces every texel into `destination[0]`.
    public void reduceTo<
        TAccumulator,
        TMap : IFunc<TAccumulator, TTexel>,
        TCombine : IFunc<TAccumulator, TAccumulator, TAccumulator>>(
        RWStructuredBuffer<TAccumulator> destination,
        TMap mapTexel,
        TCombine combine)
    {
        uint width;
        uint height;
        this.GetDimensions(width, height);
        uint count = width * height;
        uint groupCount = reduceGroupCount(count);
        if (groupCount == 1)
        {
            dispatch(
                reduceTextureBlocksCS<TTexel, TAccumulator, TMap, TCombine, format>,
                uint3(1, 1, 1))(this, destination, uint2(width, height), mapTexel, combine);
            return;
        }

        RWStructuredBuffer<TAccumulator> scratch =
            RWStructuredBuffer<TAccumulator>::alloc(groupCount);
        dispatch(
            reduceTextureBlocksCS<TTexel, TAccumulator, TMap, TCombine, format>,
            uint3(groupCount, 1, 1))(this, scratch, uint2(width, height), mapTexel, combine);
        scratch.reduceTo(destination, groupCount, combine);
    }

    /// Reduces every texel into `destination[0]` without changing its type.
    public void reduceTo<TCombine : IFunc<TTexel, TTexel, TTexel>>(
        RWStructuredBuffer<TTexel> destination,
        TCombine combine)
    {
        this.reduceTo(destination, IdentityTexel<TTexel>(), combine);
    }
}
