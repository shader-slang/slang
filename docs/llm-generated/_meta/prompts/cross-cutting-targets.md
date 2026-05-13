# Prompt: cross-cutting/targets.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/cross-cutting/targets.md` — a
description of the supported compilation targets, the capability and
profile system, and how target choice affects the IR and emit
pipelines.

Audience: a developer adding a new target, debugging a target-specific
codegen issue, or trying to understand why a feature is rejected
under a particular profile.

## Required structure

1. `# Targets, Capabilities, and Profiles` (title)
2. `## Targets` — table listing every target with its representative
   emit file and produced artefact:

   ```markdown
   | Target | Format | Emit file |
   | --- | --- | --- |
   | DXIL via DXC | HLSL text → DXIL | [slang-emit-hlsl.cpp](../../../source/slang/slang-emit-hlsl.cpp) |
   | SPIR-V (direct) | SPIR-V binary | [slang-emit-spirv.cpp](../../../source/slang/slang-emit-spirv.cpp) |
   | ... | ... | ... |
   ```

3. `## Capability system` — describe the capability set declared in
   [slang-capabilities.capdef](../../../source/slang/slang-capabilities.capdef)
   and how the runtime computes capability lattices in
   [slang-capability.cpp](../../../source/slang/slang-capability.cpp).
   Reference [../../design/capabilities.md](../../design/capabilities.md)
   only as further reading.
4. `## Profiles` — how a target combines a code-generation format with
   a profile (e.g. `Shader Model 6.6`,
   `Vulkan 1.3 + capabilities`). Cite
   [slang-profile.cpp](../../../source/slang/slang-profile.cpp) /
   [slang-profile.h](../../../source/slang/slang-profile.h) if
   present in the watched paths.
5. `## How target choice affects IR` — the IR is largely target-
   agnostic, but several passes are conditional on target. Point at
   [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) and call
   out the family of `slang-ir-{spirv,glsl,hlsl,metal,wgsl,cuda}-*.cpp`
   passes.
6. `## Adding a new target` — checklist:
   - new emit backend file under `source/slang/slang-emit-<target>.cpp`,
   - dispatcher entry in
     [slang-emit.cpp](../../../source/slang/slang-emit.cpp),
   - prelude in `prelude/`,
   - capability bits in
     [slang-capabilities.capdef](../../../source/slang/slang-capabilities.capdef),
   - target-specific IR legalization passes,
   - test fixtures under `tests/`.

## Quality checklist (in addition to the universal one)

- [ ] Every target row points at an emit file that exists in the
      watched paths.
- [ ] The capability section does not invent profile names; verify
      against the .capdef file.
- [ ] Document length under 24 KB.
