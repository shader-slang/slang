// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.compute.grid;

public interface IZeroInitializable
{
    static This zero();
}

public extension bool : IZeroInitializable
{
    public static bool zero()
    {
        return false;
    }
}

public extension int : IZeroInitializable
{
    public static int zero()
    {
        return 0;
    }
}

public extension uint : IZeroInitializable
{
    public static uint zero()
    {
        return 0u;
    }
}

public extension int64_t : IZeroInitializable
{
    public static int64_t zero()
    {
        return int64_t(0);
    }
}

public extension uint64_t : IZeroInitializable
{
    public static uint64_t zero()
    {
        return uint64_t(0);
    }
}

public extension half : IZeroInitializable
{
    public static half zero()
    {
        return half(0.0);
    }
}

public extension float : IZeroInitializable
{
    public static float zero()
    {
        return 0.0;
    }
}

public extension double : IZeroInitializable
{
    public static double zero()
    {
        return double(0.0);
    }
}

public static const uint kBufferInitThreads = 64;

public extension<T> RWStructuredBuffer<T>
{
    public void fill(T value)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);

        Grid(count).dispatch<kBufferInitThreads>(
            (uint index) =>
            { this[index] = value; });
    }
}

public extension<T : IZeroInitializable> RWStructuredBuffer<T>
{
    public void zero()
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);

        Grid(count).dispatch<kBufferInitThreads>(
            (uint index) =>
            { this[index] = T.zero(); });
    }
}

public static const uint kBufferMapThreads = 64;

public extension<T> RWStructuredBuffer<T>
{
    /// Transforms the first `count` elements of this buffer in place.
    public void mapInPlace<TTransform : IFunc<T, T>>(uint count, TTransform transform)
    {
        Grid(count).dispatch<kBufferMapThreads>(
            (uint index) =>
            { this[index] = transform(this[index]); });
    }

    /// Transforms every physically allocated element of this buffer in place.
    public void mapInPlace<TTransform : IFunc<T, T>>(TTransform transform)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);
        this.mapInPlace(count, transform);
    }

    /// Transforms the first `count` elements in place, passing each index to `transform`.
    public void mapInPlaceIndexed<TTransform : IFunc<T, uint, T>>(uint count, TTransform transform)
    {
        Grid(count).dispatch<kBufferMapThreads>(
            (uint index) =>
            { this[index] = transform(index, this[index]); });
    }

    /// Transforms every allocated element in place, passing each index to `transform`.
    public void mapInPlaceIndexed<TTransform : IFunc<T, uint, T>>(TTransform transform)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);
        this.mapInPlaceIndexed(count, transform);
    }

    /// Visits the first `count` elements with their indices for side-effect-only transforms.
    public void mapIndexed<TTransform : IFunc<void, uint, T>>(uint count, TTransform transform)
    {
        Grid(count).dispatch<kBufferMapThreads>(
            (uint index) =>
            { transform(index, this[index]); });
    }

    /// Visits every allocated element with its index for side-effect-only transforms.
    public void mapIndexed<TTransform : IFunc<void, uint, T>>(TTransform transform)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);
        this.mapIndexed(count, transform);
    }

    /// Transforms the first `count` input elements into `output`.
    public void mapTo<TOutput, TTransform : IFunc<TOutput, T>>(
        RWStructuredBuffer<TOutput> output,
        uint count,
        TTransform transform)
    {
        Grid(count).dispatch<kBufferMapThreads>(
            (uint index) =>
            { output[index] = transform(this[index]); });
    }

    /// Transforms every physically allocated input element into `output`.
    public void mapTo<TOutput, TTransform : IFunc<TOutput, T>>(
        RWStructuredBuffer<TOutput> output,
        TTransform transform)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);
        this.mapTo(output, count, transform);
    }

    /// Transforms the first `count` elements into `output`, passing each index to `transform`.
    public void mapToIndexed<TOutput, TTransform : IFunc<TOutput, uint, T>>(
        RWStructuredBuffer<TOutput> output,
        uint count,
        TTransform transform)
    {
        Grid(count).dispatch<kBufferMapThreads>(
            (uint index) =>
            { output[index] = transform(index, this[index]); });
    }

    /// Transforms every allocated element into `output`, passing each index to `transform`.
    public void mapToIndexed<TOutput, TTransform : IFunc<TOutput, uint, T>>(
        RWStructuredBuffer<TOutput> output,
        TTransform transform)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);
        this.mapToIndexed(output, count, transform);
    }
}

public extension<T> StructuredBuffer<T, ScalarDataLayout>
{
    /// Visits the first `count` elements with their indices for side-effect-only transforms.
    public void mapIndexed<TTransform : IFunc<void, uint, T>>(uint count, TTransform transform)
    {
        Grid(count).dispatch<kBufferMapThreads>(
            (uint index) =>
            { transform(index, this[index]); });
    }

    /// Visits every allocated element with its index for side-effect-only transforms.
    public void mapIndexed<TTransform : IFunc<void, uint, T>>(TTransform transform)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);
        this.mapIndexed(count, transform);
    }

    /// Transforms the first `count` elements into `output`, passing each index to `transform`.
    public void mapToIndexed<TOutput, TTransform : IFunc<TOutput, uint, T>>(
        RWStructuredBuffer<TOutput> output,
        uint count,
        TTransform transform)
    {
        Grid(count).dispatch<kBufferMapThreads>(
            (uint index) =>
            { output[index] = transform(index, this[index]); });
    }

    /// Transforms every allocated element into `output`, passing each index to `transform`.
    public void mapToIndexed<TOutput, TTransform : IFunc<TOutput, uint, T>>(
        RWStructuredBuffer<TOutput> output,
        TTransform transform)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);
        this.mapToIndexed(output, count, transform);
    }
}

public static const uint kReduceThreadCount = 64;
public static const uint kReduceItemsPerGroup = kReduceThreadCount * 2;

public uint reduceGroupCount(uint count)
{
    return (count + (kReduceItemsPerGroup - 1)) / kReduceItemsPerGroup;
}

[shader("compute")]
[numthreads(kReduceThreadCount, 1, 1)]
void reduceBufferBlocksCS<T, TCombine : IFunc<T, T, T>>(
    uint3 groupID: SV_GroupID,
    uint localIndex: SV_GroupIndex,
    RWStructuredBuffer<T> input,
    RWStructuredBuffer<T> output,
    uniform uint count,
    uniform TCombine combine)
{
    static groupshared T shared0[kReduceThreadCount];
    static groupshared T shared1[kReduceThreadCount];

    uint blockStart = groupID.x * kReduceItemsPerGroup;
    uint firstIndex = blockStart + localIndex * 2;
    uint secondIndex = firstIndex + 1;
    uint blockCount = min(count - blockStart, kReduceItemsPerGroup);
    uint activeCount = (blockCount + 1) / 2;

    if (firstIndex < count)
    {
        T value = input[firstIndex];
        if (secondIndex < count)
            value = combine(value, input[secondIndex]);
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
                T value = shared0[leftIndex];
                if (rightIndex < activeCount)
                    value = combine(value, shared0[rightIndex]);
                shared1[localIndex] = value;
            }
            else
            {
                T value = shared1[leftIndex];
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

public extension<T> RWStructuredBuffer<T>
{
    /// Reduces the first `count` elements into `destination[0]`.
    ///
    /// A zero-element reduction leaves the destination unchanged. `combine` must be associative.
    public void reduceTo<TCombine : IFunc<T, T, T>>(
        RWStructuredBuffer<T> destination,
        uint count,
        TCombine combine)
    {
        if (count == 0)
            return;

        uint groupCount = reduceGroupCount(count);
        if (groupCount == 1)
        {
            dispatch(
                reduceBufferBlocksCS<T, TCombine>,
                uint3(1, 1, 1))(this, destination, count, combine);
            return;
        }

        RWStructuredBuffer<T> scratch0 = RWStructuredBuffer<T>::alloc(groupCount);
        RWStructuredBuffer<T> scratch1 = RWStructuredBuffer<T>::alloc(reduceGroupCount(groupCount));
        RWStructuredBuffer<T> input = this;
        bool writeScratch0 = true;
        uint remaining = count;

        while (reduceGroupCount(remaining) > 1)
        {
            groupCount = reduceGroupCount(remaining);
            if (writeScratch0)
            {
                dispatch(
                    reduceBufferBlocksCS<T, TCombine>,
                    uint3(groupCount, 1, 1))(input, scratch0, remaining, combine);
                input = scratch0;
            }
            else
            {
                dispatch(
                    reduceBufferBlocksCS<T, TCombine>,
                    uint3(groupCount, 1, 1))(input, scratch1, remaining, combine);
                input = scratch1;
            }

            remaining = groupCount;
            writeScratch0 = !writeScratch0;
        }

        dispatch(
            reduceBufferBlocksCS<T, TCombine>,
            uint3(1, 1, 1))(input, destination, remaining, combine);
    }

    /// Reduces every physically allocated element into `destination[0]`.
    public void reduceTo<TCombine : IFunc<T, T, T>>(
        RWStructuredBuffer<T> destination,
        TCombine combine)
    {
        uint count;
        uint stride;
        this.GetDimensions(count, stride);
        this.reduceTo(destination, count, combine);
    }
}
