# Invoking Slang programs and methods

Save this as `main.slang`. It needs no imports and runs entirely on the CPU.

<!-- runnable: first-program backend=none -->

```slang
[host]
public int add(int a, int b)
{
    return a + b;
}

[host]
public void main()
{
    print(add(2, 3)); // 5
}
```

## Command line: slangi

The `slangi` tool provides a convenient way to run `h-slang` applications in a standalone manner without needing to set up a full project or build system.

It can simply be invoked as:

```sh
slangi main.slang
```

`slangi` compiles and invokes `[host] main()` by default. For a different no-argument entry,
use `-entry entryName`.

`slangi` can also run precompiled bytecode, which skips the front-end compilation step for faster load times.

```sh
slangc main.slang -entry main -target hgbc -o main.hgbc
slangi main.hgbc
```

`slangi` enables Slang's `-experimental-feature` flag by default. When compiling programs directly
with `slangc`, manually pass `-experimental-feature` for packages that import experimental modules.

The `-api` option selects the backend. Supported backends for `slangi` are `cuda`, `d3d12`, and `vulkan`, though `cuda` doesn't currently have a window provider, so it's better for headless invocations (such as training/fitting).

## Python: slangvm

After [building the Python package](interoperability.md#python), call concrete `[host]`
exports directly:

```python
import slangvm

with slangvm.init({"search_paths": ["."]}) as vm:
    module = vm.load("main.slang")
    print(module.add(2, 3))  # 5
```

## C++: native Slang VM

Link the native runtime and compiler libraries as shown in the
[C++ setup](interoperability.md#c). After loading and binding `add`:

```cpp
Argument args[] = {Argument::scalar(int32_t(2)), Argument::scalar(int32_t(3))};
Value result;
SLANG_RETURN_ON_FAIL(vm.invoke(add, makeArrayView(args), result, error));
int32_t sum = 0;
SLANG_RETURN_ON_FAIL(result.getScalarData(ScalarType::Int32, &sum, sizeof(sum), error));
// sum == 5
```

The [complete load/bind sequence](interoperability.md#c) supplies `vm`, `add`, and `error`.
The current native CMake targets are `slang-hgvm`, `slang-hgvm-compiler`, and `core`.

## JavaScript: slangvm

Use the [browser package built from this branch](interoperability.md#javascript):

```javascript
import slangvm from "slangvm";

const vm = await slangvm.init({ baseURL: new URL("./", import.meta.url) });
try {
  const module = await vm.load("main.slang");
  console.log(await module.add(2, 3)); // 5
} finally {
  await vm.close();
}
```

Module loading finds exports; it does not run `main`. Call `module.main()` explicitly
when needed (and `await` it in JavaScript).

Next: [allocate resources in host functions](host-functions.md) and
[launch compute kernels](grid-dispatch.md).
