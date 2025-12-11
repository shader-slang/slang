// extension-interface-requirement.slang

// Test that extension method declarations without bodies are properly diagnosed.
// Extensions cannot declare new interface requirements - they must provide implementations.
// See https://github.com/shader-slang/slang/issues/9198

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv

// Simple interface
public interface IROTensor<T, let D : int>
{
    T read_buffer(int idx);
}

// Concrete type implementing the interface
public struct ROTensor<T, let D : int> : IROTensor<T, D>
{
    public StructuredBuffer<T> _data;
    public uint[D] _strides;
    public uint _offset;

    public T read_buffer(int idx)
    {
        return _data[idx];
    }
}

// Helper function
int _idx(int i0, uint stride[1], uint offset)
{
    return i0 * stride[0] + offset;
}

// This concrete extension works fine - it has a function body
public extension<T> ROTensor<T, 1>
{
    public T load(int i0)
    {
        int idx = _idx(i0, this._strides, this._offset);
        return this.read_buffer(idx);
    }
}

// ERROR: Extension with generic type parameter that declares a method without a body.
// Extensions cannot declare new interface requirements.
public extension<T, TensorType : IROTensor<T, 1>> TensorType
{
    // CHECK: ([[# @LINE+1]]): error 45001
    public T loadGeneric(int i0);  // Error: function declaration without definition
}

// Test entry point
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    ROTensor<float, 1> tensor;
    float value = tensor.load(0);
    float value2 = tensor.loadGeneric(0);  // Uses the extension method
}
