# Browser JavaScript example

This example imports Slang's `slangvm` package, compiles `.slang` source files in the browser, calls
scalar functions, and passes an opaque `Point` between independently loaded programs. The GPU
example passes a struct containing a structured buffer between programs, then the Canvas runs
[[SIG2014] - Total Noob](https://www.shadertoy.com/view/XdlSDs) by dynamite.
A compute shader writes each pixel into an image, then HGVM's existing texture blitter presents
that image to the Canvas.

1. Build the `emscripten-slangvm` preset as described in
   [the package README](../../../javascript/slangvm/README.md).
2. Serve the repository root:

   ```sh
   python -m http.server 8000
   ```

3. Open `http://localhost:8000/examples/slangvm/js/` in a browser with JSPI support. The scalar and
   Point examples run automatically. Click **Run Shadertoy shader** for the GPU value and Canvas
   examples, then **Stop** or
   press Escape while the Canvas is focused to stop the frame loop.

The import map in `index.html` points to the generated package at
`build.em/Release/slangvm/src/index.js`. Change that mapping if the package is served elsewhere.
No application-side WASM helpers or Slang compiler glue are needed. `demo.js` contains the host
setup; `viewer.slang` imports the package-owned `slangvm_window` helpers and the shader in
`total-noob.slang`.

`gpu-state.slang` declares the value passed through JavaScript:

```slang
public struct GpuState
{
    public RWStructuredBuffer<uint> values;
    public uint count;
}
```

`gpu-producer.slang` allocates the buffer and fills it with a compute shader. JavaScript receives
an opaque `slangvm.Value` and passes it to `gpu-consumer.slang`. Copying the struct retains the
same buffer: an update through the copy is visible through the original. The example prints
sums of 46, 50, and 50, including a read after disposing the original handle. Slang's `sum` helper
does the readback; JavaScript does not inspect the resource or marshal its bytes.

The copy stays alive during the Canvas example. Stopping the example closes the VM, releases the
buffer, and prints the surviving handle's cached type name with `valid = false`. Disposing that
invalidated handle afterward is safe. Applications can instead dispose all values before closing.

The compute image matches the Canvas backing dimensions and is reused until its size changes.
The shader receives bottom-left pixel-center coordinates, the actual resolution, and elapsed
seconds from HGVM's timer. Animation continues until the window closes. It needs no texture
channels or other assets. Only presentation uses a raster blit; the shader itself runs as compute.
Its `RWTexture2D<float4>` image uses `RGBA32Float` storage and currently requires the adapter's
`float32-filterable` feature because of RHI's sampled-texture layout.

Calls start executing immediately and return Promises. Await a call before issuing another call
that would overlap its suspension on the same VM. The example explicitly closes VMs and disposes
its opaque values. Canvas backing dimensions are managed by the page's `ResizeObserver`.

`total-noob.slang` is adapted from the
[Slang Playground port](https://github.com/shader-slang/slang-playground/blob/main/public/demos/circle.slang).
The shader retains dynamite's [CC BY-NC-SA 3.0 license](https://creativecommons.org/licenses/by-nc-sa/3.0/).
This version uses elapsed time, pixel-center coordinates, and GLSL's positive modulo semantics.
