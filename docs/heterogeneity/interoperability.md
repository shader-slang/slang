# Interoperability

Load a Slang module, look up a concrete `[host]` function, and invoke it with typed
arguments. Scalar results convert to host values; aggregates and owned references travel
as opaque VM values.

## Setting up

### Python

The `slangvm` package is built from this checkout. Enable it in your native CMake build
using the Python interpreter you will run with and a nanobind source checkout:

```sh
cmake --preset default -DSLANG_ENABLE_SLANGVM_PYTHON=ON -DPython_EXECUTABLE=/path/to/python -DSLANG_NANOBIND_SOURCE_DIR=/path/to/nanobind
cmake --build --preset releaseWithDebugInfo --target _slangvm
```

For Visual Studio, substitute the presets from [installation](installation.md).
Add `<build-directory>/RelWithDebInfo/python` to `PYTHONPATH`:

```powershell
$env:PYTHONPATH = "C:/path/to/slang/build/windows-vs2022-dev/RelWithDebInfo/python"
```

```sh
export PYTHONPATH=/path/to/slang/build/RelWithDebInfo/python
```

Use the same Python ABI as the build. No SlangPy or NumPy dependency is required.
See the [binding build notes](../../source/slangvm/README.md).

With `main.slang` from [your first program](invoking.md):

```python
import slangvm

with slangvm.init({"search_paths": ["./shaders"], "working_directory": "."}) as vm:
    module = vm.load("main.slang")
    print(module.add(2, 3))  # 5
```

Python calls are synchronous. The initializer creates no device by default. Add
`"backend": "default"` to let RHI choose a device, or select a backend such as `"vulkan"` or `"d3d12"` for GPU
operations. The backend must be enabled in the build and supported by the host's device and driver.

### C++

The native implementation behind `slangvm` is currently exposed through these in-tree
CMake targets (there is no separate installed `slangvm` C++ SDK target):

```cmake
target_link_libraries(my_app PRIVATE core slang-hgvm slang-hgvm-compiler)
```

This function compiles all concrete host exports, loads the program, binds `add`, invokes
it, and extracts its integer result. Call it with the path to the first program's `main.slang`:

```cpp
#include "slang-hgvm/slang-hgvm-compiler.h"
#include "slang-hgvm/slang-hgvm.h"
#include <cstdio>

using namespace Slang;
using namespace Slang::HGVM;

SlangResult run(const char* path, int32_t& sum, String& error)
{
    ComPtr<slang::IGlobalSession> session;
    slang_createGlobalSession(SLANG_API_VERSION, session.writeRef());
    HGBCCompiler compiler(session);
    ComPtr<slang::IBlob> code;
    String diagnostics;
    SlangResult compiled = compiler.compile(
        path, nullptr, nullptr, 0, SLANG_DEBUG_INFO_LEVEL_NONE,
        SLANG_DIAGNOSTIC_COLOR_NEVER, code, diagnostics);
    if (diagnostics.getLength())
        std::fprintf(stderr, "%s\n", diagnostics.getBuffer());

    RefPtr<Program> program;
    Program::load(
        code->getBufferPointer(), code->getBufferSize(), path, program, error);

    Interpreter vm;
    const auto* exported = program->findFunctionExport("add");
    RefPtr<BoundFunction> add;
    vm.bindFunction(program, exported->id, add, error);

    Argument args[] = {Argument::scalar(int32_t(2)), Argument::scalar(int32_t(3))};
    Value result;
    vm.invoke(add, makeArrayView(args), result, error);

    result.getScalarData(ScalarType::Int32, &sum, sizeof(sum), error);
    return vm.close(error);
}
```

The caller receives `sum == 5`. Reuse the VM, program, and bound functions for repeated
calls. `Interpreter` destruction also closes it. Scalar argument types must match exactly.
See the [buildable C++ example](../../examples/slangvm/cpp/README.md).

### JavaScript

Build the browser package with Emscripten, using native generators from the same revision:

```sh
cmake --build --preset release --target all-generators
cmake --install build --config Release --prefix build/native-generators --component generators
emcmake cmake --preset emscripten -G "Ninja Multi-Config" -DSLANG_GENERATORS_PATH=/path/to/slang/build/native-generators/bin
cmake --build --preset emscripten-slangvm
```

Follow the [WASM build instructions](../../javascript/slangvm/README.md)
for generator setup. Serve the generated `build.em/Release/slangvm` package over HTTP;
map `slangvm` to its `src/index.js` using an import map or your bundler. Keep the package's
WASM and Slang files together. Use a browser with WebAssembly JSPI support.

```html
<script type="importmap">
  { "imports": { "slangvm": "/build.em/Release/slangvm/src/index.js" } }
</script>
<script type="module">
  import slangvm from "slangvm";
  const vm = await slangvm.init({ baseURL: new URL("./", location.href) });
  try {
    const module = await vm.load("main.slang");
    console.log(await module.add(2, 3)); // 5
  } finally {
    await vm.close();
  }
</script>
```

`baseURL` locates your application's `.slang` files. Initialization, module loading,
calls, and closing return promises. For GPU buffers, initialize with `backend: "webgpu"`
on HTTPS or localhost with WebGPU support. See the
[browser example](../../examples/slangvm/js/README.md).

## Invocation arguments & return values

Presently, `slangvm` supports marshalling Slang's basic types to/from host language basic types:

```slang
public float add(bool a, uint b, double c) { return float(a) + float(b) + float(c); }
```

```py
import slangvm
vm = slangvm.init({"search_paths": ["."]})
module = vm.load("add.slang")
print(module.add(True, 2, 3.0)) # 6.0
```

`slangvm` also supports moving/copying opaque Slang types such as structs, classes, runtime arrays, even those containing GPU resource allocations. `slangvm` does not automatically convert them to host language types, but they can be passed back to functions/methods that operate on them.

### Example: Pass custom `Point` type

```slang
public struct Point { public float x; public float y; }

[host]
public Point makePoint(float x, float y) { return Point(x, y); }

[host]
public float lengthSquared(Point p) { return p.x * p.x + p.y * p.y; }
```

```python
with slangvm.init({"search_paths": ["."]}) as vm:
    points = vm.load("points.slang")

    # methods & types are available on the module object as attributes.
    p = points.makePoint(3.0, 4.0)  # returns opaque slangvm.Value

    print(points.lengthSquared(p)) # 25.0; type-checked call.
```

```javascript
const points = await vm.load("points.slang");
const p = await points.makePoint(3, 4);
try {
  console.log(await points.lengthSquared(p)); // 25
} finally {
  p.dispose();
}
```

#### Example: Objects containing reference-counted fields.

Reference-counted GPU resources, such as textures, are managed by the VM. When returned from an invocation, `slangvm` wraps them in ref-counted host objects. This allows the actual object to stay in the VM's memory as long as references to it exist (even if they are from outside Slang)

```slang
public struct AccumulatedTexture
{
    public RWTexture2D<float4> texture;
    public uint frameCount;
}

[host]
public AccumulatedTexture make()
{
    return AccumulatedTexture(RWTexture2D<float4>::alloc(256, 256), 0);
}

[host]
public AccumulatedTexture addFrame(AccumulatedTexture state, RWTexture2D<float4> frame)
{
    Grid(frame.getSize()).dispatch((float2 uv) => {
        let val = (state.texture[uv] * state.frameCount + frame[uv]) / (state.frameCount + 1);
        state.texture[uv] = val;
    });
    state.frameCount ++;
    return state;
}
```

```python
# `obj.texture` contains reference to VM object
# which in turn references GPU texture.
obj = module.make()

# Pass the object.
obj = module.addFrame(obj, frame)
```

This works the same way in C++ and JavaScript, where the host language objects maintain references to the underlying VM-managed resources and ensure proper release when no longer needed.

## Ownership and lifetime

**VM-owned memory.** CPU and GPU references stay valid while an owning value retains them
and the VM remains open. VM lifetime is the upper bound, not a rule that every allocation
lasts until close. Closing invalidates surviving values; never pass a value to another VM.

**Host wrappers.** Python `slangvm.Value` objects release their native owning copy when the
last Python reference drops. C++ `Value` uses reference-counted storage (with `RefPtr` for
native runtime objects). JavaScript `Value.dispose()` releases promptly; garbage collection
is a fallback. Other aliases and queued GPU work can keep resources alive longer.

**Future tensor interop.** Automatic conversion to host tensor libraries with explicit
ownership transfer is future work.

## Rough edges

**Dynamic-language collection.** JavaScript may keep unreachable wrappers alive until a
later GC. Dispose large resource values explicitly and close the VM when finished. Python
cycles can also delay reclamation.

**Opaque resources.** Owning VM objects and fields use the same reference-counted handles in
Python, C++, and JavaScript. Creating or using a GPU resource still requires a supporting backend;
passing its handle does not convert it to a host-library resource.

**Limited support for parameter passing modes.** Export concrete host functions with supported value parameters.
Borrowed pointers, `out`/`inout`/`ref` parameters.

**Reloading.** A VM uses a source snapshot. Create a fresh VM after changing source files.
The [native/Python API notes](../../source/slangvm/README.md)
and [browser API notes](../../javascript/slangvm/README.md)
track the current supported subset.
