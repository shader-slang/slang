// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.core.simple_assert;

/// Converts a logical grid's index type to and from the GPU's three-dimensional coordinates.
public interface IGridIndex
{
    uint3 gridExtent();
    static This fromGridIndex(uint3 index);
}

public extension uint : IGridIndex
{
    public uint3 gridExtent()
    {
        return uint3(this, 1, 1);
    }
    public static uint fromGridIndex(uint3 index)
    {
        return index.x;
    }
}

public extension uint2 : IGridIndex
{
    // TODO(WORKAROUND): HGBC does not yet flatten the mixed constructor uint3(this, 1).
    public uint3 gridExtent()
    {
        return uint3(this.x, this.y, 1);
    }
    public static uint2 fromGridIndex(uint3 index)
    {
        return index.xy;
    }
}

public extension uint3 : IGridIndex
{
    public uint3 gridExtent()
    {
        return this;
    }
    public static uint3 fromGridIndex(uint3 index)
    {
        return index;
    }
}

public static const uint kGridSubgroupSize = 32;
static const uint kGridMaxDispatchGroups = 65535;

/// A distributed collection with one register-local value per lane, not a GPU buffer.
/// Use Wave.map/dispatch to access individual values. laneValue is the explicit escape hatch
/// for collective library adapters; execution-scope safety is not yet checked by the language.
public struct WaveValues<T>
{
    public T laneValue;
    public __init(T value) { laneValue = value; }
}

public extension<each T> WaveValues<Tuple<T>>
{
    /// Splits a tuple-valued collection into its component collections, including padding.
    /// This only rearranges register-local values; it does not execute another map callback.
    public Tuple<expand WaveValues<each T>> unzip()
    {
        let elements = laneValue;
        return makeTuple(expand WaveValues<each T>(each elements));
    }
}

/// A 32-lane cooperative scope over a Grid. All lanes execute collective code together.
/// Scalar callbacks may diverge, but must not call collective operations. Padding lanes skip
/// these callbacks and receive the supplied inactive value (default-initialized if omitted),
/// while still joining collective work.
/// This is currently a library convention, not a statically enforced execution-scope boundary.
public struct Wave<Index : IGridIndex, let SubgroupCount : int>
{
    private Index index;
    private bool isValid;

    public static const uint subgroupSize = kGridSubgroupSize;
    public static const int subgroupCount = SubgroupCount;

    public __init(Index index, bool isValid)
    {
        this.index = index;
        this.isValid = isValid;
    }

    /// Runs local per-coordinate work and initializes padding without calling the function.
    public WaveValues<T> map<T, Func : IFunc<T, Index>>(Func function, T inactiveValue = {})
    {
        T value = inactiveValue;
        if (isValid)
            value = function(index);
        return WaveValues<T>(value);
    }

    /// Transforms each valid lane's value without another dispatch or intermediate allocation.
    public WaveValues<U> map<T, U, Func : IFunc<U, T>>(
        WaveValues<T> values,
        Func function,
        U inactiveValue = {})
    {
        U value = inactiveValue;
        if (isValid)
            value = function(values.laneValue);
        return WaveValues<U>(value);
    }

    /// Transforms each valid lane's value without another dispatch or intermediate allocation.
    public WaveValues<U> map<T, U, Func : IFunc<U, Index, T>>(
        WaveValues<T> values,
        Func function,
        U inactiveValue = {})
    {
        U value = inactiveValue;
        if (isValid)
            value = function(index, values.laneValue);
        return WaveValues<U>(value);
    }

    /// Performs per-coordinate side effects on valid lanes only. This is local device work.
    public void dispatch<Func : IFunc<void, Index>>(Func function)
    {
        if (isValid)
            function(index);
    }

    /// Consumes a distributed value, passing the corresponding coordinate to each valid lane.
    public void dispatch<T, Func : IFunc<void, Index, T>>(WaveValues<T> values, Func function)
    {
        if (isValid)
            function(index, values.laneValue);
    }

    /// Consumes two collections from this scope without materializing a zipped buffer.
    public void dispatch<T, U, Func : IFunc<void, Index, T, U>>(
        WaveValues<T> first,
        WaveValues<U> second,
        Func function)
    {
        if (isValid)
            function(index, first.laneValue, second.laneValue);
    }
}

/// Computes a conservative cross-backend dispatch shape, spilling long X grids into Y.
[host]
uint3 gridDispatchSize<let X : int, let Y : int, let Z : int>(uint3 extent, out uint3 groupCount)
{
    static_assert(X > 0 && Y > 0 && Z > 0, "Workgroup dimensions must be positive.");
    static_assert(X <= 1024 && Y <= 1024 && Z <= 64, "Workgroup dimensions exceed grid limits.");
    static_assert(X * Y <= 1024 && X * Y * Z <= 1024, "Workgroups cannot exceed 1024 lanes.");
    uint3 threads = uint3(X, Y, Z);
    // Callers reject empty extents. Subtract before dividing to avoid uint overflow.
    groupCount = (extent - 1) / threads + 1;
    assert(groupCount.y <= kGridMaxDispatchGroups && groupCount.z <= kGridMaxDispatchGroups);
    if (groupCount.x <= kGridMaxDispatchGroups)
        return groupCount;

    // Only a one-dimensional logical group grid can reuse the physical Y dimension this way.
    assert(groupCount.y == 1 && groupCount.z == 1);
    uint rows =
        groupCount.x / kGridMaxDispatchGroups + uint(groupCount.x % kGridMaxDispatchGroups != 0);
    assert(rows <= kGridMaxDispatchGroups);
    return uint3(kGridMaxDispatchGroups, rows, 1);
}

/// Maps physical groups back to the logical grid and rejects entirely unused spill groups.
bool gridInvocation<let X : int, let Y : int, let Z : int>(
    uint3 groupID,
    uint3 localID,
    uint3 extent,
    uint3 groupCount,
    out uint3 index,
    out bool isValid)
{
    if (groupCount.x > kGridMaxDispatchGroups)
        groupID = uint3(groupID.x + groupID.y * kGridMaxDispatchGroups, 0, 0);
    index = 0;
    isValid = false;
    if (any(groupID >= groupCount))
        return false;
    uint3 origin = groupID * uint3(X, Y, Z);
    // Test the remaining extent before adding localID, which can wrap on a padding lane.
    isValid = all(localID < extent - origin);
    index = origin + localID;
    return true;
}

[shader("compute")]
[numthreads(X, Y, Z)]
void dispatchGridCS<
    let X : int,
    let Y : int,
    let Z : int,
    Index : IGridIndex,
    Func : IFunc<void, Index>>(
    uint3 groupID: SV_GroupID,
    uint3 localID: SV_GroupThreadID,
    uniform uint3 extent,
    uniform uint3 groupCount,
    uniform Func function)
{
    uint3 index;
    bool isValid;
    if (gridInvocation<X, Y, Z>(groupID, localID, extent, groupCount, index, isValid) && isValid)
        function(Index.fromGridIndex(index));
}

[shader("compute")]
[WaveSize(kGridSubgroupSize)]
[numthreads(kGridSubgroupSize * subgroupCount, 1, 1)]
void dispatchCooperativeGridCS<
    let subgroupCount : int,
    Index : IGridIndex,
    Func : IFunc<void, Index, uint, bool>>(
    uint3 groupID: SV_GroupID,
    uint3 localID: SV_GroupThreadID,
    uniform uint3 extent,
    uniform uint3 groupCount,
    uniform Func function)
{
    uint3 index;
    bool isValid;
    // This test is uniform across the whole workgroup, including groups added by X spilling.
    if (!gridInvocation<kGridSubgroupSize * subgroupCount, 1, 1>(
            groupID,
            localID,
            extent,
            groupCount,
            index,
            isValid))
        return;
    // TODO(WORKAROUND): Specialized generic entry points currently fail with SV_GroupIndex.
    // As in texture_map, derive the linear local index from SV_GroupThreadID instead.
    function(Index.fromGridIndex(index), localID.x, isValid);
}

// Keep builtin dispatch calls outside DispatchGrid: its dispatch member shadows the builtin name.
[host]
void enqueueGrid<
    let X : int,
    let Y : int,
    let Z : int,
    Index : IGridIndex,
    Func : IFunc<void, Index>>(Index size, Func function)
{
    uint3 extent = size.gridExtent();
    if (any(extent == 0))
        return;
    uint3 groupCount;
    uint3 dispatchSize = gridDispatchSize<X, Y, Z>(extent, groupCount);
    dispatch(dispatchGridCS<X, Y, Z, Index, Func>, dispatchSize)(extent, groupCount, function);
}

[host]
void enqueueCooperativeGrid<
    let subgroupCount : int,
    Index : IGridIndex,
    Func : IFunc<void, Index, uint, bool>>(Index size, Func function)
{
    static_assert(subgroupCount > 0 && subgroupCount <= 32, "Expected 1 to 32 subgroups.");
    uint3 extent = size.gridExtent();
    if (any(extent == 0))
        return;
    uint3 groupCount;
    uint3 dispatchSize =
        gridDispatchSize<kGridSubgroupSize * subgroupCount, 1, 1>(extent, groupCount);
    dispatch(
        dispatchCooperativeGridCS<subgroupCount, Index, Func>,
        dispatchSize)(extent, groupCount, function);
}

/// A resource-free iteration domain. Construction does not allocate or enqueue GPU work.
public struct DispatchGrid<Index : IGridIndex>
{
    public Index size;

    public __init(Index size) { this.size = size; }

    /// Enqueues one callback per valid coordinate, skipping padding lanes. Does not synchronize.
    /// Callbacks using workgroup collectives must use dispatchCooperative instead.
    [host]
    public void dispatch<
        let X : int = 64,
        let Y : int = 1,
        let Z : int = 1,
        Func : IFunc<void, Index>>(Func function)
    {
        enqueueGrid<X, Y, Z>(size, function);
    }

    /// Enqueues whole workgroups of subgroupCount 32-lane subgroups, laid out along X.
    /// All lanes invoke the callback; padding indices are unspecified and isValid is false.
    /// Guard their resource accesses, but do not skip collective operations on padding lanes.
    [host]
    public void dispatchCooperative<let subgroupCount : int, Func : IFunc<void, Index, uint, bool>>(
        Func function)
    {
        enqueueCooperativeGrid<subgroupCount>(size, function);
    }

    /// Enqueues collective wave callbacks, with subgroupCount waves per workgroup.
    /// Every lane invokes the outer function, including padding lanes. Use the scope's local
    /// map/dispatch for pixel work, and only subgroup-local collectives between those calls.
    /// In particular, independent wave control flow must not reach workgroup-wide barriers.
    [host]
    public void dispatchWaves<
        let subgroupCount : int,
        Func : IFunc<void, Wave<Index, subgroupCount>>>(Func function)
    {
        // TODO(WORKAROUND): Naming the type here avoids capturing subgroupCount as a runtime
        // lambda field when it appears in a generic type argument inside the lambda body.
        typealias Scope = Wave<Index, subgroupCount>;
        enqueueCooperativeGrid<subgroupCount>(
            size,
            (Index index, uint localIndex, bool isValid) =>
            { function(Scope(index, isValid)); });
    }
}

/// Infers the coordinate type from a uint, uint2, or uint3 extent.
public DispatchGrid<Index> Grid<Index : IGridIndex>(Index size)
{
    return DispatchGrid<Index>(size);
}
