---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:59:05+00:00
source_commit: 30ae111120515b7406aa6f427a4eaaa28a0903d8
watched_paths_digest: 30983b1eac237a20bb36b39636936cb7cb3bc5b003a6f2f819545a3ac80fb871
source_doc: docs/llm-generated/architecture/dependency-graph.md
source_doc_digest: 3cb2e36ed79632f04be823e3e1bb14782a2979342d9ee050543f7702c34d098a
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for architecture/dependency-graph

## Intent

Tests verify the slangc-observable consequences of the subsystem link
edges enumerated in
[`docs/llm-generated/architecture/dependency-graph.md`](../../../docs/llm-generated/architecture/dependency-graph.md).
That doc is almost entirely a **build-system** description: it records
`LINK_WITH_PUBLIC` / `LINK_WITH_PRIVATE` edges between per-directory
CMake targets in `source/<subsystem>/CMakeLists.txt`. Those edges are
invisible to `slangc`'s command line — the runner cannot inspect the
CMake graph, the order of static-link inputs, or which `.a` / `.so`
artifact contains a given symbol.

The bundle is therefore intentionally **very small** (five tests) and
the `## Out of scope` section is **long**: it itemises the per-edge
citations, the external-dependency notes, the build-system
invariants, the dashed source-include edge, and the "no observed link
edge" subsystems that the doc lists but which the runner cannot
probe.

The slangc-observable consequences that *are* tested are:

- the **core module is linked into the default-built `slang` library**
  (the `coreModule → coreLib` and `slangLib → coreModule` solid edges
  with `SLANG_EMBED_CORE_MODULE=ON` in the default build) — built-in
  vector types resolve without any `import`;
- the **`slangc → slang` and `slangc → core` edges** make `slangc` a
  driver that links against the full compilation pipeline — any
  non-trivial compile to text output is evidence that the linked
  `slang` library carries AST/IR/emit;
- the **`slang → prelude` private-include edge** materialises in
  emitted text — CUDA and CPP backends literally `#include` the
  prelude headers shipped from `prelude/`, naming the same files the
  external-dependency notes list;
- the **`glslModule → coreLib` and `slangLib → coreModule` chain**
  makes the GLSL module reachable from any `slangc` invocation —
  `import glsl;` resolves with no extra search paths.

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| The `glslModule -> coreLib` edge plus `slangLib -> coreModule` linkage make the GLSL module reachable from a normal slangc invocation; `import glsl;` resolves with no extra search paths. | functional | [#edges-intra-project-only](../../../docs/llm-generated/architecture/dependency-graph.md#edges-intra-project-only) | [`glsl-module-via-slang-lib.slang`](glsl-module-via-slang-lib.slang) |
| The `slangLib -> prelude` private-include edge ships per-target prelude headers; CPP emit literally `#include`s `slang-cpp-prelude.h`. | functional | [#edges-intra-project-only](../../../docs/llm-generated/architecture/dependency-graph.md#edges-intra-project-only) | [`cpp-prelude-include.slang`](cpp-prelude-include.slang) |
| The `slangLib -> prelude` private-include edge ships per-target prelude headers; CUDA emit literally `#include`s `slang-cuda-prelude.h`. | functional | [#edges-intra-project-only](../../../docs/llm-generated/architecture/dependency-graph.md#edges-intra-project-only) | [`cuda-prelude-include.slang`](cuda-prelude-include.slang) |
| In the default build the core module is linked into `slang` (SLANG_EMBED_CORE_MODULE); built-in vector types resolve without any `import`. | functional | [#notable-invariants](../../../docs/llm-generated/architecture/dependency-graph.md#notable-invariants) | [`core-module-embedded-default.slang`](core-module-embedded-default.slang) |
| `source/slang/` is the only target carrying AST/IR/emit; `slangc` links against `slang` (and `core`) and exposes the full lex-to-emit pipeline at the CLI. | functional | [#notable-invariants](../../../docs/llm-generated/architecture/dependency-graph.md#notable-invariants) | [`slangc-driver-emits-text.slang`](slangc-driver-emits-text.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#include](../../../docs/llm-generated/architecture/dependency-graph.md#include) | undocumented-behavior | The doc states `slangLib → prelude` is "a private include dep, not a static link, but is listed here to match `module-map.md`". The CLI consequence (prelude `#include` text in emitted CUDA / CPP) is the *only* way an outside observer can sense the edge; the doc could note this explicitly so readers do not look for a libslang symbol that does not exist. |  |
| [#notable-invariants](../../../docs/llm-generated/architecture/dependency-graph.md#notable-invariants) | undocumented-behavior | The "Notable invariants" bullet about `SLANG_EMBED_CORE_MODULE` cites the CMake variable but does not state the **default value**. The agentic test that exercises the embedded path implicitly assumes the default is `ON`; a one-line note in the doc would let future regenerations confirm the assumption without consulting the root `CMakeLists.txt`. |  |
| [#notable-invariants](../../../docs/llm-generated/architecture/dependency-graph.md#notable-invariants) | undocumented-behavior | The "Notable invariants" bullet about `slangc → slang` says "Every other binary that needs compilation services (such as `slangc`) links against `slang` rather than reaching into individual files." The CLI consequence (a single `slangc` invocation can run lex → parse → check → IR → emit on one source) is *implied* by the wording but not spelled out as a slangc-observable rule. |  |
| [#edges-intra-project-only](../../../docs/llm-generated/architecture/dependency-graph.md#edges-intra-project-only) | undocumented-behavior | The "Edges (intra-project only)" diagram and "Edge citations" table use the bare subsystem name (`coreLib`, `slangLib`, `slangc`) but the per-edge slug used in this bundle's `doc_ref` resolves only at H2 level. Multiple distinct edges share one anchor (`#edges-intra-project-only`). The doc has no H3 sub-anchor per edge; adding one (e.g. `### slang → prelude (private-include)`) would let per-edge tests be cited individually. |  |
| [#no-ordinary-linkwith-edge](../../../docs/llm-generated/architecture/dependency-graph.md#no-ordinary-linkwith-edge) | undocumented-behavior | The doc lists three "no ordinary `LINK_WITH_*` edge" subsystems (`source/standard-modules/`, `source/slang-record-replay/`, `source/slang-llvm/`) but none expose a clean slangc CLI surface. The `standard-modules` `neural/` module is gated behind `-experimental-feature`; `slang-record-replay` is folded into `slang` sources; `slang-llvm` is downloaded out-of-tree. The doc could mark which of these has a CLI observable so future bundles do not waste cycles probing them. |  |

## Out of scope (no-GPU runner)

(In this bundle the section heading is used for "claims unobservable
through any allowed test directive" — none are literally GPU-bound.
The dependency graph is a build-system document; nearly every claim
in it concerns the contents of `CMakeLists.txt` files, which the
runner cannot read.)

### Per-edge CMake clauses (cited by the doc, unobservable to slangc)

- `compiler-core → core` from
  `source/compiler-core/CMakeLists.txt` (`LINK_WITH_PRIVATE core`).
  The diagnostic-sink, downstream-compiler glue, and lexer
  infrastructure live in `compiler-core`, but the *static link
  clause itself* is not visible to `slangc`. Diagnostic behavior is
  tested in cross-cutting/diagnostics bundles.
- `capability-lookup → core` and
  `capability-lookup → capability-defs` from
  `source/slang/CMakeLists.txt`. The capability system has CLI
  observables (rejection of unsupported targets, capability error
  text), but those are anchored in
  `cross-cutting/capability-system.md`, not in the link graph.
- `capability-defs → core` (`LINK_WITH_PUBLIC core`) — the only
  `PUBLIC` edge among the capability libraries. The
  public/private split is invisible to `slangc`.
- `lookup-tables → core` from `source/slang/CMakeLists.txt`.
  Lookup tables back SPIR-V opcode lookup and similar mappings;
  their *existence as a separate library* is not slangc-observable.
- `core-module → core`, `core-module → capability-defs`,
  `core-module → fiddle-output` from
  `source/slang-core-module/CMakeLists.txt`. The core module's
  CLI consequence (built-in types resolve) is tested via C-01;
  the per-edge structure is not.
- `slang → {core, prelude, compiler-core, capability-defs,
  capability-lookup, fiddle-output, lookup-tables, core-module}`
  from `source/slang/CMakeLists.txt`. Only `slang → prelude`
  (C-03) and `slang → core-module` (C-01, C-04) have direct CLI
  observables; the other six edges are intermediates between
  build subsystems that contribute jointly to the end-to-end
  compile (covered loosely by C-02 but not per-edge).
- `slangc → core`, `slangc → slang` from
  `source/slangc/CMakeLists.txt`. The composite consequence is
  covered by C-02; the existence of two separate edges (vs one
  transitive edge via `slang`) is a build-system fact.
- `slang-dispatcher → core` from
  `source/slang-dispatcher/CMakeLists.txt`. `slang-dispatcher` is
  an out-of-tree-invoked tool; the runner does not exercise it.
- `slang-wasm → {slang, core, compiler-core, capability-defs,
  capability-lookup, fiddle-output}` from
  `source/slang-wasm/CMakeLists.txt`. The WASM binding is consumed
  by JavaScript callers, not by `slangc`. Unobservable here.

### External dependencies (listed in the doc, unobservable to slangc)

- `core` external deps: `miniz`, `lz4_static`, `Threads::Threads`,
  `unordered_dense`, `${CMAKE_DL_LIBS}`. These back blob compression,
  thread primitives, and dynamic loading inside `slangc`; their CLI
  consequences (artifact compression, thread-pool usage, dynamic
  module loading) are not surfaced as user-visible CLI behavior.
- `slang` + `slang-wasm` external dep: `SPIRV-Headers`. The SPIR-V
  headers back opcode definitions used during SPIR-V emit; the
  end-to-end consequence is "SPIR-V emit works," which overlaps with
  general emit and is anchored elsewhere.
- `slang-rt` external deps: `miniz`, `lz4_static`, `Threads`,
  `unordered_dense`, `${CMAKE_DL_LIBS}`. The doc notes "`slang-rt`
  does not consume the compiler's own code." `slang-rt` is shipped
  alongside emitted CPU-target output and is invoked by user host
  code, not by `slangc`; the runner cannot reach it.
- `slang-glslang` external deps: `glslang`, `SPIRV`,
  `SPIRV-Tools-opt`, `SPIRV-Tools-link`. The CLI consequence is
  "`slangc` can use glslang as a downstream compiler" — observable
  but anchored in the targets / downstream-compiler docs, not in the
  link-graph doc.
- `slang-lookup-tables` external dep: `SPIRV-Headers`. Same family
  as the SPIR-V emit case; build-system fact.

### "No observed link edge" subsystems

- `source/standard-modules/` has no `slang_add_target` of its own;
  it only `configure_file`s a config header and `add_subdirectory`s
  the `neural` module, which ships as a standalone `.slang-module`.
  Loading `neural` requires `-experimental-feature` and the doc
  makes no commitment about a CLI shape. No test.
- `source/slang-record-replay/` has no `CMakeLists.txt`; its
  sources are pulled into `slang` via the `SLANG_RECORD_REPLAY_SYSTEM`
  variable in `source/slang/CMakeLists.txt` (the dashed edge in the
  diagram). The record/replay API is invoked from host-side code
  through `include/slang.h`, not from the `slangc` command line. No
  test.
- `source/slang-llvm/` has no `CMakeLists.txt`; the artifact is
  produced out-of-tree (or downloaded as a prebuilt binary controlled
  by `SLANG_SLANG_LLVM_FLAVOR`). The LLVM-backed CPU path is engaged
  via `-target` flags but the bridge directory's *existence* is not
  CLI-observable. No test.

### Build-system convenience facts

- The `slang-common-objects` indirection in
  `source/slang/CMakeLists.txt` (object library re-linked into both
  `slang-without-embedded-core-module` and `slang`). A CMake-only
  optimisation; the consequence (two flavours of `libslang` ship)
  is observable only when running an installer that includes both
  flavours, which the runner does not exercise.
- The cited line numbers (`source/slang/CMakeLists.txt lines
  164-167` for `SLANG_RECORD_REPLAY_SYSTEM`, root `CMakeLists.txt`
  around line 355 for `SLANG_SLANG_LLVM_FLAVOR`). Locations of
  build-system code, not runtime behavior.
- The project rule "Public headers in `include/` must not include
  private headers from `source/`." The doc names this as a project
  rule, not a build-system constraint; it is enforced by code
  review, not by `slangc` at runtime. No test.

### Per-edge `PUBLIC` vs `PRIVATE` distinction

- The doc carefully distinguishes `LINK_WITH_PUBLIC` (one edge,
  `capability-defs → core`) from `LINK_WITH_PRIVATE` (all others).
  The transitive-include consequence (public consumers of
  `capability-defs` also see `core`'s interface headers) is a C++
  / CMake matter, not slangc-observable.

### Cycles and irregularities

- The doc states "No link-level cycles are observed." A negative
  build-system claim about the CMake graph; cannot be falsified by
  `slangc` at the CLI.

### Cross-doc pointers

- `module-map.md` (for file-level inventory) and
  `pipeline/overview.md` (for runtime data flow) — navigation; each
  has its own bundle.
