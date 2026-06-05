---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-05T09:24:37Z
source_commit: 52339028a2aa703271533454c6b9528a534bac31
watched_paths_digest: 6dc28f908084269f31c6e55e648ebd8307ae6b527db79dfc00f74b5e82c5c6ed
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Code Emission

This document describes how the legalized IR (output of
[05-ir-passes.md](05-ir-passes.md)) is turned into target code:
HLSL, GLSL, SPIR-V, Metal, WGSL, C++, CUDA, PyTorch glue, LLVM IR,
or VM bytecode. The intended reader is a developer adding or
modifying a target backend.

## Inputs and outputs

- **Input**: a fully linked, specialized, and legalized `IRModule`
  produced by `linkAndOptimizeIR`
  ([slang-emit.cpp](../../../../source/slang/slang-emit.cpp) line 893)
  for one `TargetRequest`.
- **Output**: a target artefact wrapped in an `IArtifact`
  (declared in
  [slang-artifact.h](../../../../source/compiler-core/slang-artifact.h)).
  The artefact carries the textual or binary code together with the
  diagnostic record and any side products (dependency files,
  reflection blobs).

## Emit dispatcher

The dispatch happens in `emitEntryPointsSourceFromIR`
([slang-emit.cpp](../../../../source/slang/slang-emit.cpp) line 2487 at
`source_commit`) plus the variants invoked when targeting non-textual
formats. The dispatcher:

1. Selects the appropriate `CLikeSourceEmitter` subclass (for textual
   targets) or the SPIR-V / LLVM / VM emitter directly.
2. Constructs a `SourceWriter`
   ([slang-emit-source-writer.h](../../../../source/slang/slang-emit-source-writer.h))
   to buffer the emitted text.
3. Walks the IR and lets the backend emit one declaration / function
   at a time.
4. Invokes the precedence helper
   ([slang-emit-precedence.h](../../../../source/slang/slang-emit-precedence.h))
   to insert minimal parentheses.
5. Wraps the result in an `IArtifact` and hands it back.

The set of `#include`s at the top of
[slang-emit.cpp](../../../../source/slang/slang-emit.cpp) is the
authoritative list of which backends are linked into the dispatcher.

## Backends

### HLSL

[slang-emit-hlsl.h](../../../../source/slang/slang-emit-hlsl.h) /
[slang-emit-hlsl.cpp](../../../../source/slang/slang-emit-hlsl.cpp).
Emits HLSL source text. The HLSL prelude is emitted via
[slang-emit-hlsl-prelude.cpp](../../../../source/slang/slang-emit-hlsl-prelude.cpp)
and uses the header
[slang-hlsl-prelude.h](../../../../prelude/slang-hlsl-prelude.h) shipped
under [prelude/](../../../../prelude). HLSL output is typically handed
off to DXC or FXC via the downstream-compiler bridge in
[source/compiler-core/](../../../../source/compiler-core).

### GLSL

[slang-emit-glsl.h](../../../../source/slang/slang-emit-glsl.h) /
[slang-emit-glsl.cpp](../../../../source/slang/slang-emit-glsl.cpp).
Emits GLSL source text. GLSL output is consumed by
`source/slang-glslang/` to produce SPIR-V via Khronos `glslang` when
the user asks for SPIR-V via GLSL.

### SPIR-V (direct)

[slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp).
Emits SPIR-V binary directly without going through GLSL. The opcode
tables are split out into
[slang-emit-spirv-ops.h](../../../../source/slang/slang-emit-spirv-ops.h)
and
[slang-emit-spirv-ops-debug-info-ext.h](../../../../source/slang/slang-emit-spirv-ops-debug-info-ext.h).

### Metal

[slang-emit-metal.h](../../../../source/slang/slang-emit-metal.h) /
[slang-emit-metal.cpp](../../../../source/slang/slang-emit-metal.cpp).
Emits Metal Shading Language. The Metal prelude is in
[slang-emit-metal-prelude.cpp](../../../../source/slang/slang-emit-metal-prelude.cpp).

### WGSL

[slang-emit-wgsl.h](../../../../source/slang/slang-emit-wgsl.h) /
[slang-emit-wgsl.cpp](../../../../source/slang/slang-emit-wgsl.cpp).
Emits WGSL.

### C++

[slang-emit-cpp.h](../../../../source/slang/slang-emit-cpp.h) /
[slang-emit-cpp.cpp](../../../../source/slang/slang-emit-cpp.cpp).
Emits C++ source, paired with the preludes
[slang-cpp-prelude.h](../../../../prelude/slang-cpp-prelude.h),
[slang-cpp-types-core.h](../../../../prelude/slang-cpp-types-core.h),
[slang-cpp-types.h](../../../../prelude/slang-cpp-types.h),
[slang-cpp-host-prelude.h](../../../../prelude/slang-cpp-host-prelude.h),
and
[slang-cpp-scalar-intrinsics.h](../../../../prelude/slang-cpp-scalar-intrinsics.h).
The runtime support that emitted C++ links against lives in
[source/slang-rt/](../../../../source/slang-rt).

### CUDA

[slang-emit-cuda.h](../../../../source/slang/slang-emit-cuda.h) /
[slang-emit-cuda.cpp](../../../../source/slang/slang-emit-cuda.cpp).
Emits CUDA source. Prelude:
[slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h).

### Torch

[slang-emit-torch.h](../../../../source/slang/slang-emit-torch.h) /
[slang-emit-torch.cpp](../../../../source/slang/slang-emit-torch.cpp).
Emits the PyTorch C++ glue used to bind Slang shaders into PyTorch
extensions. Prelude:
[slang-torch-prelude.h](../../../../prelude/slang-torch-prelude.h).

### LLVM

[slang-emit-llvm.h](../../../../source/slang/slang-emit-llvm.h) /
[slang-emit-llvm.cpp](../../../../source/slang/slang-emit-llvm.cpp).
Hands off to the JIT / native compilation path implemented in
[source/slang-llvm/](../../../../source/slang-llvm).
Helper header: [slang-llvm.h](../../../../prelude/slang-llvm.h).

### VM

[slang-emit-vm.h](../../../../source/slang/slang-emit-vm.h) /
[slang-emit-vm.cpp](../../../../source/slang/slang-emit-vm.cpp).
Emits Slang's interpreter bytecode (used by the `slangi` tool and by
`INTERPRET` tests, see [CLAUDE.md](../../../../CLAUDE.md)).

### Slang round-trip

[slang-emit-slang.h](../../../../source/slang/slang-emit-slang.h) /
[slang-emit-slang.cpp](../../../../source/slang/slang-emit-slang.cpp).
Re-emits Slang source from IR. Used for diagnostics, tests, and
debugging (`-target slang`).

### Shared C-like base

The textual backends (HLSL, GLSL, Metal, WGSL, C++, CUDA, Torch)
share most of their machinery:

- [slang-emit-c-like.h](../../../../source/slang/slang-emit-c-like.h) /
  [slang-emit-c-like.cpp](../../../../source/slang/slang-emit-c-like.cpp)
  — the base class `CLikeSourceEmitter`. It walks the IR,
  declares a virtual interface (`emitDeclarator`, `emitType`,
  `emitOperand`, ...) for the per-target subclasses to override, and
  implements all the parts that the targets share.
- [slang-emit-base.h](../../../../source/slang/slang-emit-base.h) /
  [slang-emit-base.cpp](../../../../source/slang/slang-emit-base.cpp)
  — the lowest common interface, also shared by SPIR-V and LLVM.

A new textual backend typically subclasses `CLikeSourceEmitter` and
overrides only the operations that differ from the C-like default.

## Source-writer abstraction

[slang-emit-source-writer.h](../../../../source/slang/slang-emit-source-writer.h)
declares `SourceWriter`, the buffer that all textual backends write
into. Its features (visible in the header):

- `emit*` overloads for raw text, integers, doubles, and `Name`s.
- `indent()` / `dedent()` to manage indentation.
- `advanceToSourceLocation(SourceLoc)` to emit `#line` (or GLSL
  `#line`-equivalent) directives so that downstream compilers report
  errors at the user's original source position.
- `LineDirectiveMode` configures the directive style (C / GLSL /
  none).
- A `SourceMap` companion for source-mapping debug information.

## Operator precedence and parenthesization

[slang-emit-precedence.h](../../../../source/slang/slang-emit-precedence.h)
/
[slang-emit-precedence.cpp](../../../../source/slang/slang-emit-precedence.cpp)
encode operator precedences for the textual targets so that emitted
code uses the minimum number of parentheses needed to preserve
semantics. Each backend asks the precedence helper before printing a
binary or unary operator.

## Preludes

Each textual target ships a prelude header in
[prelude/](../../../../prelude) that the emitted code includes. The
prelude defines target-specific built-in functions, type aliases,
and helper macros so that the emitted source is self-contained.

| Target | Prelude header |
| --- | --- |
| HLSL | [slang-hlsl-prelude.h](../../../../prelude/slang-hlsl-prelude.h) |
| CUDA | [slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h) |
| C++ shader | [slang-cpp-prelude.h](../../../../prelude/slang-cpp-prelude.h), [slang-cpp-types-core.h](../../../../prelude/slang-cpp-types-core.h), [slang-cpp-types.h](../../../../prelude/slang-cpp-types.h), [slang-cpp-scalar-intrinsics.h](../../../../prelude/slang-cpp-scalar-intrinsics.h) |
| C++ host | [slang-cpp-host-prelude.h](../../../../prelude/slang-cpp-host-prelude.h) |
| Torch | [slang-torch-prelude.h](../../../../prelude/slang-torch-prelude.h) |
| LLVM | [slang-llvm.h](../../../../prelude/slang-llvm.h) |

GLSL, Metal, WGSL, and SPIR-V do not use a `prelude/` header in the
same way; the runtime / built-in vocabulary they rely on lives inside
their own emit files (e.g. the Metal prelude is generated by
[slang-emit-metal-prelude.cpp](../../../../source/slang/slang-emit-metal-prelude.cpp)
rather than shipped as a header). The preludes are introduced from the
core-module side in
[../cross-cutting/core-module.md](../cross-cutting/core-module.md).

## Dependency-file output

[slang-emit-dependency-file.h](../../../../source/slang/slang-emit-dependency-file.h)
/
[slang-emit-dependency-file.cpp](../../../../source/slang/slang-emit-dependency-file.cpp)
produces Make-style `.d` files listing the source files a compile
depended on. Used by build systems to track header / module
dependencies.

## Adding a new backend

1. Add `slang-emit-<target>.{h,cpp}` under
   [source/slang/](../../../../source/slang). For a textual target,
   subclass `CLikeSourceEmitter`.
2. Register the new backend in
   [slang-emit.cpp](../../../../source/slang/slang-emit.cpp) — both the
   `#include` at the top and the dispatch logic in
   `emitEntryPointsSourceFromIR`.
3. Add a prelude under [prelude/](../../../../prelude) if the emitted
   code needs runtime support, and emit a `#include` for it from the
   backend.
4. Add capability bits in
   [slang-capabilities.capdef](../../../../source/slang/slang-capabilities.capdef)
   so the front-end can reject features the new target does not
   support — see
   [../cross-cutting/targets.md](../cross-cutting/targets.md).
5. Add target-specific IR legalization passes if needed
   ([05-ir-passes.md](05-ir-passes.md)) and gate them on the
   `TargetRequest`.
6. Add tests under [tests/](../../../../tests), typically using
   `COMPARE_COMPUTE` or `INTERPRET` directives plus per-backend
   variants — see [CLAUDE.md](../../../../CLAUDE.md) for the test
   conventions.
