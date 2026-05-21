# Prompt: tests-agentic/cross-cutting/core-module/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/cross-cutting/core-module/`,
anchored to
[`docs/llm-generated/cross-cutting/core-module.md`](../../../docs/llm-generated/cross-cutting/core-module.md).

Audience: nightly CI. The bundle exercises the **core module / GLSL
module / standard module / prelude** abstraction described in the
source doc. The angle is:

1. The core module is a single, identified Slang module
   (`public module core;`) whose declarations are visible without an
   explicit `import`.
2. Its declarations include scalar/vector/matrix base types,
   typedef aliases (`int32_t`, `uint32_t`, `float32_t`, `float64_t`,
   `float16_t`, `size_t`), and per-target intrinsic spellings driven
   by `__intrinsic_op` / `__target_intrinsic` modifiers.
3. The HLSL meta-module supplies HLSL-named texture / sampler /
   buffer types and intrinsics (`mul`, `dot`, `length`, ...) so HLSL
   source compiles unchanged.
4. The diff meta-module declares `IDifferentiable` and the
   differentiable-pair types.
5. The GLSL module supplies `vec3` / `mat4` / `gl_*` aliases when
   the active target / source family wants GLSL-flavoured names.
6. Standard modules (`import slang.<name>`; currently `neural`) are
   loaded on demand at runtime, not embedded.
7. Per-target preludes (the C/C++/CUDA headers in `prelude/`) are
   referenced from emitted text output, not from input-side
   compilation.

The bundle is **multi-target where the claim is target-dependent**
(intrinsic lowering, prelude inclusion). Module-identity claims
(e.g. that `core` is one identified module, that `int32_t` is a
typedef alias) are target-independent and can use a single emit or
the interpreter.

## The translation rule: claims to observations

### Observable claims (write tests for these)

- **Core typedef aliases resolve** (`#what-the-core-module-provides`,
  `#core-module`): `int32_t` / `uint32_t` / `float32_t` /
  `float64_t` / `float16_t` / `size_t` are usable in user code
  without an `import`, because `public module core;` is implicitly
  in scope.
- **The core module is one identified module** (`#core-module`):
  there is one `public module core;` and its declarations are
  visible from user code without explicit import. Observable
  through use of a core typedef in a minimal entry point.
- **HLSL meta-module intrinsics lower per target**
  (`#what-the-core-module-provides`, `#core-module`): `dot`,
  `length`, `mul` compile on every text-emit backend and surface
  target-shaped output (e.g. `dot` -> HLSL `dot`, GLSL `dot`,
  SPIR-V `OpExtInst ... Length` for `length`, Metal `dot`).
- **HLSL meta-module resource types lower per target**
  (`#core-module`): `RWStructuredBuffer<T>` / `StructuredBuffer<T>`
  legalize to per-target binding shapes — HLSL `register(uN)`,
  GLSL/SPIR-V `Binding`, Metal `[[buffer(N)]]`, WGSL `@binding`.
- **GLSL module aliases compile when used in Slang source**
  (`#glsl-module`): `vec3` is an alias for `float3`; using it in
  user code is accepted and lowers to the same per-target spelling
  as `float3` (HLSL `float3`, GLSL `vec3`, SPIR-V `OpTypeVector
  %float 3`).
- **Diff meta-module supplies `IDifferentiable`** (`#core-module`):
  a struct declared `: IDifferentiable` with `Differential = This`
  is accepted by the front-end and an `__init_differential` /
  `dadd` call site compiles. (Front-end check only; full autodiff
  lives in `pipeline/05-ir-passes`.)
- **C++ prelude is referenced from emitted CUDA / CPP text**
  (`#preludes`): a compute kernel emitted with `-target cpp` /
  `-target cuda` references prelude artefacts (`SLANG_PRELUDE_EXPORT`
  for CPP; `__global__` / `extern "C"` for CUDA). The HLSL prelude
  is emitted via `slang-hlsl-prelude.h` references in HLSL output.
- **`size_t` width follows the target's pointer width**
  (`#core-module`): `size_t` is documented as `typedef uintptr_t
  size_t;` so it lowers per target; observe it via a simple
  declaration / use.
- **Float-bit-width aliases survive lowering**
  (`#what-the-core-module-provides`): `float16_t` lowers to
  `half` / 16-bit float on HLSL and to `float16_t` on SPIR-V
  (where supported); `float32_t` lowers to `float` / 32-bit float.
- **Standard module `neural` is not implicitly imported**
  (`#standard-modules`): a reference to a `neural`-namespaced name
  without `import slang.neural;` is an unknown identifier on the
  front end. (Negative test.)
- **`Optional<T>` / `Tuple<T...>` are core-module types**
  (`#what-the-core-module-provides`): they are usable without an
  explicit `import`. Observe via a minimal use.

### Not testable through slangc text emit (record under `## Out of scope`)

- **Atomic-op intrinsics** (e.g. `InterlockedAdd` / `atomicAdd` /
  `OpAtomicIAdd`) — target-specific spellings; previous wave got
  stuck iterating. Skip entirely.
- **The build-system selection of `SLANG_EMBED_CORE_MODULE`
  ON vs. OFF** (`#building-the-core-module`) — affects how the
  module is shipped, not what user code sees. Not user-observable.
- **The byte-for-byte embedded module artefact**
  (`#building-the-core-module`) — internal build product.
- **The standard-module-config.h.in template machinery**
  (`#standard-modules`) — internal build infrastructure.
- **The full intrinsic list** — the doc explicitly disclaims
  enumerating it; the authoritative source is the `*.meta.slang`
  files.
- **Documentation-only enumeration of preludes by family**
  (`#preludes`) — the table lists files; we test that one or two
  are referenced from emit, not that every header in `prelude/`
  is touched.
- **Adding-a-new-built-in workflow** (`#adding-a-new-built-in`) —
  developer workflow, not a user-observable behaviour.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. 10 to 18 `.slang` test files. Module-identity claims are
   target-independent (one backend suffices). Intrinsic-lowering
   and prelude-inclusion claims carry multiple `//TEST:SIMPLE`
   directives, one per text-emit target where the claim is
   observable.
3. Mix:
   - **multi-target SIMPLE** tests for intrinsic lowering;
   - one or two **`DIAGNOSTIC_TEST`** tests (e.g. unimported
     standard-module name rejected);
   - **`INTERPRET`** tests where the claim is genuinely target-
     independent (e.g. `int32_t` typedef compiles and runs).

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/cross-cutting/core-module.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/llm-generated/syntax-reference/keywords-and-builtins.md`
  (for `__intrinsic_op` / `__target_intrinsic` modifier vocab)
- `docs/llm-generated/pipeline/05-ir-passes.md` (for diff)
- `docs/llm-generated/pipeline/06-emit.md` (for prelude inclusion)

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm the spelling of a typedef,
intrinsic, or capability atom. You may **not** mine them for
behavioural claims the doc does not make.

- `source/slang/core.meta.slang`
- `source/slang/hlsl.meta.slang`
- `source/slang/diff.meta.slang`
- `source/slang/glsl.meta.slang`
- `prelude/slang-cpp-prelude.h`, `prelude/slang-cuda-prelude.h`,
  `prelude/slang-hlsl-prelude.h`

## Test directives

Most lowering claims are multi-target. The default for a positive
emit test mirrors `cross-cutting-targets.md`:

```
//TEST:SIMPLE(filecheck=HLSL):-target hlsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda  -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp   -entry main -stage compute
```

For a module-identity claim (target-independent), prefer:

```
//TEST:INTERPRET(filecheck=CHECK):
```

For a negative (unimported module name) test:

```
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute
```

Avoid GPU-only directives. **Do not write atomic-op tests.**

## Lessons specific to core-module tests

These add to the universal lessons in `_common.md`:

- **Atomics are forbidden ground in this bundle.** Their per-target
  spellings (HLSL `InterlockedAdd`, GLSL `atomicAdd`, SPIR-V
  `OpAtomicIAdd`) are not directly cited in the source doc and
  prior agents have wasted iterations on them. List them under
  `## Out of scope` with one sentence and move on.
- **Don't duplicate `syntax-reference/keywords-and-builtins`.** That
  bundle owns the modifier-vocabulary surface (`__intrinsic_op`,
  `__target_intrinsic`). This bundle is about module identity and
  observable per-target lowering through those modifiers — the
  emit text, not the modifier itself.
- **`float16_t` requires capability on some targets** — HLSL needs
  `-profile sm_6_2` or similar for 16-bit float. Use targets that
  natively accept 16-bit (SPIR-V with `-emit-spirv-directly` plus
  `-capability spvFloat16` if needed) or fall back to documenting
  the limitation in a comment.
- **The diff meta-module's surface is type-level** — a test that
  declares a struct with `: IDifferentiable` and `typealias
  Differential = This` is enough to assert front-end acceptance;
  no need to invoke `__bwd_diff` or actual autodiff.
- **`neural` standard module is not embedded.** A test that asserts
  it is **not** auto-imported must not depend on its presence at
  the agentic runner. The negative test should reject a `neural`-
  namespaced name without `import slang.neural;`.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `cross-cutting/core-module.md` (or a listed secondary doc).
- [ ] No test exercises an atomic intrinsic.
- [ ] Each multi-target `.slang` file carries `//TEST:SIMPLE`
      directives for every text-emit target where the claim is
      observable.
- [ ] CHECK patterns avoid raw `[[...]]` (FileCheck variable
      syntax) and use wildcards (`{{[0-9]+}}`, `{{.*}}`) for
      mangled identifiers.
- [ ] At least one test exercises a `DIAGNOSTIC_TEST` (e.g. the
      unimported standard-module name).
- [ ] No test depends on a GPU.
- [ ] No test was written by inspecting an uncovered source line.
- [ ] `## Out of scope` lists atomic intrinsics, the
      `SLANG_EMBED_CORE_MODULE` build option, the embedded
      artefact's byte form, the standard-module-config template,
      and the adding-a-new-built-in workflow.
