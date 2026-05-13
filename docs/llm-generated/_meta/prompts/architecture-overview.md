# Prompt: architecture/overview.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/architecture/overview.md` — the
top-level orientation document for the entire LLM-generated tree. A new
contributor should be able to read this single page and walk away with
a correct mental model of how the Slang source tree is organized into
subsystems, what depends on what, and where to look next.

Audience: a competent C++ engineer who has never opened the Slang source
tree before.

## Required sections

1. `# Architectural Overview` (title)
2. `## Purpose` — one paragraph naming Slang's role (a shading-language
   compiler) and the public artefacts it produces (`slangc`, the
   `libslang` shared library, runtime bindings).
3. `## Top-level decomposition` — describe the major subsystem layers in
   the source tree:
   - `source/core/` — platform-agnostic C++ utilities.
   - `source/compiler-core/` — language-agnostic compiler infrastructure
     (lexer, diagnostics, downstream-compiler glue, artifact model).
   - `source/slang/` — the Slang frontend, IR, IR passes, and emit
     backends (the bulk of the compiler).
   - `source/slang-core-module/`, `source/slang-glsl-module/`,
     `source/standard-modules/`, `prelude/` — the standard libraries
     and language preludes.
   - `source/slang-rt/` — runtime support library.
   - `source/slang-record-replay/` — API call recorder/replayer.
   - `source/slang-wasm/` — WebAssembly bindings.
   - `source/slangc/` — the command-line driver.
   - `source/slang-llvm/`, `source/slang-glslang/`,
     `source/slang-dispatcher/` — downstream-compiler shims.
   - `tools/`, `tests/`, `extras/`, `external/` — auxiliary trees.
   Each layer gets a one- to three-sentence description anchored to a
   specific representative file.
4. `## Compilation request lifecycle` — describe (without becoming a
   pipeline document) the high-level objects that flow through
   compilation: `CompileRequest`, `TranslationUnitRequest`,
   `EntryPointRequest`, `TargetRequest`, `Linkage`, `Module`, `Session`,
   `IComponentType`. Cite where they are declared
   ([slang-compile-request.h](../../../source/slang/slang-compile-request.h),
   [slang-module.h](../../../source/slang/slang-module.h),
   [slang.h](../../../include/slang.h)).
5. `## Where the public API lives` — point at
   [include/slang.h](../../../include/slang.h) and explain that the
   COM-style interfaces it declares (`ISession`, `IModule`,
   `IComponentType`, ...) are the stable surface; everything in
   `source/` is implementation.
6. `## Reading guide` — link to the rest of the LLM-generated tree:
   [module-map.md](module-map.md), [dependency-graph.md](dependency-graph.md),
   [../pipeline/overview.md](../pipeline/overview.md), and the
   cross-cutting docs.

## Quality checklist (in addition to the universal one)

- [ ] Each subsystem listed in `## Top-level decomposition` cites at
      least one concrete file path that exists in the watched paths.
- [ ] No description of a pass, lowering step, or emit backend (those
      belong in the pipeline tree).
- [ ] Document length under 16 KB.
