# slangvm for browsers

This package combines Slang's compiler and host graph virtual machine in one WebAssembly module.
The JavaScript layer marshals scalars and opaque native values through the same invocation API used
by Python, C++, and `slangi`. It requires WebAssembly JSPI; WebGPU is optional for scalar programs.

```js
import slangvm from "slangvm";
import {
  HTTPSlangFileSystem,
  standardLibraryURL,
} from "slangvm/http-file-system";
import {
  MemorySlangFileSystem,
  OverlaySlangFileSystem,
} from "slangvm/file-system";

const fileSystem = new OverlaySlangFileSystem([
  new HTTPSlangFileSystem(new URL("./", import.meta.url)),
  new HTTPSlangFileSystem(standardLibraryURL),
]);

const vm = await slangvm.init({ fileSystem });
try {
  const m = await vm.load("main.slang");
  console.log(await m.add(2, 3));

  const producer = await vm.load("producer.slang");
  const point = await producer.makePoint(3, 4);
  const consumer = await vm.load("consumer.slang");
  console.log(point.typeName);
  console.log(await consumer.lengthSquared(point));
  point.dispose();
} finally {
  await vm.close();
}
```

## Execution and ownership

- `init`, `load`, `createWindow`, exported Slang calls, and `close` return Promises.
- Calling an exported function immediately enters WASM. Execution continues synchronously until
  the function finishes or a host operation suspends, such as fetching a source file, waiting for a
  frame, or reading back GPU data. There is no automatic instruction-budget yielding.
- A call that completes synchronously still returns a Promise. Calls that overlap a suspended
  operation on the same VM are rejected as busy. Separate VMs have independent state.
- The optional `print` callback receives complete lines synchronously. Returned Promises are not
  awaited; asynchronous output hooks are outside the initial API.
- JavaScript numbers and booleans are checked against the declared Slang scalar parameter types.
  Integers require integral, in-range numbers; floating-point arguments support `half`, `float`,
  and `double`. Native values remain opaque and expose `typeName` and `dispose()`.
- Structs, classes, strings, and runtime arrays can contain owning references. Passing a value
  copies its inline fields and retains its managed fields; class and array aliases continue to
  reference the same objects. Strings remain Slang values rather than converting to JS strings.
  All owning VM objects, including buffers, textures, acceleration structures, streams, and
  boxed values, use this same opaque-handle path. Resource operations require a supporting GPU
  backend; returning a handle does not add backend capabilities. Borrowed addresses and values
  with borrowed fields remain unsupported at this boundary.
- Values can pass between programs loaded by the same VM. Cross-VM values are rejected.
- `close()` invalidates all JavaScript handles immediately, cancels pending file reads for that VM, and wakes frame
  waits. Its Promise resolves after outstanding native execution unwinds and its memory is released.
  A browser GPU operation already in progress may need to finish first.
- Explicit `dispose()` releases values promptly. Garbage collection is a fallback; releases are
  deferred while the VM is executing. Releasing a value drops its owned references; other values
  sharing those objects remain valid. Keep the VM open while using its values.

## Filesystems

`init({ fileSystem })` accepts any object implementing `loadFile(path, { signal })`.
Return a `Uint8Array`, or `null` for a missing file; either may also be returned through a Promise.
Throw or reject for other errors. Returned bytes must not be mutated. This same filesystem backs
compiler imports and runtime file reads, including binary `.slang-module` files.

Paths are virtual, use `/` separators, and are rooted at `/`; they are not URLs. The adapter
normalizes paths and rejects traversal outside the root. `searchPaths` defaults to `["/"]`.
The native adapter implements `ISlangFileSystemExt`, supplies path identities, and retains an
immutable snapshot of successfully loaded bytes per VM. Create a new VM for edited files.
The VM does not add implicit standard-library search paths or fetch program files itself.
`wasmURL` separately selects the WASM asset directory; relative values resolve against the
package's entry module.

`MemorySlangFileSystem` snapshots a record of strings or `Uint8Array` values. Compose it before
an HTTP filesystem with `OverlaySlangFileSystem` to let edited cells override downloaded files.
An overlay falls through only on `null`; network failures remain errors.

```js
const libraries = new HTTPSlangFileSystem(standardLibraryURL);
const vm = await slangvm.init({
  fileSystem: new OverlaySlangFileSystem([
    new MemorySlangFileSystem({ "main.slang": editor.value }),
    libraries,
  ]),
  backend: "webgpu",
  print: (line) => console.log(line),
});
try {
  await (await vm.load("main.slang")).main();
} finally {
  await vm.close();
}
```

The package includes precompiled standard modules and matching sources under `slang/`, alongside
`src/` and `wasm/`. `standardLibraryURL` points there. Execution prefers `.slang-module` files;
language services can use the retained `.slang` sources. Rebuild modules together with WASM using
matching native generators and the core archive; do not mix compiler builds.

`HTTPSlangFileSystem(baseURL, options)` treats the URL as a directory, caches successful reads,
and deduplicates requests already in progress. Cache hits return bytes synchronously, avoiding
unnecessary WASM suspension. A custom `fetch` implementation may be supplied for authentication
or other request configuration. Keep the filesystem alive to reuse downloads across VMs.
Closing one VM cancels its own reads; shared HTTP downloads can finish and populate the cache.
`clearCache()` drops downloaded bytes, without changing files already loaded into a VM.

HTTP 404/410 caching is **off by default**. Set `cacheNotFound: true` to remember misses for that
filesystem's lifetime. Optionally supply `missingFileCache`, an object with synchronous `has(url)`
and `add(url)` methods, to share misses across filesystem instances or forward them to a parent
page. Supplying a cache alone does not enable caching. A `Set` works; clear it or construct a fresh
filesystem to retry previously missing URLs. `clearCache()` only clears successful downloads.
Other HTTP errors, network failures, and cancellation never count as missing files.

The docs explicitly enable `cacheNotFound`, retain misses across replacement workers, and clear
them on page reload. In-memory editor files still take precedence over cached misses.

## Preloading

`preload(paths, { concurrency: 8, signal })` downloads required files into the normal cache.
It rejects when a required file is missing. Directory preloading needs an explicit inventory,
since HTTP servers generally do not expose directory listings:

```js
const libraries = new HTTPSlangFileSystem(standardLibraryURL, {
  cacheNotFound: true,
});
const bytes = await libraries.loadFile("standard-modules.json");
if (bytes === null) throw new Error("Missing standard-module inventory");
const manifest = JSON.parse(new TextDecoder().decode(bytes));
await libraries.preload(["slang/runtime.slang-module"]);
await libraries.preloadDirectory("slang/runtime/", {
  manifest,
  concurrency: 8,
});
```

The runtime entry and subtree currently contain all their imports. For other preload groups,
include dependencies outside the directory as well; directory preloading does not infer imports.
This is a set of parallel HTTP requests, not a single archive download.

`snapshot()` copies resolved HTTP bytes into a record suitable for `postMessage` and
`new MemorySlangFileSystem(snapshot)`. It excludes pending requests and cached misses. Filesystem
objects themselves cannot be sent to workers. The docs retain the preloaded runtime on the page,
then seed each new execution worker from a snapshot. Other imports load on demand.

The former `init({ baseURL, sources, missingFileCache })` options have been replaced by explicit
filesystem composition. Use `HTTPSlangFileSystem` for `baseURL` and HTTP caching, and
`MemorySlangFileSystem` for `sources`.

For interactive compute examples, run this in a module worker so compilation and synchronous
Slang execution do not block the page. A new worker per run allows cancellation with
`worker.terminate()`, including CPU loops that never suspend. Canvas window helpers require
the DOM and are not supported in workers.

Module members are callable exports. Use `m.getFunction("then")` for an export called `then`, which
cannot appear directly as a callable property without making the module a JavaScript thenable.
`getFunction` also accesses exports named `getFunction` or `functionNames`.

## Canvas windows

```js
const vm = await slangvm.init({
  fileSystem,
  backend: "webgpu",
});
try {
  const window = await vm.createWindow(document.querySelector("canvas"));
  const m = await vm.load("viewer.slang");
  await m.main(window);
} finally {
  await vm.close();
}
```

Slang code can `import slang.runtime;` for the standard Window, event, and surface helpers. These
wrap existing HGVM operations; the browser host supplies their implementations. Event polling is
synchronous. `processWindowEvents` waits for the next animation frame and returns false after the
window closes. Calling `window.close()` from JavaScript ends the window's event loop.

Use `createSurface<TextureFormat::rgba8Unorm>(window)` to match WebGPU's canvas formats. Explicit sRGB
surface requests are currently unsupported; adding them requires RHI support for sRGB canvas views.
The example fills a writable image with a compute shader and scales it into the surface through
HGVM's existing texture blitter. RHI's WebGPU backend does not yet implement standalone texture-clear
commands; the package does not expose a clear helper that would depend on those commands.
The compute image uses the existing `RWTexture2D<float4>` path with `RGBA32Float` storage. The current
RHI sampled-texture layout requires a WebGPU adapter with `float32-filterable` for this example.

The embedding page owns Canvas CSS layout and backing resolution (`canvas.width` and `canvas.height`).
Changes to the backing dimensions generate resize events. Pointer positions use backing pixels.
For responsive rendering, supply a synchronous `beforeFrame(canvas)` initialization setting
that updates those dimensions from the desired display size. It runs before resize detection
and before `processWindowEvents` resumes, keeping resize events and the next frame consistent.
Size the canvas before calling the Slang entry point as well, for programs that draw only once.
Returned Promises are not awaited; a thrown error rejects the waiting Slang call.

Each Canvas may have one registered Window at a time across VMs. Disposing the Window releases its
registration once outstanding native references are gone, allowing the Canvas to be adopted again.
The Window retains its underlying presentation surface, so a one-shot Slang function can return
without clearing the displayed canvas. To preserve pixels after closing the VM, supply
`onPresent: canvas => snapshotContext.drawImage(canvas, 0, 0)` in the initialization settings.
This synchronous callback runs while the frame's canvas image is still current; copying later
can produce an empty image. Returned Promises are not awaited. VM/window teardown releases
the presentation surface.
An optional `canvas` setting supplies the Canvas used by the first Slang-side `createWindow` call;
otherwise that operation creates and appends a Canvas.

## Build and package

With Emscripten configured and Slang's native generators available, configure the `emscripten`
preset and build the `emscripten-slangvm` preset. On Windows, explicitly select Ninja Multi-Config
when using `emcmake`, which otherwise supplies its own single-config generator:

```sh
emcmake cmake --preset emscripten -G "Ninja Multi-Config" \
    -DSLANG_GENERATORS_PATH=/absolute/path/to/native/generators/Release/bin
cmake --build --preset emscripten-slangvm
```

The resulting `build.em/Release/slangvm/` directory is a self-contained package containing the ESM
wrapper, declarations, standard modules, and generated WASM artifacts. It can be installed as a local
package by a bundler or mapped directly with an HTML import map. Serve it over HTTP; localhost is
suitable for WebGPU. An optional `wasmURL` setting changes the directory used for `.wasm` loading.
Keep the generated JavaScript, WASM, and package sources from the same build.

`SLANG_GENERATORS_PATH` names the directory containing the native `slang-bootstrap` executable
and the other generator tools. An installed `generators` component uses its installation's `bin`
directory instead of the build-tree path shown above.

See [the browser example](../../examples/slangvm/js/README.md) for a complete page and source files.
The existing compiler-only `slang-wasm` target remains available independently.

## Focused tests

```sh
node --test javascript/slangvm/tests/*.test.js
```

These tests exercise the JavaScript scheduling, ownership, filesystem cache, and DOM adapter with
native mocks. They do not validate WASM compilation or JSPI itself.

After building, serve the repository root and open `javascript/slangvm/tests/browser.html` for
real compiler/VM tests. Add `?gpu=1` to exercise WebGPU dispatch/readback, Canvas presentation,
frame suspension, and closing a suspended VM. The page exposes its machine-readable result as
`globalThis.__slangvmTest`.
