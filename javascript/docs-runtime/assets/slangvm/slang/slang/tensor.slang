// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.resource.resizable_buffer;
import slang.numerics;
import slang.compute.buffer_utils;
import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

/// A contiguous GPU scratch tensor with one to four dimensions. The first axis is contiguous:
/// Tensor<T, 2>(uint2(width, height))[uint2(x, y)] addresses x + width * y.
/// Assignments share the current buffer but have independent shapes. Growing a copy only replaces
/// its buffer; already captured tensors continue to refer to the old allocation.
/// Storage stays descriptor-backed on both host and device, including in autodiff contexts.
public struct Tensor<T, let Rank : int>
{
    public typealias Index = vector<uint, Rank>;

    /// Carries a descriptor-backed allocation and a snapshot of this tensor's logical layout.
    /// Dereference in shader code to use the regular tensor indexing and metadata methods.
    public struct Handle
    {
        public ResizeableBuffer<T>.Handle storage;
        public Index shape;
        public uint count;
    }

    private no_diff ResizeableBuffer<T>.Handle storage;
    private no_diff Index _shape;
    private no_diff uint count;

    public property buffer : RWStructuredBuffer<T>
    {
        get
        {
            return *storage.buffer;
        }
    }
    public property shape : Index
    {
        get {
            return _shape;
        }
    }
    public property capacity : uint
    {
        get {
            return storage.capacity;
        }
    }

    [host]
    private static uint checkedElementCount(Index sizes)
    {
        static_assert(Rank >= 1 && Rank <= 4, "Tensor rank must be between one and four.");
        // Check for an empty shape first: even (UINT_MAX, UINT_MAX, 0) needs no elements.
        if (any(sizes == 0))
            return 0;
        uint total = 1;
        for (uint axis = 0; axis < uint(Rank); ++axis)
        {
            assert(total <= 0xffffffffu / sizes[axis]);
            total *= sizes[axis];
        }
        return total;
    }

    [host]
    public __init(Index sizes)
    {
        count = checkedElementCount(sizes);
        _shape = sizes;
        // The backing helper uses a one-element allocation for a logically empty tensor.
        storage = ResizeableBuffer<T>(count).getDescriptorHandle();
    }

    /// Reconstructs the captured view without allocating, resizing, or revalidating its layout.
    public __init(Handle handle)
    {
        storage = handle.storage;
        _shape = handle.shape;
        count = handle.count;
    }

    /// Captures both storage and layout; subsequent reserve()/resize() calls preserve this view.
    [host]
    public Handle getDescriptorHandle() { return Handle(storage, _shape, count); }

    public uint elementCount() { return count; }

    /// Reads one logical element by flat index, waiting for its GPU contents on the host.
    [host]
    public T readBack(uint index)
    {
        assert(index < count);
        return buffer.readBack(index);
    }

    /// Ensures capacity without changing the shape. Growth discards contents; callers must
    /// overwrite the scratch data before reading it. Requests within capacity preserve data.
    [host]
    [mutating]
    public void reserve(uint elementCapacity)
    {
        var allocation = *storage;
        allocation.reserve(elementCapacity);
        storage = allocation.getDescriptorHandle();
    }

    /// Changes the shape and grows storage only if needed. Does not initialize or rearrange data;
    /// within capacity, the same flat contents are reinterpreted using the new dimensions.
    /// Growth discards the old contents, just as reserve does.
    [host]
    [mutating]
    public void resize(Index sizes)
    {
        uint newCount = checkedElementCount(sizes);
        reserve(newCount);
        _shape = sizes;
        count = newCount;
    }

    /// Flattens a coordinate. Host accesses check the logical shape; shader bounds remain the
    /// caller's contract.
    [ForceInline]
    public uint flatIndex(Index index)
    {
        __target_switch
        {
        case hgbc: assert(!any(index >= _shape)); break;
        default: break;
        }
        uint flat = 0;

        [ForceUnroll]
        for (uint axis = uint(Rank); axis > 0; --axis)
            flat = flat * _shape[axis - 1] + index[axis - 1];

        return flat;
    }

    /// Converts an in-bounds flat index to a coordinate, with the first axis contiguous.
    /// Like flatIndex, this requires a valid logical element; empty tensors have no valid indices.
    [ForceInline]
    public Index indexFromFlat(uint flat)
    {
        Index index;
        [ForceUnroll]
        for (uint axis = 0; axis + 1 < uint(Rank); ++axis)
        {
            index[axis] = flat % _shape[axis];
            flat /= _shape[axis];
        }
        index[Rank - 1] = flat;
        return index;
    }

    public __subscript(Index index)->T
    {
        get {
            return buffer[flatIndex(index)];
        }
    // Only the referenced GPU memory changes, not this tensor's shape or resource handle.
    [nonmutating]
        set {
            buffer[flatIndex(index)] = newValue;
        }
    }
}

__generic<T, let Rank : int> [ForceInline]
public __prefix Tensor<T, Rank> operator*(Tensor<T, Rank>.Handle handle)
{
    return Tensor<T, Rank>(handle);
}

/// Differentials own separate storage. Assigning a tensor does not allocate or clear gradients.
/// Keeping this conformance conditional preserves ordinary integer and boolean tensors.
public extension<T : IDifferentiable, let Rank : int> Tensor<T, Rank> : IDifferentiablePtrType
{
    public typealias Differential = Tensor<T.Differential, Rank>;
}

/// Ordinary tensor operations do not record differentiation contexts. Temporary allocations
/// are local to each operation and use HGVM's buffer pool.
public extension<T, let Rank : int> Tensor<T, Rank>
{
    /// Copies one value into a new singleton tensor (every dimension has extent one).
    /// Named construction keeps integer values distinct from allocation by shape.
    /// Import slang.tensor.differentiable for derivatives with respect to value.
    [host]
    public static Tensor<T, Rank> fromValue(T value)
    {
        let result = Tensor<T, Rank>(vector<uint, Rank>(1u));
        result.fill(value);
        return result;
    }

    /// Initializes logical elements only; spare capacity is not part of the tensor.
    [host]
    public void fill(T value)
    {
        let destination = buffer;
        Grid(elementCount())
            .dispatch(
                (uint index) =>
                { destination[index] = value; });
    }

    /// Copies the logical contents to a distinct allocation, preserving the current shape.
    [host]
    public Tensor<T, Rank> copy()
    {
        let result = Tensor<T, Rank>(shape);
        buffer.mapTo(result.buffer, elementCount(), (T value) => value);
        return result;
    }
}

public extension<T, let Rank : int> Tensor<T, Rank>
{
    /// Maps logical coordinates and values into a distinct tensor of the same shape.
    [host]
    public Tensor<U, Rank> map<U, F : IFunc<U, vector<uint, Rank>, T>>(F transform)
    {
        let input = this;
        let result = Tensor<U, Rank>(shape);
        buffer.mapToIndexed(
            result.buffer,
            elementCount(),
            (uint flat, T value) => transform(input.indexFromFlat(flat), value));
        return result;
    }

    /// Reduces logical elements on the GPU. The identity supplies the result for an empty tensor;
    /// combine must be associative. The returned one-element tensor does not require a readback.
    [host]
    public Tensor<T, 1> reduce<F : IFunc<T, T, T>>(T identity, F combine)
    {
        let result = Tensor<T, 1>(1u);
        if (elementCount() == 0)
            result.fill(identity);
        else
            buffer.reduceTo(result.buffer, elementCount(), combine);
        return result;
    }
}

/// Computes a scalar inner product over both tensor elements and their numeric components.
[host]
public Tensor<float, 1> innerProduct<T : IDotProduct, let Rank : int>(
    Tensor<T, Rank> left,
    Tensor<T, Rank> right)
    where T.Scalar == float
{
    for (uint axis = 0; axis < uint(Rank); ++axis)
        assert(left.shape[axis] == right.shape[axis]);
    let products = left.map(
        (vector<uint, Rank> index, T value) =>
        {
            float product = value.dotProductWith(right[index]);
            return product;
        });
    return products.reduce(0.0f, (float a, float b) => a + b);
}
