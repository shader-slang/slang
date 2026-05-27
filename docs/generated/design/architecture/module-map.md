---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-15T15:30:00+00:00
source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
watched_paths_digest: f11187e79ffaa9e1c1966046a9bb76df4bb33cabc81ba4f496d6f224fcf0ca12
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Module Map

This document is the look-up table that complements
[overview.md](overview.md). For each major subsystem in the source
tree it lists the files (or file families, where the family is large)
that constitute its **logical units**, with one-line responsibility
notes. Use it to answer questions of the form *"where is X
implemented?"*.

The intended reader has already read [overview.md](overview.md) and
wants to find a specific subsystem.

## How to read the tables

Where a logical unit is a single header / source pair, the row cites
the pair. Where a logical unit is a family of files sharing a prefix
(typical for the AST and IR), the row cites a representative file and
notes the prefix; the full enumeration belongs in the corresponding
pipeline doc.

Files cited in this document exist at the recorded `source_commit`. If
you find a discrepancy, do not hand-edit this file: update the watched
paths in [../_meta/manifest.yaml](../_meta/manifest.yaml) and
regenerate.

## source/core/ — platform-agnostic utilities

Foundational layer. Used by every other subdirectory and itself
depends on nothing else in the project.

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Containers and basic types | [slang-basic.h](../../../../source/core/slang-basic.h), `slang-array.h`, `slang-array-view.h`, `slang-dictionary.h`, `slang-list.h`, `slang-chunked-list.h` | Slang-specific replacements for `std::vector`, `std::array`, hash maps |
| Strings and char encoding | `slang-string.h`, `slang-string-util.h`, `slang-char-encode.h`, `slang-char-util.h` | UTF-8 / UTF-16 strings and helpers |
| Smart pointers and reference counting | `slang-smart-pointer.h`, `slang-com-object.h`, `slang-castable.h` | Intrusive refcounting and COM-style smart pointers |
| File system | `slang-file-system.h`, `slang-archive-file-system.h`, `slang-implicit-directory-collector.h` | Abstract file-system interfaces and archive-backed implementations |
| Hashing and crypto | `slang-hash.h`, `slang-crypto.h` | Stable hashes used by serialization and deduplication |
| Compression and blobs | `slang-blob.h`, `slang-blob-builder.h`, `slang-deflate-compression-system.h` | In-memory binary buffers shared with the public API |
| Command-line parsing | `slang-command-line.h`, `slang-command-options.h`, `slang-command-options-writer.h` | Generic option parsing reused by `slangc` and tools |
| Allocation | `slang-allocator.h`, `slang-free-list.h` | Arena and free-list allocators used by the AST and IR |

## source/compiler-core/ — language-agnostic compiler infrastructure

Reusable compiler infrastructure that is independent of Slang as a
language. May depend on `source/core/` but not on `source/slang/`.

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Lexer | [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h), [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp) | Token producer over a source buffer |
| Tokens | `slang-token.h`, `slang-token.cpp` | `Token` and `TokenKind` types; see [../syntax-reference/tokens.md](../syntax-reference/tokens.md) |
| Source locations | `slang-source-loc.h`, `slang-source-loc.cpp` | Compact integer source-location encoding and `SourceManager` |
| Diagnostic sink | [slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h), `slang-diagnostic-sink.cpp` | Generic interface for emitting diagnostics |
| Diagnostic core catalog | `slang-core-diagnostics.h`, `slang-core-diagnostics.cpp`, `slang-json-diagnostic-defs.h`, `slang-json-diagnostics.cpp` | Diagnostic codes shared across languages |
| Doc extractor | `slang-doc-extractor.h`, `slang-doc-extractor.cpp` | Pulls doc comments out of token streams |
| Artifact model | `slang-artifact.h`, `slang-artifact-impl.cpp`, `slang-artifact-handler-impl.cpp`, `slang-artifact-representation*` | Polymorphic compiled-output containers used by the public API |
| Artifact utilities | `slang-artifact-associated-impl.cpp`, `slang-artifact-container-util.cpp`, `slang-artifact-desc-util.cpp`, `slang-artifact-diagnostic-util.cpp`, `slang-artifact-helper.cpp`, `slang-artifact-util.cpp` | Helpers and metadata accessors for artifacts |
| Downstream-compiler glue | `slang-downstream-compiler.h`, `slang-downstream-compiler-set.h`, `slang-downstream-compiler-util.h` | Abstracts invocation of external compilers |
| Per-vendor compilers | `slang-dxc-compiler.cpp`, `slang-fxc-compiler.cpp`, `slang-glslang-compiler.cpp`, `slang-gcc-compiler-util.cpp`, `slang-json-lexer.cpp` | Concrete bridges to DXC, FXC, glslang, GCC, JSON parsing |
| Include search | `slang-include-system.h`, `slang-include-system.cpp` | Path resolution for `#include` and `import` |
| JSON lexer | `slang-json-lexer.h`, `slang-json-lexer.cpp` | JSON tokenizer used by some downstream tools |
| Command-line args | `slang-command-line-args.h`, `slang-command-line-args.cpp` | Generic argument extraction shared with frontends |

## source/slang/ — frontend, IR, passes, emit

The bulk of the compiler. Files share consistent prefixes that mark
which subsystem they belong to. The deep treatment of each subsystem
lives in the [pipeline](../pipeline) and
[cross-cutting](../cross-cutting) docs; this section is the index.

### Compile-request orchestration

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Front-end compile request | [slang-compile-request.h](../../../../source/slang/slang-compile-request.h), `slang-compile-request.cpp` | Drives parsing, checking, lowering for a translation unit |
| End-to-end compile request | `slang-end-to-end-request.cpp` | Backs a single `slangc` / public-API compile invocation |
| Module | [slang-module.h](../../../../source/slang/slang-module.h), `slang-module.cpp` | Holds AST + IR for a translation unit; implements `IModule` |
| Module library | `slang-module-library.h`, `slang-module-library.cpp` | Bundles compiled modules into reusable libraries |
| Linkage | `slang-session.h`, `slang-session.cpp` | The class behind the public `slang::ISession` — a per-configuration scope owning search paths, target settings, and the source manager |
| Linkable components | `slang-linkable.h`, `slang-linkable-impl.cpp` (and friends) | `IComponentType` and its composite / specialized variants — the linkable-program abstraction used by the back-end |
| Session (global) | `slang-global-session.h`, `slang-global-session.cpp` | Process-wide `Session` class behind `slang::IGlobalSession` |

### Frontend (lex / preprocess / parse)

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Preprocessor | [slang-preprocessor.h](../../../../source/slang/slang-preprocessor.h), [slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp) | `#include`, macros, conditional directives |
| Parser | [slang-parser.h](../../../../source/slang/slang-parser.h), [slang-parser.cpp](../../../../source/slang/slang-parser.cpp) | Recursive-descent parser that produces the AST; two-stage model |
| Syntax declarations | `slang-syntax.h`, `slang-syntax.cpp` | Syntax-as-declaration: keyword → parser-callback registry |

### AST

The AST is a strongly-typed C++ class hierarchy with FIDDLE-generated
visitor and serialization support. One logical unit per node family.

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Forward declarations and bases | `slang-ast-forward-declarations.h`, `slang-ast-base.h`, `slang-ast-base.cpp` | Root types for AST nodes |
| All-in-one umbrella | `slang-ast-all.h` | Collected include for users |
| Declarations | `slang-ast-decl.h`, `slang-ast-decl.cpp`, `slang-ast-decl-ref.cpp` | `Decl` hierarchy and `DeclRef` |
| Expressions | `slang-ast-expr.h` | `Expr` hierarchy |
| Statements | `slang-ast-stmt.h` | `Stmt` hierarchy |
| Types | `slang-ast-type.h`, `slang-ast-type.cpp` | `Type` hierarchy |
| Modifiers | `slang-ast-modifier.h`, `slang-ast-modifier.cpp` | Attribute / qualifier nodes attached to decls |
| Vals | `slang-ast-val.h`, `slang-ast-val.cpp` | Compile-time value nodes (used by generics) |
| Builder | `slang-ast-builder.h`, `slang-ast-builder.cpp` | Allocation, hash-consing of types |
| Dispatch / iteration | `slang-ast-dispatch.h`, `slang-ast-iterator.h` | Visitor dispatch over the hierarchy |
| Dump and print | `slang-ast-dump.h`, `slang-ast-dump.cpp`, `slang-ast-print.h`, `slang-ast-print.cpp` | Debug rendering |
| Synthesis | `slang-ast-synthesis.h`, `slang-ast-synthesis.cpp` | Generated members, default conformances |
| Natural layout | `slang-ast-natural-layout.h`, `slang-ast-natural-layout.cpp` | Layout assigned during AST traversal |
| Support types | `slang-ast-support-types.h`, `slang-ast-support-types.cpp` | Misc helpers and small types |
| Boilerplate | `slang-ast-boilerplate.cpp` | FIDDLE-driven boilerplate definitions |

### Semantic checking

One file per checking concern; all collaborate through
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h).

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Top-level driver | [slang-check.h](../../../../source/slang/slang-check.h), [slang-check.cpp](../../../../source/slang/slang-check.cpp) | Visitor orchestration; entry point for AST checking |
| Internal interface | `slang-check-impl.h` | Shared state passed between check files |
| Declaration checking | `slang-check-decl.cpp` | Resolves and validates `Decl` nodes |
| Expression checking | `slang-check-expr.cpp` | Type-checks `Expr` nodes |
| Statement checking | `slang-check-stmt.cpp` | Type-checks `Stmt` nodes |
| Type checking | `slang-check-type.cpp` | Resolves `Type` references |
| Overload resolution | `slang-check-overload.cpp` | Selects a candidate from an overload set |
| Conformance | `slang-check-conformance.cpp` | Verifies and synthesizes interface conformances |
| Conversions | `slang-check-conversion.cpp` | Implicit-conversion ranking |
| Inheritance | `slang-check-inheritance.cpp` | Inheritance / extension lookup |
| Modifier validation | `slang-check-modifier.cpp` | Verifies modifier combinations |
| Constraint solving | `slang-check-constraint.cpp` | Generic constraints from `where`-clauses |
| Resolved-value checking | `slang-check-resolve-val.cpp` | Validates `Val` nodes after substitution |
| Shader-specific checks | `slang-check-shader.cpp` | Entry-point and stage-specific validation |
| Out-of-bound access | `slang-check-out-of-bound-access.h`, `slang-check-out-of-bound-access.cpp` | Detects literal out-of-bound indexing |

### AST → IR lowering

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Lowering driver | [slang-lower-to-ir.h](../../../../source/slang/slang-lower-to-ir.h), [slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp) | Walks the checked AST and emits IR via `IRBuilder` |

### IR core

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| IR types and instructions | [slang-ir.h](../../../../source/slang/slang-ir.h), [slang-ir.cpp](../../../../source/slang/slang-ir.cpp) | `IRInst`, `IRModule`, `IRBuilder`, traversal helpers |
| Instruction definitions | [slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h), [slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) | Lua-driven catalog of opcodes; see [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md) |

### IR passes

The `slang-ir-*.cpp` family contains roughly 300 files that implement
analyses and transformations on the IR. **Do not enumerate them
here**; the pass categories and per-pass entries belong in
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md). Examples
of the categories visible from filenames:

- Cleanup / canonicalization (`slang-ir-dce`, `slang-ir-simplify`,
  `slang-ir-cleanup-void`).
- Specialization and generics (`slang-ir-specialize`,
  `slang-ir-defunctionalization`, `slang-ir-bind-existentials`,
  `slang-ir-any-value-marshalling`, `slang-ir-any-value-inference`).
- Differentiation (`slang-ir-autodiff`, `slang-ir-autodiff-fwd`,
  `slang-ir-autodiff-rev`, `slang-ir-autodiff-transpose`,
  `slang-ir-autodiff-unzip`, `slang-ir-autodiff-loop-analysis`,
  `slang-ir-autodiff-cfg-norm`, `slang-ir-autodiff-pairs`,
  `slang-ir-autodiff-primal-hoist`, `slang-ir-autodiff-region`,
  `slang-ir-check-differentiability`).
- Layout / binding (`slang-ir-collect-global-uniforms`,
  `slang-ir-com-interface`, `slang-ir-byte-address-legalize`).
- Validation (`slang-ir-check-recursion`,
  `slang-ir-check-shader-parameter-type`,
  `slang-ir-check-unsupported-inst`,
  `slang-ir-check-optional-none-usage`,
  `slang-ir-check-specialize-generic-with-existential`,
  `slang-ir-detect-uninitialized-resources`).
- Coverage instrumentation (`slang-ir-coverage-instrument`).
- Target-specific lowering (passes whose name starts with a target
  prefix or is suffixed with the target acronym).
- Shared utilities (`slang-ir-clone`, `slang-ir-dominators`,
  `slang-ir-call-graph`, `slang-ir-deduplicate`).

### Code emission

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Emit dispatcher | [slang-emit.cpp](../../../../source/slang/slang-emit.cpp) | Selects backend per `TargetRequest` |
| Common base | `slang-emit-base.h`, `slang-emit-base.cpp` | Shared state between backends |
| C-like text base | `slang-emit-c-like.h`, `slang-emit-c-like.cpp` | Shared parent for HLSL/GLSL/Metal/WGSL/CPP/CUDA |
| HLSL | `slang-emit-hlsl.cpp`, `slang-emit-hlsl.h`, `slang-emit-hlsl-prelude.cpp` | HLSL text output |
| GLSL | `slang-emit-glsl.cpp`, `slang-emit-glsl.h` | GLSL text output |
| SPIR-V | `slang-emit-spirv.cpp`, `slang-emit-spirv-ops.h`, `slang-emit-spirv-ops-debug-info-ext.h` | Direct SPIR-V binary |
| Metal | `slang-emit-metal.cpp`, `slang-emit-metal.h`, `slang-emit-metal-prelude.cpp` | Metal Shading Language |
| WGSL | `slang-emit-wgsl.cpp`, `slang-emit-wgsl.h` | WGSL output |
| CPP | `slang-emit-cpp.cpp`, `slang-emit-cpp.h` | C++ output |
| CUDA | `slang-emit-cuda.cpp`, `slang-emit-cuda.h` | CUDA output |
| Torch | `slang-emit-torch.cpp`, `slang-emit-torch.h` | PyTorch glue |
| LLVM | `slang-emit-llvm.cpp`, `slang-emit-llvm.h` | LLVM IR / native via `slang-llvm` |
| VM | `slang-emit-vm.cpp`, `slang-emit-vm.h` | Slang interpreter bytecode |
| Slang round-trip | `slang-emit-slang.cpp`, `slang-emit-slang.h` | Re-emit Slang source from IR |
| Source writer | `slang-emit-source-writer.h`, `slang-emit-source-writer.cpp` | Indented text with `#line`-style location tracking |
| Operator precedence | `slang-emit-precedence.h`, `slang-emit-precedence.cpp` | Parenthesization for textual targets |
| Dependency-file output | `slang-emit-dependency-file.h`, `slang-emit-dependency-file.cpp` | Make-style `.d` files |

### Cross-cutting

These prefixes have their own dedicated docs in
[../cross-cutting/](../cross-cutting):

- Diagnostics (`slang-diagnostics*`,
  [diagnostics/](../../../../source/slang/diagnostics)) →
  [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).
- Capability system (`slang-capability*`,
  `slang-capabilities.capdef`) →
  [../cross-cutting/targets.md](../cross-cutting/targets.md).
- Profiles (`slang-profile.h`, `slang-profile.cpp`) →
  [../cross-cutting/targets.md](../cross-cutting/targets.md).
- Serialization (`slang-serialize*`) →
  [../cross-cutting/serialization.md](../cross-cutting/serialization.md).

### API surface helpers

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Public API entry | `slang-api.cpp` | Implements the C entry points declared in `slang.h` |
| Artifact output | `slang-artifact-output-util.h`, `slang-artifact-output-util.cpp` | Bridges the artifact model to public outputs |

## source/slang-core-module/ — embedded core module

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Embedded source | `slang-embedded-core-module.cpp`, `slang-embedded-core-module-source.cpp` | Source text and embed glue for the core module compiled into `libslang` |

The actual core-module source code lives in
[source/slang/core.meta.slang](../../../../source/slang/core.meta.slang),
[source/slang/hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang),
and
[source/slang/diff.meta.slang](../../../../source/slang/diff.meta.slang).
See [../cross-cutting/core-module.md](../cross-cutting/core-module.md).

## source/slang-glsl-module/ — embedded GLSL module

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Embedded source | `slang-embedded-glsl-module.cpp` | Embeds [glsl.meta.slang](../../../../source/slang/glsl.meta.slang) |

## source/standard-modules/ — standard libraries

| Logical unit | Files | Responsibility |
| --- | --- | --- |
| Configuration | `slang-standard-module-config.h.in` | CMake-templated configuration header |
| Neural module | [neural/](../../../../source/standard-modules/neural) | Standard module for neural / ML workloads |

## prelude/ — per-target prelude headers

These headers are shipped alongside emitted text targets so that the
output can be compiled by the downstream toolchain.

| Logical unit | Files | Target |
| --- | --- | --- |
| Host C++ prelude | [slang-cpp-host-prelude.h](../../../../prelude/slang-cpp-host-prelude.h) | C++ host runtime |
| C++ prelude | [slang-cpp-prelude.h](../../../../prelude/slang-cpp-prelude.h), [slang-cpp-types-core.h](../../../../prelude/slang-cpp-types-core.h), [slang-cpp-types.h](../../../../prelude/slang-cpp-types.h) | C++ shader output |
| C++ scalar intrinsics | [slang-cpp-scalar-intrinsics.h](../../../../prelude/slang-cpp-scalar-intrinsics.h) | C++ scalar intrinsic implementations |
| CUDA prelude | [slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h) | CUDA |
| HLSL prelude | [slang-hlsl-prelude.h](../../../../prelude/slang-hlsl-prelude.h) | HLSL |
| LLVM helpers | [slang-llvm.h](../../../../prelude/slang-llvm.h) | `slang-llvm` integration |
| Torch prelude | [slang-torch-prelude.h](../../../../prelude/slang-torch-prelude.h) | PyTorch glue |

## Other source/ subdirectories

| Subdirectory | Role |
| --- | --- |
| [source/slang-llvm/](../../../../source/slang-llvm) | LLVM-based JIT / native compilation |
| [source/slang-glslang/](../../../../source/slang-glslang) | Bridge to Khronos glslang for SPIR-V via GLSL |
| [source/slang-dispatcher/](../../../../source/slang-dispatcher) | Common dispatcher for downstream-tool invocation |
| [source/slang-rt/](../../../../source/slang-rt) | Runtime library used by emitted CPU/Torch/CUDA targets |
| [source/slang-record-replay/](../../../../source/slang-record-replay) | Public-API call recorder/replayer |
| [source/slang-wasm/](../../../../source/slang-wasm) | WebAssembly bindings |
| [source/slangc/](../../../../source/slangc) | Command-line driver for the compiler |
