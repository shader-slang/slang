# Python value interop

Build the `_slangvm` target with `SLANG_ENABLE_SLANGVM_PYTHON=ON` and the Python interpreter you
will use to run the example. See [the interop build instructions](../../../source/slangvm/README.md)
for the Python and nanobind CMake settings. Neither SlangPy nor NumPy is required. Only the GPU
example needs a device and driver supported by a backend enabled in your build.

Set `PYTHONPATH` to `<build directory>/<configuration>/python`, then run from the repository root:

```text
python examples/slangvm/py/demo.py
```

For example, with a configured Windows build and its matching Python environment:

```powershell
$env:PYTHONPATH = (Resolve-Path build/windows-vs2022-dev/Release/python).Path
python examples/slangvm/py/demo.py
```

The example first calls scalar functions from `main.slang`. It then creates a `Point` in
`producer.slang`, loads `consumer.slang`, and passes that value between functions. Both programs
import the same declaration from `geometry.slang`. The squared lengths are `25.0`, `52.0`, and
`25.0`: translating the point creates a new value and leaves the original unchanged. The final
representation prints its cached type name after the VM closes.

## Reference-counted objects

```text
python examples/slangvm/py/objects.py
```

`objects.slang` returns a `SharedCounters` struct whose two fields reference the same `Counter`
class instance, then returns a direct handle to that instance. Python passes the opaque values
back to Slang to mutate and read them. The sum is `20`; deleting the struct leaves the direct
handle usable, and a subsequent increment produces `12`. This demo needs no GPU.

## GPU resource values

```text
python examples/slangvm/py/gpu.py --backend vulkan
```

Use `--backend d3d12` on a supported Windows machine, or omit the option to let RHI select its
default device. The Python host creates the device with `slangvm.init({"backend": "default"})`.
Calling `slangvm.init()` without a backend still creates a VM with no device.

`gpu-producer.slang` uses a compute shader to initialize a buffer and returns this type from
`gpu-state.slang`:

```slang
public struct GpuState
{
    public RWStructuredBuffer<uint> values;
    public uint count;
}
```

`gpu.py` receives the struct as a `slangvm.Value` and passes it to `gpu-consumer.slang`. Both
programs import the same type declaration. Copying the struct retains its buffer, so mutating
through the copy is visible through the original. The demo prints sums of `46`, `50`, and `50`,
including after deleting the original Python handle. It then extracts a direct buffer handle,
deletes both structs, and reads back the first element (`11`) through a Slang helper.

Both demos use `del` and `gc.collect()` to demonstrate releasing individual owning handles.
They deliberately keep one handle until after the VM context exits, showing that `close()`
invalidates outstanding values and releases their native payloads. Printing a cached type name
remains safe after close. These are opaque values; Python does not inspect fields or map buffers.

## Validation

Run focused Python validation with:

```text
ctest --test-dir <build directory> -C <configuration> -R slangvm-python --output-on-failure
```

GPU tests are skipped by default. To include them, set a backend before running the same command:

```powershell
$env:SLANGVM_TEST_BACKEND = "vulkan"
```

On Linux/macOS, use `export SLANGVM_TEST_BACKEND=vulkan` (or another supported backend). Selecting
a backend explicitly makes unavailable-device errors test failures.

For the matching native invocation checks, build `slang-test`, `test-server`, and
`gfx-unit-test`, then run the test executable from the repository root:

```text
<build directory>/<configuration>/bin/slang-test -use-test-server -server-count 1 gfx-unit-test-tool/hgvmScalarInvocation gfx-unit-test-tool/hgvmInvocation gfx-unit-test-tool/hgvmOwned
```

Keep test runs headless and do not run concurrent `slang-test` processes. These commands describe
how to validate the example; their results are recorded separately when the implementation is tested.
