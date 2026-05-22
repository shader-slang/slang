# Prompt: tests-agentic/pipeline/04c-layout-ir/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/pipeline/04c-layout-ir/`,
anchored to
[`docs/llm-generated/pipeline/04c-layout-ir.md`](../../../docs/llm-generated/pipeline/04c-layout-ir.md).

Audience: nightly CI. The bundle exercises the **per-target layout IR
module** built by `TargetProgram::createIRModuleForLayout` in
[slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp).
The layout-IR module is a sibling of the executable IR module
documented in [04b-pre-link-passes.md](04b-pre-link-passes.md); its
only job is to carry `IRLayoutDecoration`s on stub globals and
entry-point functions for one specific target's chosen layout rules.

The source doc organises the construction sequence into:

- **Per-global-parameter steps** — for each `varLayout` in the global
  struct: materialise a stub `IRGlobalVar`, lower its `IRVarLayout`,
  and attach an `IRLayoutDecoration`; feed the result to the module-
  level `IRStructTypeLayout` builder.
- **Global-scope type layout** — `_lowerTypeLayoutCommon` produces an
  `IRStructTypeLayout` for the whole global scope, wrapped in an
  `IRParameterGroupTypeLayout` when the global scope is a constant
  buffer / push-constant block. The result is attached to the module
  instance via `addLayoutDecoration`.
- **Per-entry-point steps** — for each `entryPointLayout`: skip if no
  AST is available or the generic is unspecialised, lower the function
  type and materialise a stub `IRFunc`, attach an `import` decoration
  if the stub is otherwise unlinked, forward SPIR-V / Metal capability
  atoms, lower `IREntryPointLayout`, and attach an `IRLayoutDecoration`.
- **Optional obfuscation pass** — when `shouldObfuscateCode()` is set,
  strip name hints and source locs and DCE while keeping exports and
  layouts alive.
- **Cache and reuse** — the result is cached on
  `TargetProgram::m_irModuleForLayout` (lazy build).

## The translation rule: claims to observations

The layout IR module itself does **not** get its own `### LAYOUT-IR:`
dump section. Instead, after the per-translation-unit executable IR is
emitted (`### LOWER-TO-IR:`), the layout module is merged into the
post-link IR before the post-link pipeline runs. The **first
`### AFTER <pass>:` block** in `-dump-ir` output (e.g.
`### AFTER validateAndRemoveAssumeAddress:`) is therefore the earliest
post-link snapshot, and the layout decorations attached by
`createIRModuleForLayout` are visible there as:

- `[layout(%N)]` decorations on stub globals (`global_param`,
  `let ... = global_param`) and on the entry-point `func`.
- `EntryPointLayout(...)` instructions naming the function's
  `varLayout` / `typeLayout` operands (the `IREntryPointLayout` from
  step 7 of the per-entry-point table).
- `parameterGroupTypeLayout(...)` when the global scope is wrapped in
  a constant buffer / push-constant block (the global-scope
  `IRParameterGroupTypeLayout`).
- `structTypeLayout(...)` for the global scope's struct layout (the
  return of `_lowerTypeLayoutCommon`).
- `structFieldLayout(...)` per global parameter, paired with
  `varLayout(...)` operands carrying per-target binding / offset / size
  (the per-global-parameter `lowerVarLayout` outputs).
- `offset(...)` / `size(...)` operands inside those `varLayout`
  entries.

By contrast, the `### LOWER-TO-IR:` block (pre-link executable IR) has
**no** `[layout(...)]` decorations and **no** `EntryPointLayout` /
`structTypeLayout` instructions. That asymmetry is the primary
observation surface for the bundle: a `CHECK-NOT` over the
`### LOWER-TO-IR:` block confirms the executable IR is layout-free,
and a `CHECK` over the first `### AFTER ...:` block confirms the
layout-IR module has been merged in.

### Observable claims (write tests for these)

#### Per-global-parameter steps

- A simple **`RWStructuredBuffer<int> buf;`** global produces a
  `[layout(%N)]` decoration on the stub `global_param` in the first
  `### AFTER ...:` block, where the referenced `varLayout` operand
  encodes the buffer's per-target binding.
- The **per-target `varLayout`** carries an `offset(...)` operand —
  same source, different target (SPIR-V vs HLSL), different
  numerical operands — confirming layout is per-`TargetProgram`.
- **Multiple distinct global parameters** each get their own
  `[layout(...)]` decoration; the `IRStructTypeLayout` for the
  global scope groups them via `structFieldLayout(field, varLayout)`
  entries.

#### Global-scope type layout

- When the global scope is wrapped in a **`cbuffer`** /
  **`ConstantBuffer<T>`**, a `parameterGroupTypeLayout(...)`
  instruction appears in the first `### AFTER ...:` block (rather
  than a plain `structTypeLayout`). The doc names this as the
  `IRParameterGroupTypeLayout` path.
- The **module instance** itself carries a `varLayout(...)` operand
  at the top of the layout block, anchoring the global-scope layout.
  (The doc names this as the
  `addLayoutDecoration(irModule->getModuleInst(), irGlobalScopeVarLayout)`
  call.)

#### Per-entry-point steps

- The **entry-point `func %main`** carries a `[layout(%N)]`
  decoration in the first `### AFTER ...:` block whose operand
  resolves to an `EntryPointLayout(...)` instruction. The
  `IRLayoutDecoration` on the entry-point function is what the
  reflection API queries.
- The **`EntryPointLayout(...)` instruction** names two operands —
  a `varLayout` for the function parameters and a `varLayout` for
  the result — corresponding to `entryPointLayout->parametersLayout`
  and `entryPointLayout->resultLayout` in the doc.
- The **mangled-name `import`** on the layout-module stub is the
  same name as the executable-IR `export` for the entry point. The
  doc names this in step 5 of the per-entry-point table
  (`addImportDecoration(irFunc, mangledName)`). Observable as the
  matching `[export("_SR..._main...")]` decoration that survives
  link.

#### What this module is not

- The **`### LOWER-TO-IR:` block (pre-link executable IR) contains no
  `[layout(...)]` decorations and no `EntryPointLayout` /
  `structTypeLayout` instructions.** That is the doc's "executable IR
  is layout-free" claim from "What this module is not".
- **No mandatory optimization passes run on the layout module.** The
  layout module's per-function body is empty (stubs), so the
  `constructSSA` / SCCP / CFG-simplify / DCE sequence does not
  produce visible effects on the layout side. (Verifiable indirectly:
  the entry-point `func %main` body in the executable side is what
  carries the SSA work — the layout-side `func %main` is a stub. Not
  worth a dedicated test slot; documented under
  `## Untested claims` if needed.)

### Conditional / target-specific gates

- **Capability decoration filter** (lines 15462-15463 of
  `createIRModuleForLayout`). The doc states that only SPIR-V and
  Metal layout-module entry points carry
  `IRRequireCapabilityAtomDecoration`s; HLSL, WGSL, and CUDA do not.
  This is observable in principle by comparing the post-link IR for
  the same source across targets, **but** the capability
  decorations from the layout module are merged with the
  executable-side ones and the post-link pipeline rewrites them
  heavily before the first `### AFTER ...:` snapshot. Treat the
  capability filter as a doc claim worth a single test only if a
  clean surface signal exists; otherwise record as out-of-scope.

### Not testable through slangc (record under `## Untested claims`)

- The **lazy construction / cache reuse** of `m_irModuleForLayout`
  (a second `getOrCreateIRModuleForLayout` call returns the cached
  module). There is no user-visible signal of cache hit vs miss
  through `slangc`; the only observation is correctness on multiple
  compilations, which is not bundle-testable.
- The **optional obfuscation pass** on the layout module. The pass
  is gated on `linkage->m_optionSet.shouldObfuscateCode()` and the
  user-visible effect couples to obfuscated source-map machinery
  outside this bundle's scope.
- The `materialize` failure path (`SLANG_UNEXPECTED("unhandled
  value flavor")`). The doc names this as a hard crash for
  corrupted per-module IR caches; it is not user-reachable from
  source.
- The `SLANG_ASSERT(m_layout)` assertion for callers that bypass
  `getOrCreateIRModuleForLayout`. Not reachable from user source.
- The **`buildMangledNameToGlobalInstMap`** call at the end of the
  function — same as in 04b, no user-observable consequence.
- The **per-`TargetProgram` independence** of layout modules (two
  layout modules in memory when compiling for two targets). A
  single `slangc` invocation builds one `TargetProgram`; the
  cross-target independence is observed by running two invocations
  and comparing outputs, which is what the per-target `varLayout`
  operand tests already do indirectly.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for unobservable
   items.
2. 8–14 `.slang` test files. All are
   `//TEST:SIMPLE(filecheck=CHECK):-target <T> -dump-ir
   -o /dev/null -entry main -stage compute` for `T` in
   `{spirv-asm, hlsl}` (occasionally also `glsl` or `metal` when
   the claim is target-comparison). The `size_cap_files` cap is 30;
   staying in the 8-14 range keeps each test laser-focused on one
   layout-module consequence.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/pipeline/04c-layout-ir.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for _verification only_

- `source/slang/slang-lower-to-ir.cpp` (`createIRModuleForLayout`,
  for the actual construction sequence and surrounding context)
- `source/slang/slang-target-program.h` (cache field, lazy accessor)
- `source/slang/slang-parameter-binding.cpp` (produces `ProgramLayout`)

You may **not** mine these for behavioural claims that the doc
does not make.

## Test directive

Default required directive for layout-IR observation tests:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

or, when verifying the same observation on a different target:

```
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute
```

Anchor `CHECK` patterns at the **first `### AFTER ...:` block** (the
earliest post-link snapshot, which contains the layout-IR module
merged in). The `### LOWER-TO-IR:` block (pre-link executable IR) is
the right place for `CHECK-NOT` patterns asserting the absence of
layout decorations, since the layout module has not been merged at
that point. Note that FileCheck patterns are evaluated in order; use
`// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:` to pin
subsequent `CHECK` lines to the post-link snapshot. Be aware that
identifiers inside the IR dump (`%N`) are renumbered at each pass
boundary, so use FileCheck regex-variable captures
(`[[#%layout_id:]]`) when referencing the same instruction across
multiple `CHECK` lines.

## Lessons captured for layout-IR tests

These are in addition to the universal lessons in `_common.md`.

- The layout IR module is **not** exposed under its own `### LAYOUT-IR:`
  header. Its effects are visible as `[layout(...)]` decorations and
  `EntryPointLayout` / `structTypeLayout` / `varLayout` instructions
  in the first post-link `### AFTER ...:` block of `-dump-ir` output.
- The `### LOWER-TO-IR:` block (pre-link executable IR) is layout-free.
  Use this as the negative anchor: `CHECK-NOT: \[layout` between the
  `### LOWER-TO-IR:` label and the first `###` separator is the
  cleanest way to assert layout absence in the executable IR.
- **Per-target `offset` / `size` operands differ**: a global with
  `RWStructuredBuffer<int>` gets `offset(9 : Int, ...)` on SPIR-V
  (kind 9 = descriptor binding) and a different kind on HLSL
  (`register(uN)` lowering). Don't FileCheck the numeric kind across
  targets — instead, assert the **structure** (`varLayout(...)` with
  some `offset(...)` operand) is present.
- The `cbuffer { ... }` block produces `parameterGroupTypeLayout(...)`
  in the layout instructions, whereas a plain `RWStructuredBuffer`
  global at module scope produces `structTypeLayout(...)` for the
  global scope plus per-field `varLayout(...)`. The difference is the
  doc's "Global-scope type layout" section.
- The `EntryPointLayout(...)` instruction has two `varLayout(...)`
  operands (one for parameters, one for result). Match its shape
  with two regex-variable captures.
- `[layout(%N)]` decorations exist on **both** the per-global stubs
  and the entry-point function. The same `%N` is also the operand of
  an `EntryPointLayout(...)` or a `varLayout(...)` instruction
  earlier in the block. Use `// CHECK: \[layout(%[[#L:]])\]` followed
  by a back-reference if you want to assert the connection — but
  cross-line variable references over a 20-line gap are brittle.
  Prefer asserting the existence of each piece separately.
- The `CHECK-LABEL: ### LOWER-TO-IR:` anchor is fragile if you also
  want `CHECK-LABEL: ### AFTER ...:` later — FileCheck consumes the
  buffer linearly, so place all `CHECK-NOT` patterns first (bound to
  `### LOWER-TO-IR:`), then a single `CHECK-LABEL: ### AFTER` to move
  the cursor, then the positive `CHECK` lines for the post-link
  snapshot.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `pipeline/04c-layout-ir.md`.
- [ ] All tests use the canonical
      `-target <T> -dump-ir -o /dev/null -entry main -stage compute`
      directive, with `T` chosen to make the claim observable.
- [ ] No test asserts on the order in which the layout-IR
      construction steps run; only post-construction observable
      shape.
- [ ] No test exercises the obfuscation gate or the lazy-cache
      mechanism — both are recorded under `## Untested claims`.
- [ ] No test asserts numeric `offset` / `size` operand values
      across targets (each target has different numeric encodings;
      structure-level assertions only).
- [ ] `## Doc gaps observed` records layout-IR claims that the doc
      makes but were not testable here (capability filter, cache
      reuse, obfuscation pass).
- [ ] `## Untested claims` records claims that are
      genuinely unobservable through any allowed directive
      (`materialize` failure, lazy cache, multi-`TargetProgram`
      independence within one invocation).
