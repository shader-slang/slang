## Display the result on a canvas

The page supplies a `Window` to `main`. The bundled runtime creates a presentation
surface, and the acquired image reports the canvas's current pixel dimensions.
This example draws once and returns. Resizing afterward clears the image; press ![Run](images/snippet-run.svg "Run")
to render again. WebGPU uses an unorm surface.

<!-- runnable: canvas-gradient canvas=16:9 -->

```slang
import slang.runtime;

[host]
void main(Window w)
{
    let surface = createSurface<TextureFormat::rgba8Unorm>(w);
    let presented = acquireSurfaceTexture(surface);
    uint width, height;
    presented.GetDimensions(width, height);
    let size = uint2(width, height);
    let image = RWTexture2D<float4>::alloc(width, height);
    Grid(size).dispatch<8, 8>((uint2 p) =>
    {
        float2 uv = (float2(p) + 0.5) / float2(size);
        image[p] = float4(uv.x, uv.y, 0.35, 1.0);
    });
    blitTexture2D(presented, image);
    presentSurface(surface);
}
```

### Continuous drawing and input

`processWindowEvents` waits for a frame and yields to the browser. Poll input and
apply resize events before acquiring the next surface image. Keep acquired images
inside a separate drawing function so their references are released before resizing.
Move the pointer over the canvas to move the highlight. Stop retains the last frame.

<!-- runnable: canvas-interactive canvas=16:9 -->

```slang
import slang.runtime;

[host]
void drawFrame(Surface<TextureFormat::rgba8Unorm> surface,
               float time, float2 position, bool moved)
{
    RenderTarget2D<float4, TextureFormat::rgba8Unorm> presented;
    if (!tryAcquireSurfaceTexture(surface, presented))
        return;
    uint width, height;
    presented.GetDimensions(width, height);
    let size = uint2(width, height);
    let image = RWTexture2D<float4>::alloc(width, height);
    uint hasPointer = moved ? 1u : 0u;
    Grid(size).dispatch<8, 8>((uint2 p) =>
    {
        float2 uv = (float2(p) + 0.5) / float2(size);
        float2 pointer = hasPointer != 0 ? position / float2(size) : float2(0.5);
        float2 delta = uv - pointer;
        float glow = exp(-60.0 * dot(delta, delta));
        float wave = 0.5 + 0.5 * sin(time * 2.0 + uv.x * 8.0);
        image[p] = float4(uv.x + glow, uv.y + glow, wave + glow, 1.0);
    });
    blitTexture2D(presented, image);
    tryPresentSurface(surface);
}

[host]
void main(Window w)
{
    let surface = createSurface<TextureFormat::rgba8Unorm>(w);
    let timer = createTimer();
    float2 position = float2(0.0);
    bool moved = false;
    while (processWindowEvents(w))
    {
        bool resized = false;
        uint2 size = uint2(0u, 0u);
        var event = pollInputEvent(w);
        while (event.type != InputEventType.none)
        {
            if (event.type == InputEventType.mouseMove)
            {
                position = event.pos;
                moved = true;
            }
            if (event.type == InputEventType.windowResize)
            {
                size = event.size;
                resized = true;
            }
            event = pollInputEvent(w);
        }
        if (resized && (any(size == 0) || !resizeSurface(surface, size.x, size.y)))
            continue;
        drawFrame(surface, elapsedSeconds(timer), position, moved);
    }
}
```
