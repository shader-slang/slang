import slang.runtime;

[shader("compute")]
[numthreads(1, 1, 1)]
void fill(RWStructuredBuffer<uint> output)
{
    output[0] = 42;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void fillColor(RWTexture2D<float4> output, uint3 thread : SV_DispatchThreadID)
{
    output[thread.xy] = float4(0.25, 0.5, 0.75, 1.0);
}

[host]
uint runReadBack()
{
    RWStructuredBuffer<uint> output = RWStructuredBuffer<uint>::alloc(1);
    dispatch(fill, uint3(1, 1, 1))(output);
    return output.readBack(0);
}

[host]
uint present(Window window)
{
    let surface = createSurface<TextureFormat::rgba8Unorm>(window);
    let target = acquireSurfaceTexture(surface);
    let image = RWTexture2D<float4>::alloc(256, 256);
    dispatch(fillColor, uint3(32, 32, 1))(image);
    blitTexture2D(target, image);
    presentSurface(surface);
    print("presented");
    // Yield so JavaScript can snapshot this frame before the browser expires its canvas image.
    processWindowEvents(window);
    return 1;
}

[host]
bool waitForFrame(Window window)
{
    print("before-frame");
    bool open = processWindowEvents(window);
    print("after-frame");
    return open;
}
