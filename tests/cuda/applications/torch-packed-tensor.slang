// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Reduced from SlangPy's pack(TorchTensor) -> read_first(Tensor<float, 1>) wrapper.
// Consider a tensor stored in a ParameterBlock and passed to a value-taking helper. The load
// below resolves to the generic slicing overload with SliceD == D, even for a full-rank value.
// It copies descriptor fields and forwards the stride array and offset through out parameters.
// Replacing that overload with value = this changes the diagnostic and is not this reproducer.
// ContextND<0>, the pointer-backed descriptor, both overloads and the slice call are intentional.
// Desired execution: result[0] receives tensor.data[tensor.offset], e.g. 42 for [42, 1, 2].
// Definitions are reduced from SlangPy's core/tensor/tensorcommon shader sources (Apache-2.0
// WITH LLVM-exception); application revision and exact paths are in the corpus manifest.

public struct ContextND<let N : int>
{
    // Workaround for slang bug #6039
    public uint3 thread_id;
    public int[N] call_id;

    public ContextND<0> map(int)
    {
        return { thread_id };
    }

    public ContextND<M> map<let M : int>(int[M] mapping)
    {
        ContextND<M> result;
        result.thread_id = thread_id;
        [ForceUnroll]
        for (int i = 0; i < M; ++i)
            result.call_id[i] = call_id[mapping[i]];
        return result;
    }
}
public typealias Context0D = ContextND<0>;

public typealias Context = ContextND<0>;

public struct StorageTraits<T>
{
    public typealias BufferType = T*;
}
public struct Tensor<T, let D : int>
{
    public StorageTraits<T>::BufferType _data;
    public uint[D] _shape;
    public uint[D] _strides;
    public uint _offset;
    public T load<I : __BuiltinIntegerType>(I idx[D])
    {
        int offset = _offset;
        [ForceUnroll]
        for (int i = 0; i < D; i++)
            offset += idx[i].toUInt() * _strides[i];
        return _data[offset];
    }
    public __subscript<I : __BuiltinIntegerType>(I indices[D]) -> T
    {
        get
        {
            return load(indices);
        }
    }
    public void __slangpy_load(Context0D context, out This value)
    {
        value = this;
    }
    public void __slangpy_load<let SliceD : int>(
        ContextND<D - SliceD> context, out Tensor<T, SliceD> value)
    {
        value._data = _data;
        slice(context.call_id, _strides, _offset, value._strides, value._offset);
        [ForceUnroll]
        for (int i = 0; i < SliceD; i++)
            value._shape[i] = _shape[D - SliceD + i];
    }

}
public struct RWValueRef<T>
{
    RWStructuredBuffer<T> value;
    public void __slangpy_store(Context0D context, in T value)
    {
        this.value[0] = value;
    }
}

public extension<T> Tensor<T, 1>
{
    public __subscript(int index) -> T
    {
        get
        {
            return _data[index * _strides[0] + _offset];
        }
    }
}

public void slice<let D : int, let SliceD : int>(
    int idx[D - SliceD],
    uint stride[D],
    uint offset,
    out uint newStride[SliceD],
    out uint newOffset)
{
    [ForceUnroll]
    for (int i = 0; i < SliceD; i++)
        newStride[i] = stride[D - SliceD + i];
    newOffset = offset;
    [ForceUnroll]
    for (int i = 0; i < D - SliceD; i++)
        newOffset += stride[i] * idx[i];
}

float read_first(Tensor<float, 1> buf)
{
    return buf[0];
}

static const int dispatch_thread_x_stride = 67108832;

static const int _m_buf = 0;
static const int _m__result = 0;

ParameterBlock<Tensor<float, 1>> _param_buf;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    int3 physical_thread_id : SV_DispatchThreadID,
    uniform uint3 _thread_count,
    uniform RWValueRef<float> _result)
{
    int3 flat_call_thread_id = int3(
        physical_thread_id.y * dispatch_thread_x_stride + physical_thread_id.x, 0, 0);
    if (any(flat_call_thread_id >= _thread_count))
        return;
    Context __slangpy_context__ = {flat_call_thread_id};
    float __tmp__result;
    Tensor<float, 1> __tmp_buf;
    _param_buf.__slangpy_load(__slangpy_context__.map(_m_buf), __tmp_buf);
    __tmp__result = read_first(__tmp_buf);
    _result.__slangpy_store(__slangpy_context__.map(_m__result), __tmp__result);
}
