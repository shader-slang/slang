# C++ hello world

[hello-world.cpp](hello-world.cpp) is a complete native host for
[hello-world.slang](hello-world.slang). It creates a compiler session, compiles `[host] main` to
HGBC, loads and stages the bytecode, binds the function, calls it, and closes the VM. Compiler diagnostics
and runtime failures are reported to stderr with a nonzero exit status. Slang's `print` writes:

```text
Hello from HGVM
```

The example runs on the CPU without creating a GPU device or window. Python and nanobind are not
required. HGVM currently depends on the `slang-rhi` library at build time, so configure Slang with
`SLANG_ENABLE_EXAMPLES=ON` and `SLANG_ENABLE_SLANG_RHI=ON`.

From the repository root, using a source checkout with its submodules initialized:

```sh
cmake --preset default -DSLANG_ENABLE_EXAMPLES=ON -DSLANG_ENABLE_SLANG_RHI=ON
cmake --build build --config Release --target slangvm-hello-world
./build/Release/bin/slangvm-hello-world examples/slangvm/cpp/hello-world.slang
```

On Windows with Visual Studio 2022, the equivalent PowerShell commands are:

```powershell
cmake --preset vs2022-dev -DSLANG_ENABLE_EXAMPLES=ON -DSLANG_ENABLE_SLANG_RHI=ON
cmake --build build/windows-vs2022-dev --config Release --target slangvm-hello-world
./build/windows-vs2022-dev/Release/bin/slangvm-hello-world.exe examples/slangvm/cpp/hello-world.slang
```

For an existing build, use its build directory in the build and run commands. The executable takes
the Slang source path as its only argument; pass an absolute path when running from another
directory. See [Building Slang](../../../docs/building.md) for platform prerequisites and presets.

## Include and link HGVM in a C++ host

The source includes these two headers:

```cpp
#include "slang-hgvm/slang-hgvm-compiler.h"
#include "slang-hgvm/slang-hgvm.h"
```

[CMakeLists.txt](CMakeLists.txt) registers the executable with the repository's `slang_add_target`
helper. The essential link dependencies, expressed with ordinary CMake, are:

```cmake
target_link_libraries(my-host PRIVATE core slang-hgvm slang-hgvm-compiler)
```

Use this after defining your executable in a CMake build that includes the Slang source targets.
`core` provides the native strings, containers, and reference counting, including the `source/`
include path. `slang-hgvm` provides `Program`, `Interpreter`, and the invocation/value API.
`slang-hgvm-compiler` provides source-to-HGBC compilation and brings in the Slang compiler.
CMake propagates the headers, compile definitions, and remaining link dependencies. A host that
loads precompiled HGBC can omit `slang-hgvm-compiler` and its header.

These are currently source-tree C++ interfaces, not an installed `find_package(slang)` HGVM SDK.
Build the host and libraries together with matching compiler/ABI settings. The example target
uses the normal Slang binary directory so its shared libraries are available at runtime. If you
relocate the executable, also make its dependent shared libraries available to the OS loader.

`Program::load` validates and owns VM-independent bytecode. `vm.prepareProgram` stages that program
into this VM's shared address space; Python's `vm.load` performs both steps. Binding reuses the
prepared program. If preparation was omitted, binding stages it lazily for convenience.

The bound function is callable: `(*main)({}, result, error)` invokes it on its owning VM and
returns a `SlangResult`. The dereference is needed because `main` is a `RefPtr<BoundFunction>`.
It uses the same execution path as `vm.invoke(main, {}, result, error)` and rejects calls after
VM close or destruction. For a parameterless entry point whose result you want to discard,
`vm.call(program, "main", error)` is
the convenience form. See [the native invocation documentation](../../../source/slangvm/README.md#shared-c-invocation)
for scalar arguments and passing owned aggregate values between functions.
