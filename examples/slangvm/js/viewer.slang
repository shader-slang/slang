import slangvm_window;
import total_noob;

[shader("compute")]
[numthreads(8, 8, 1)]
void fillImage(
    WindowImage output,
    uint2 imageSize,
    float time,
    uint3 thread : SV_DispatchThreadID)
{
    if (any(thread.xy >= imageSize))
        return;

    // Shadertoy uses pixel centers measured from the lower-left corner.
    float2 fragCoord = float2(float(thread.x) + 0.5, float(imageSize.y - thread.y) - 0.5);
    output[thread.xy] = totalNoob(fragCoord, float2(imageSize), time);
}

[host]
void main(Window window)
{
    print("Running Shadertoy XdlSDs via compute.");
    WindowSurface surface = createSurface(window);
    Timer timer = createTimer();
    Optional<WindowImage> image = none;
    uint2 imageSize = uint2(0u);
    while (processWindowEvents(window))
    {
        for (;;)
        {
            InputEvent event = pollInputEvent(window);
            if (event.type == InputEventType.none)
                break;
            if (event.type == InputEventType.windowResize)
                resizeSurface(surface, event.size.x, event.size.y);
            if (event.type == InputEventType.keyPress && event.key == 256)
                closeWindow(window);
        }

        WindowTarget target;
        if (tryAcquireSurfaceTexture(surface, target))
        {
            uint2 targetSize;
            target.GetDimensions(targetSize.x, targetSize.y);
            if (any(targetSize == 0u))
                continue;

            // Match the Canvas backing size and retain the texture until the surface changes.
            if (any(targetSize != imageSize))
            {
                image = allocateTexture2D<float4>(targetSize.x, targetSize.y);
                imageSize = targetSize;
            }

            uint2 groupCount = (imageSize + 7u) / 8u;
            dispatch(fillImage, uint3(groupCount, 1))(image.value, imageSize, elapsedSeconds(timer));
            blitTexture2D(target, image.value);
            tryPresentSurface(surface);
        }
    }
}
