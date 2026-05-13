# Prompt: pipeline/06-emit.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/pipeline/06-emit.md` — a description of
how the legalized IR is turned into target code (HLSL, GLSL, SPIR-V,
Metal, WGSL, C++/CUDA, Torch, VM bytecode, ...).

Audience: a developer adding or modifying a target backend.

## Required structure

1. `# Code Emission` (title)
2. `## Inputs and outputs` — legalized IR → target source / binary
   artefact.
3. `## Emit dispatcher` — describe how
   [slang-emit.cpp](../../../source/slang/slang-emit.cpp) selects the
   right backend for each `TargetRequest` (cite the function that
   chooses).
4. `## Backends` — one level-3 subsection per emit backend, each
   citing its files in the watched paths and stating the target it
   produces:
   - HLSL ([slang-emit-hlsl.cpp](../../../source/slang/slang-emit-hlsl.cpp))
   - GLSL ([slang-emit-glsl.cpp](../../../source/slang/slang-emit-glsl.cpp))
   - SPIR-V ([slang-emit-spirv.cpp](../../../source/slang/slang-emit-spirv.cpp))
   - Metal ([slang-emit-metal.cpp](../../../source/slang/slang-emit-metal.cpp))
   - WGSL ([slang-emit-wgsl.cpp](../../../source/slang/slang-emit-wgsl.cpp))
   - C++ ([slang-emit-cpp.cpp](../../../source/slang/slang-emit-cpp.cpp))
   - CUDA ([slang-emit-cuda.cpp](../../../source/slang/slang-emit-cuda.cpp))
   - Torch ([slang-emit-torch.cpp](../../../source/slang/slang-emit-torch.cpp))
   - LLVM ([slang-emit-llvm.cpp](../../../source/slang/slang-emit-llvm.cpp))
   - VM bytecode ([slang-emit-vm.cpp](../../../source/slang/slang-emit-vm.cpp))
   - Slang round-trip ([slang-emit-slang.cpp](../../../source/slang/slang-emit-slang.cpp))
   Mention the `c-like` shared base
   ([slang-emit-c-like.cpp](../../../source/slang/slang-emit-c-like.cpp))
   that the textual backends inherit from.
5. `## Source-writer abstraction` —
   [slang-emit-source-writer.h](../../../source/slang/slang-emit-source-writer.h)
   and how it tracks indentation, source locations, and `#line`-style
   directives.
6. `## Operator precedence and parenthesization` — pointer to
   [slang-emit-precedence.h](../../../source/slang/slang-emit-precedence.h).
7. `## Preludes` — briefly explain the per-target preludes that emitted
   code includes (`slang-cpp-prelude.h`, `slang-cuda-prelude.h`,
   `slang-hlsl-prelude.h`, `slang-torch-prelude.h`,
   `slang-emit-hlsl-prelude.cpp`, `slang-emit-metal-prelude.cpp`).
   Link [../cross-cutting/core-module.md](../cross-cutting/core-module.md)
   for prelude content.
8. `## Adding a new backend` — short checklist of the touch points
   (new emit file, register in the dispatcher, prelude, capability
   bits, tests).

## Quality checklist (in addition to the universal one)

- [ ] Every backend in the watched paths gets a subsection.
- [ ] No subsection invents a backend that does not have a
      `slang-emit-*.cpp` file in the watched paths.
- [ ] Document length under 32 KB.
