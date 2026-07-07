# slang-wasm-wasi

A WebAssembly build of the Slang shader compiler, callable from any
WASI-compatible runtime.

The module is a plain WASI reactor exporting a flat C ABI.
This is currently a WASIp1 project.

---

## Consuming the module

The consumer has to do the following:

1. Parse and instantiate the module, providing WASI Preview 1 imports.
2. Call the exported `_initialize` function once, before any other export — this is the standard
   WASI **reactor** convention.
3. Write UTF-8 input (module names, source text, …) into the module's linear memory via
   `slang_wasm_alloc` the memory object the runtime exposes, then call whichever C ABI export is
   needed, reading results back out of linear memory the same way.

## Example consumer: Java via Endive

The [slang-wasm-endive](https://github.com/RefuX/slang-wasm-endive) repository is an implementation that demonstrates how to
instantiate the WASI module, call the flat C ABI, and manage memory across the host/module boundary.

### Running the example's tests

The example's tests cover
compilation, the builder/CompileRequest API, modules, specialization, type conformance, typed and
declaration reflection, structured diagnostics, and disassembly.

```bash
# From a clone of https://github.com/RefuX/slang-wasm-endive
./gradlew test
```

## Example consumer: Python via Wasmtime

For a smaller low-level reference, see [smoke-test.py](../../tests/wasm-wasi/smoke/smoke-test.py).
It is the CI smoke test rather than a reusable binding, but it shows the raw host-side mechanics:
instantiating the module with the Python Wasmtime bindings, calling `_initialize`, using the exported
allocator, passing `(ptr, len)` strings, reading result buffers, resolving enum values through
`slang_wasm_enum_metadata_ptr/len`, and destroying handles.

---

## Building the WASM module

See [Building Slang From Source § WASI-SDK Build](../../docs/building.md#wasi-sdk-build-slang-wasm-wasi)
for the canonical prerequisites and build steps.

One detail specific to this module: the C++ metadata blob is a build-time custom command output, not
committed to the repository (it is gitignored). `cmake --build` automatically (re)runs the enum
binding generator whenever [include/slang.h](../../include/slang.h) or the generator script itself
is newer than the generated file, or the generated file doesn't exist at all yet — no separate
`cmake` reconfigure step is needed.

### Enum binding generator

[generate-slang-bindings.py](tools/generate-slang-bindings.py) reads `include/slang.h` and emits the following artefact:

| Output                              | Purpose                                                                                                                                                                                                                                  |
| ----------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `slang-wasm-wasi-enum-generated.cpp` | C++ static JSON blob baked into the WASM module, exported via `slang_wasm_enum_metadata_ptr/len`. Dynamic-language runtimes (Go, Python, Rust) call these two exports at startup to resolve enum integer values without hardcoding them. |

Options:

| Option               | Default                                                    | Purpose                                                                              |
| -------------------- | ---------------------------------------------------------- | ------------------------------------------------------------------------------------ |
| `--java-out DIR`     | unset                                                      | Output directory for generated Java enum source files. Omit to skip Java generation. |
| `--java-package PKG` | `org.shaderslang.wasm.enums`                               | Java package declared in each generated enum source file.                            |
| `--slang-h PATH`     | `include/slang.h`                                          | Input Slang public header to parse for enum values.                                  |
| `--cpp-out PATH`     | `source/slang-wasm-wasi/slang-wasm-wasi-enum-generated.cpp` | Output path for the C++ enum metadata translation unit.                              |

### Consuming the enum metadata at runtime

Rather than hardcoding Slang's enum integer values (which can shift as `slang.h` evolves), a
consumer reads them out of the module itself:

1. Call `slang_wasm_enum_metadata_ptr` and `slang_wasm_enum_metadata_len` to get the `(ptr, len)`
   of a JSON blob living in the module's linear memory.
2. Read those bytes out of memory and parse them as UTF-8 JSON. The top-level object maps each enum
   name (`Target`, `Stage`, `CompilerOptionName`, …) to an object of `{ "MEMBER_NAME": intValue }`.
   One entry, `TargetFlags`, is not a plain discriminant like the others but a set of OR-able bit
   flags (e.g. `DUMP_IR: 512`, `GENERATE_SPIRV_DIRECTLY: 1024`) — combine them with bitwise-OR
   rather than treating them as mutually exclusive.
3. Use the parsed values instead of literal integers, e.g. to interpret the return value of
   `slang_wasm_target_from_string` / `slang_wasm_stage_from_string`, or to populate a target list
   via `slang_wasm_target_list_add`.

This path is for dynamic-language consumers that resolve enum values at runtime. Java consumers get
a build-time alternative instead: passing `--java-out` to the generator (see above) emits fully
typed Java `enum` classes generated directly from `slang.h`, so a Java consumer such as
`slang-wasm-endive` never needs to call
`slang_wasm_enum_metadata_ptr/len` or parse JSON at all.

The blob is regenerated from `include/slang.h` whenever it changes (see the CMake step above), so
its exact set of top-level keys and members can grow or change across Slang versions.

It follows the same pattern as `smoke-test.py`: after calling `_initialize`, it resolves the
metadata once, then passes the parsed dictionary into each later test:

```python
# Resolve enum values from the module's own metadata rather than hardcoding them,
# so this test tracks slang.h automatically.
meta_ptr = abi.call("slang_wasm_enum_metadata_ptr")
meta_len = abi.call("slang_wasm_enum_metadata_len")
metadata = json.loads(abi.read(meta_ptr, meta_len).decode("utf-8"))
```

It then checks enum-name resolvers against that dict rather than a literal value:

```python
spirv_ptr, spirv_len = abi.alloc(b"spirv")
check(
    abi.call("slang_wasm_target_from_string", spirv_ptr, spirv_len) == metadata["Target"]["SPIRV"],
    "target_from_string('spirv') mismatch",
)
abi.free(spirv_ptr)
```

Because the blob is a static string baked into the module's data section, no allocation or
`slang_wasm_free` call is needed for `meta_ptr` — it stays valid for the lifetime of the module.

## C ABI reference

The authoritative ABI reference is [slang-wasm-wasi.h](slang-wasm-wasi.h). That header documents the current function signatures, ownership rules, handle lifetimes, diagnostics behavior, and result-buffer accessors.

At a high level, the ABI covers:

- Memory allocation helpers
- Enum metadata and enum-name resolvers
- Session creation/destruction and session-descriptor builders
- Module loading, entry-point enumeration, serialization, and destruction
- Compilation entry points
- Specialization arguments
- Type conformance (explicit interface dynamic-dispatch control)
- Declaration reflection and disassembly
- Result accessors and result destruction
- Build/version information
