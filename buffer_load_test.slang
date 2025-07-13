[vk::binding(0, 0)] RWByteAddressBuffer bufferHandles[];
[vk::binding(1, 0)] RWByteAddressBuffer outputBuffer;

struct RenderResourceHandle {
    uint _index;
    uint index() { return _index; }
}

struct Buffer<T> {
    RenderResourceHandle handle;
    __subscript(uint i) -> T
    {
        get { return bufferHandles[handle.index()].Load<T>(sizeof(T) * i); }
        set { bufferHandles[handle.index()].Store<T>(sizeof(T) * i, newValue); }
    }
}

struct Bindings {
    Buffer<uint32_t> indices;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) 
{
    Bindings bindings;
    bindings.indices.handle._index = 0;
    
    uint sum = 0;
    // Test loading - this is where the issue would manifest
    for(int j = 0; j < 10; j++) {
        uint32_t value = bindings.indices[j];
        sum += value;
    }
    
    // Use the result so it doesn't get optimized away
    outputBuffer.Store(0, sum);
}