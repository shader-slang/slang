# Prompt: cross-cutting/core-module.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/generated/design/cross-cutting/core-module.md` — a
description of the bundled standard libraries (core module, GLSL
module, standard modules) and the per-target preludes shipped under
`prelude/`.

Audience: a developer adding a built-in function, intrinsic, or per-
target prelude entry.

## Required structure

1. `# Core Module and Preludes` (title)
2. `## What ships with the compiler` — three families:
   - The **core module** (`source/slang/core.meta.slang`,
     `hlsl.meta.slang`, `glsl.meta.slang`, `diff.meta.slang`,
     embedded by `source/slang-core-module/`,
     `source/slang-glsl-module/`).
   - The **standard modules** (`source/standard-modules/`).
   - The **target preludes** (`prelude/*.h`).
3. `## Core module` — describe how the `*.meta.slang` files are
   compiled at compiler startup into an embedded module, and that
   they introduce types (`int`, vector / matrix types), intrinsics,
   and `__target_intrinsic` mappings to per-target functions. Cite
   [core.meta.slang](../../../../source/slang/core.meta.slang) and the
   embedding glue in
   [slang-embedded-core-module.cpp](../../../../source/slang-core-module/slang-embedded-core-module.cpp).
4. `## GLSL module` — same treatment for
   [glsl.meta.slang](../../../../source/slang/glsl.meta.slang) and
   [slang-embedded-glsl-module.cpp](../../../../source/slang-glsl-module/slang-embedded-glsl-module.cpp).
5. `## Standard modules` — list each module under
   [standard-modules/](../../../../source/standard-modules) (including
   `neural`) with one-line descriptions; cite their README if present.
6. `## Preludes` — describe each `prelude/*.h` and which target it
   serves. The preludes are emitted alongside textual target output
   (HLSL, GLSL, CUDA, C++, Torch) so the generated code can compile
   with the downstream toolchain.
7. `## Building the core module` — note the cmake option
   `SLANG_EMBED_CORE_MODULE` and explain the difference between
   embedded and out-of-line compilation of the core module (cite
   [CLAUDE.md](../../../../CLAUDE.md) — for context only, do not copy).

## Quality checklist (in addition to the universal one)

- [ ] Every meta-slang file in the watched paths is mentioned.
- [ ] Every prelude header in `prelude/` is mentioned.
- [ ] No invented intrinsic names; cite the watched paths instead of
      enumerating intrinsics in detail.
- [ ] Document length under 24 KB.
