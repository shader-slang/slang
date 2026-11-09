# Python interop

`slangvm` provides synchronous, headless calls to concrete Slang `[host]` functions. It shares
HGVM's C++ invocation API with `slangi` and requires no GPU device for ordinary CPU values.

## Build and run

Enable `SLANG_ENABLE_SLANGVM_PYTHON` in an ordinary Slang CMake configuration. Select a Python
interpreter with development headers using `Python_EXECUTABLE`. Supply either an installed
nanobind CMake package (`nanobind_DIR`) or a nanobind source checkout
(`SLANG_NANOBIND_SOURCE_DIR`). Neither SlangPy nor NumPy is a runtime dependency.

For example, add these options to the platform's usual configure command:

```text
-DSLANG_ENABLE_SLANGVM_PYTHON=ON
-DPython_EXECUTABLE=<python executable>
-DSLANG_NANOBIND_SOURCE_DIR=<nanobind checkout>
```

Build `_slangvm`, then add `<build directory>/<configuration>/python` to `PYTHONPATH`.
On Windows, the target copies its linked runtime DLLs and the versioned standard-module
directory beside the extension. Keep them together when relocating the package. The compiler,
runtime, and modules must come from the same build. `import slang.runtime;`, `slang.tensor`,
and the buffer/texture utility imports resolve without an additional checkout or search path.
Run `examples/slangvm/py/demo.py` using the selected Python:

```python
import slangvm

with slangvm.init({"search_paths": ["./shaders"]}) as vm:
    m = vm.load("main.slang")
    print(m.add(2, 3))
```

Run `ctest --test-dir <build directory> -C <configuration> -R slangvm-python --output-on-failure`
for the Python integration tests. Native invocation tests have names beginning with
`hgvmScalarInvocation`, `hgvmInvocation`, and `hgvmOwned` in `gfx-unit-test`.

## API and conversion rules

`init(settings=None)` creates an independent VM. Settings support `search_paths`,
`working_directory`, and `backend`; paths are resolved when initializing the VM. `load(path)` compiles all
concrete `[host]` entry points in that source module without executing them. Modules and their
functions retain their native VM. `m.function(...)` and `m["function"](...)` access the same cached
callable; `dir(m)` lists exports. `close()` is idempotent and invalidates subsequent calls.

Calls take positional scalar arguments and opaque `slangvm.Value` objects, and return Python
scalars, opaque native values, or `None` for void. Integer
parameters accept Python integers with checked signed/unsigned 32-bit ranges. Boolean parameters
accept only Python booleans. Floating-point parameters accept Python integers or floats and round
to the declared half, float, or double representation using HGVM's scalar conversion helpers.
Booleans are rejected for numeric parameters. Finite floating-point overflow raises
`OverflowError`; NaNs, infinities, and signed zeros are preserved where representable.

Wrong argument types/counts raise `TypeError`; numeric overflow raises `OverflowError`. Compiler
and VM diagnostics become `RuntimeError`. Argument validation completes before VM execution.
Runtime errors unwind the VM invocation, allowing subsequent calls to reuse the interpreter.

Opaque values support structs, classes, strings, and runtime arrays, including nested aggregates
and fixed arrays containing owning references. Results expose `type_name` and `repr`, and can be
passed to a function expecting the same nominal type in another loaded program. Both programs
must use the same declaration;
equal field lists or display names do not make independent structs interchangeable. Values
from another VM are rejected. The Point example in `examples/slangvm/py/demo.py` demonstrates this:

```python
producer = vm.load("producer.slang")
p = producer.makePoint(3.0, 4.0)
consumer = vm.load("consumer.slang")
assert consumer.lengthSquared(p) == 25.0
q = consumer.translate(p, 1.0, 2.0)
assert consumer.lengthSquared(q) == 52.0
```

Values are opaque handles; Python constructors and field access are not exposed. Passing a value
copies its inline fields and retains its reference-counted fields. Referenced objects keep their
identity, so Slang functions can mutate a shared class instance through another returned handle.
Strings remain Slang values rather than converting to Python strings. Deleting the last Python
handle releases its native owning copy; another value that references the same object keeps it alive.
Dropping a module does not invalidate its values while the VM is open. `close()` invalidates
all values and immediately clears their native payloads, including values retained by Python;
cached type names and `repr` remain safe after close. Loaded programs are retained until close.

Set `backend` to create an RHI device for GPU operations. The default setting, `"none"`, creates
no device; `"default"` delegates device selection to RHI. Explicit names are `"vulkan"`, `"d3d12"`,
`"d3d11"`, `"metal"`, `"cuda"`, `"webgpu"`, and `"cpu"`. A selected backend must be enabled in
the build and available on the host; initialization raises `RuntimeError` if device creation fails.
HGVM retains the device and drains pending GPU work before releasing it on close. Calls remain
synchronous, though submitted GPU work may complete later; Slang readback helpers wait for results.

Owning VM objects, including buffers, textures, and acceleration structures, can cross the boundary
directly or inside structs, using the same `Value` ownership and cross-program type checks as CPU
objects. For example, using
the sources in `examples/slangvm/py`:

```python
with slangvm.init({"backend": "default", "working_directory": "examples/slangvm/py"}) as vm:
    producer = vm.load("gpu-producer.slang")
    state = producer.makeState(4, 10)
    consumer = vm.load("gpu-consumer.slang")
    print(state.type_name, consumer.sum(state))  # GpuState, 46
    consumer.add(state, 1)
    print(consumer.sum(state))  # 50
    del state  # Releases the Python handle's owning copy.
```

See [the Python demos](../../examples/slangvm/py/README.md) for shared class references and GPU
buffer aliases, collection, and close invalidation. There is no conversion to a Python GPU or
tensor library. Resource operations still require support from the selected backend.

GPU integration tests are opt-in: set `SLANGVM_TEST_BACKEND=vulkan` (or another available backend)
before running the Python tests above. Without that variable, GPU tests are skipped and ordinary
CPU tests still run. An explicitly selected unavailable backend fails the tests rather than skipping.

The API does not bind borrowed pointers,
`out`/`inout`/`ref` parameters, generic functions, keyword arguments, or overloaded free-function
names. It has no window host. Calls retain the Python GIL. Source modules are
compiler snapshots: create a new VM
to reload changed sources. HGBC exports are the available compiled API, not complete source
reflection of unused public declarations.

## Shared C++ invocation

For a complete buildable host, see the [C++ hello-world example](../../examples/slangvm/cpp/README.md),
including its source, CMake link dependencies, and build/run commands.

Include `slang-hgvm/slang-hgvm.h` and link `slang-hgvm` and `core`. Source compilation is a separate
reusable `slang-hgvm-compiler` target, with `HGBCCompiler` declared in
`slang-hgvm/slang-hgvm-compiler.h`. Pass a null entry-point name to compile all host entry points,
or a name to compile the command-line tool's selected entry point. Load the resulting blob with
`Program::load`. This validates and owns the bytecode independently of any VM. Call
`Interpreter::prepareProgram` to stage it into a particular VM's shared address space, as
Python's `vm.load()` does. Binding reuses this prepared program, or stages it lazily if needed.

For a loaded `Program` containing `[host] int add(int a, int b)`, bind once and invoke:

```cpp
using namespace Slang;
using namespace Slang::HGVM;

const auto* function = program->findFunctionExport("add");
String error;
Interpreter interpreter;
uint64_t programId = 0;
SLANG_RETURN_ON_FAIL(interpreter.prepareProgram(program, programId, error));
RefPtr<BoundFunction> add;
SLANG_RETURN_ON_FAIL(interpreter.bindFunction(program, function->id, add, error));
Argument arguments[] = {Argument::scalar(int32_t(2)), Argument::scalar(int32_t(3))};
Value result;
SLANG_RETURN_ON_FAIL((*add)(makeArrayView(arguments), result, error));
int32_t number = 0;
SLANG_RETURN_ON_FAIL(result.getScalarData(ScalarType::Int32, &number, sizeof(number), error));
// number == 5
```

`BoundFunction::operator()` dispatches to its owning VM's existing invocation path. Because `add`
is a `RefPtr<BoundFunction>`, dereference it to call the function. The explicit
`interpreter.invoke(add, makeArrayView(arguments), result, error)` form remains available. Both return a
`SlangResult` and report diagnostics through `error`; they do not throw ordinary call errors.
Bindings do not keep the VM alive, and calls fail safely after VM close or destruction.

Pass an aggregate result into another bound function with `Argument::fromValue(result)`.
Native scalar arguments have explicit scalar kinds; C++ invocation performs no implicit numeric
conversion. `Argument::fromScalar` also accepts canonical half bits with `ScalarType::Float16`.
Function/type IDs belong to a source `Program`; bound functions resolve them to canonical
VM addresses once. Their cached host metadata is available through `getParameterTypes()` and
`getResultType()`.

The scalar compatibility overload using `ValueView` and `MutableValueView` borrows caller storage
only during synchronous invocation and adapts to the same bound invocation path.
Arguments are copied before execution, so input/result buffers can alias. Result bytes are
published only after success. The VM uses the same parameter transfers, return destinations, and
frame cleanup for external and internal calls. `Interpreter::call(program, name, error)` delegates
to `invoke` with no arguments and discards an owned result; `slangi` and the debugger retain
that convenience path. Calls on one interpreter must be serialized, and reentrant execution is
rejected. An owned output may also be an input; failure leaves it unchanged. `close(error)`
invalidates all bound functions and values, while metadata remains available. Copies of a C++
`Value` share native storage; destroying the last copy releases its managed fields. Borrowed
references and raw aggregate buffers are excluded from the external boundary.
