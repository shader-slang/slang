import slang.runtime;

[host]
void main()
{
    // Create an empty buffer to hold 1024 float values.
    let values = RWStructuredBuffer<float>::alloc(1024);

    // Populate the buffer via a compute shader lambda (1024 threads)
    // Buffers are captured and bound automatically!
    //
    Grid(1024).dispatch(
        (uint i) =>
        { values[i] = float(i); });

    // Select group size via generics (default = 64, 1, 1)
    Grid(1024).dispatch<128>(
        (uint i) =>
        { values[i] = float(i); });

    // Compute (`scale` * x + 1) and store it in a new buffer.
    // Scalars (e.g. `scale`) are captured as push/root constants by default.
    //
    let output = RWStructuredBuffer<float>::alloc(1024);
    float scale = 2.0;
    Grid(1024).dispatch(
        (uint i) =>
        { output[i] = scale * values[i] + 1.0; });

    // Read back to CPU to verify information.
    //    Warning: reading back flushes device commands & forces synchronization.
    print(readBack(output, 256)); // 513.00

    // Multi-dimensional dispatch by using an nd-grid.
    Grid(uint2(32, 32))
        .dispatch<8, 8>(
            (uint2 tid) =>
            { output[tid.y * 32 + tid.x] = float(tid.x + tid.y * 32); });
}
