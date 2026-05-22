# Prompt: tests-agentic/ir-reference/decorations/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/ir-reference/decorations/`,
anchored to
[`docs/llm-generated/ir-reference/decorations.md`](../../../docs/llm-generated/ir-reference/decorations.md).

Audience: nightly CI. This bundle is the **per-opcode reference**
for the `Decoration` family — the largest single family in the IR
schema (~180 documented opcodes), broken into categories such as
naming/provenance, layout/binding, loop and branch hints,
target-specific intrinsic markers, capability requirements,
interpolation and shader IO, entry-point and stage attributes,
linkage/lifetime, inlining/optimization, autodiff markers,
SPIR-V hints, debug, and miscellaneous.

This bundle does **not** try to enumerate every documented opcode
(180+ rows). It samples representatively across categories,
focusing on opcodes that:

- have a natural, AST-side surface that produces them at
  LOWER-TO-IR (not `(synthesized)` deeper in the pipeline);
- appear in IR-dump output with a stable spelling that FileCheck
  can pin without coupling to mangled-name internals;
- are user-facing rather than internal markers introduced by
  autodiff / specialization / debug passes.

This bundle is adjacent to:

- `cross-cutting/ir-instructions` — the category-level view that
  samples each IR family once.
- `ir-reference/structure` — the structural / hierarchy family
  (`func`, `struct`, `interface`, `witness_table`). That bundle
  already observes a couple of decorations attached to its
  structural parents (`nameHint` on a `func`, `export` on a key);
  this bundle drills the decoration family in its own right.
- `ir-reference/values` — value-producing opcodes. Decorations
  attach to those instructions but are a distinct family.

Anchor each test at the section heading in `decorations.md` that
names the category (`#naming-and-provenance`,
`#layout-and-binding`, `#loop-and-branch-hints`,
`#target-specific-definition-and-intrinsics`,
`#capability-and-availability`, `#interpolation-and-shader-io`,
`#entry-point-and-stage`, `#linkage-and-lifetime`,
`#inlining-and-optimization`, `#other`) or at the per-opcode
"Notable opcodes" sub-section (`#namehint-namehintdecoration`,
`#layout-layoutdecoration`,
`#targetintrinsic-targetintrinsicdecoration`,
`#keepalivedecoration`, `#entrypoint-entrypointdecoration`).

## The translation rule: claims to observations

`decorations.md` lists each decoration opcode in tables under a
category section, naming `Opcode`, `C++ wrapper`, `Operands`,
`Flags`, `AST origin`, and `Summary`. The testable consequences
are:

- **"AST surface X produces decoration Y on inst Z with operand
  shape O"** — compile to a text target with `-dump-ir` and
  FileCheck for the `[opcode(...operands...)]` line on the inst
  it decorates. This is the primary mode.
- **"Decoration Y attaches to a specific parent inst form"** — the
  IR-dump prints the decoration on a line _immediately preceding_
  the decorated inst (`[nameHint("foo")]` / `let %foo : Int = ...`).
- **"Operand types match the schema"** — string-literal operands
  appear quoted (`[nameHint("foo")]`); int-literal operands appear
  as `N : Int` (`[numThreads(8 : Int, 4 : Int, 2 : Int)]`); inst
  operands appear as `%name` references (`[layout(%174)]`).

### Observable claims (write tests for these)

Sample 20–35 decorations across categories, picking 2-3 per
category where feasible. The known-observable decorations from
natural Slang surface code are:

- **Naming and provenance**: `nameHint` on user-named functions,
  fields, parameters, locals; `BuiltinDecoration` on core-module
  insts (visible as `[BuiltinDecoration]` lines in the preamble,
  but anchor on a user symbol instead).
- **Layout and binding**: `layout` on entry-point parameters and
  global params (visible as `[layout(%N)]` after the layout pass —
  for LOWER-TO-IR dumps the `[layout(...)]` line appears on the
  param); `HasExplicitHLSLBinding` on a `uniform` decorated with
  `register(uN)`; `glslLocation` on a field decorated with
  `[vk::location(N)]`.
- **Loop and branch hints**: `loopControl(mode : Int)` on a
  for-loop decorated with `[unroll]` (mode = 0) or `[loop]`
  (mode = 1); `flatten` on `[flatten] if (...)`; `branch` on
  `[branch] if (...)`.
- **Target-specific definition and intrinsics**: `targetIntrinsic`
  attached to core-module function declarations (observable by
  calling a built-in like `pow`, `sin`, etc. and FileCheck-ing
  the dumped core-module portion).
- **Interpolation and shader IO**: `semantic("NAME", -1 : Int)`
  on a field declared with `: SV_Foo` / `: TEXCOORD0`;
  `interpolationMode(N : Int)` on a field declared with `linear` /
  `nointerpolation`.
- **Entry-point and stage**: `entryPoint(profileTag : Int, "name",
  "moduleName")` on a function decorated with `[shader("compute")]`
  / `[numthreads(...)]`; `numThreads(x, y, z)` on a compute entry
  point; `waveSize(N)` on `[WaveSize(N)]`; `earlyDepthStencil` on
  `[earlydepthstencil]`.
- **Linkage and lifetime**: `export("mangled")` on every
  user-declared top-level function / struct / global; `keepAlive`
  added by the front-end on entry points and globals (visible as
  `[keepAlive]` on `global_param` decls).
- **Inlining and optimization**: `ForceInline` on
  `[ForceInline]`; `noInline` on `[noinline]`; `noSideEffect` on
  `[__NoSideEffect]`; `readNone` (synthesized for pure built-ins —
  visible as `[readNone]` in the core-module dump).
- **Other**: `method` on a member function declared in a struct;
  `constructor(...)` on the synthesized `__init` of a struct.

### Untested claims (record under the bundle's out-of-scope heading)

- All decorations marked `(synthesized)` in the `AST origin`
  column. Examples that should not be written as tests in this
  bundle: `BinaryInterfaceType`, `PhysicalType`,
  `AlignedAddressDecoration`, `SizeAndAlignment`, `Offset`,
  `optimizableTypeDecoration`, `NonDynamicUniformReturnDecoration`,
  `DefaultValue` (limited surface), `bindExistentialSlots`,
  `DispatchFuncDecoration`, `SpecializationDepthDecoration`,
  `SequentialIDDecoration`, `RTTI_typeSize`, `AnyValueSize`, and
  the entire **Differentiation markers** category
  (`primalInstDecoration`, `diffInstDecoration`,
  `mixedDiffInstDecoration`, `BackwardDerivativePrimalContextDecoration`,
  ...). Autodiff decorations are introduced by the autodiff pass
  on transcribed insts; they appear in the LOWER-TO-IR dump's
  core-module section as part of the AD machinery but the
  decoration → user-AST mapping is not stable enough to test as a
  per-opcode reference from natural code.
- **Mesh / geometry shader decorations** — `[shader("mesh")]`
  and `[shader("geometry")]` entry points compile, but the
  per-decoration spellings (`pointPrimitiveType`,
  `linePrimitiveType`, `vertices`, `indices`, `primitives`,
  `HLSLMeshPayloadDecoration`) require pipeline-specific test
  scaffolding that distracts from the per-opcode reference goal.
  Sample one (`numThreads` on a mesh entry) and record the rest
  as out-of-scope.
- **NVAPI / DLL / CUDA / Python interop decorations**
  (`requiresNVAPI`, `nvapiMagic`, `nvapiSlot`, `dllImport`,
  `dllExport`, `cudaDeviceExport`, `CudaKernel`, `CudaHost`,
  `TorchEntryPoint`, `AutoPyBindCUDA`, `PyExportDecoration`,
  `PyBindExportFuncInfo`) — these require non-default `-target`
  options or specialized core-module markers. Out of scope.
- **Hull / domain / geometry-shader entry-point decorations**
  (`patchConstantFunc`, `maxTessFactor`, `outputControlPoints`,
  `outputTopology`, `partitioning`, `domain`, `maxVertexCount`,
  `instance`, `streamOutputTypeDecoration`) — observable in
  principle but require valid hull/domain/geometry entry-point
  scaffolding; out of scope for this representative bundle.
- **`BackwardDerivativePrimalContextDecoration` / `diffInstDecoration`**
  per the doc's notable-opcodes discussion — observable only on
  transcribed AD instructions after the autodiff pass, with no
  stable user-side anchor.

If you would write a test for a decoration in the categories above,
stop — record it as out-of-scope and move on.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for the
   unobservable items.
2. **20 to 35** `.slang` test files. Aim for **one decoration per
   test**; group two decorations into one test only when they
   share the same minimal surface construct (e.g. `entryPoint` +
   `numThreads` both attach to the same `func %main`).

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/ir-reference/decorations.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/llm-generated/cross-cutting/ir-instructions.md`
- `docs/llm-generated/ir-reference/metadata.md`
- `docs/llm-generated/pipeline/04-ast-to-ir.md`
- `docs/llm-generated/ast-reference/modifiers.md`

If you would cite anything else, stop and record a doc-gap
finding in `README.md`.

## Test directives

Decoration claims are observed through `-dump-ir` against the IR
dump. The standard form used here is:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

Per the universal `_common.md` rule: combine `-dump-ir` with
**`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
goes to stdout uncontaminated by target text. Use
`pipeline_stage=lower` in `//META` — these are LOWER-TO-IR
observations.

Anchor patterns at user-named top-level symbols (`func %main`,
`func %helperInline`, `let %buf`) or at named fields
(`let %x : _ = key`). The IR-dump preamble emits many built-in
decorations (`[BuiltinDecoration]`, `[readNone]`, `[targetIntrinsic]`
on core-module functions); FileCheck patterns must be
positioned _after_ a user-named anchor with `// CHECK:` ordering
so they don't false-match the preamble.

Outputs that escape DCE need to write to an
`RWStructuredBuffer<T>` parameter — purely-internal computations
are removed before the dump. The decorated inst (function,
struct, field, global) must be reachable from the entry point so
the linker keeps it alive.

Use `SV_DispatchThreadID` to defeat constant folding when the
test body invokes a decorated helper; literal-only call arguments
get folded before the call survives to the dump.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ir-reference/decorations.md` (or one of the listed
      secondary docs).
- [ ] Every test uses `-target spirv-asm -dump-ir -o /dev/null
      -entry main -stage compute` per CLAUDE.md.
- [ ] Outputs escape DCE: write to an `RWStructuredBuffer<T>` so
      the decorated declaration stays linked.
- [ ] CHECK patterns anchored at a user-named top-level symbol —
      the IR-dump preamble must not match (the preamble emits its
      own copies of many decorations on core-module functions).
- [ ] No test depends on a GPU.
- [ ] No test asserts a C++ wrapper-struct identity
      (`NameHintDecoration`, `EntryPointDecoration`, etc.) — the
      dump prints the lowercase opcode spelling
      (`nameHint`, `entryPoint`).
- [ ] No test is anchored on a decoration whose `AST origin` is
      `(synthesized)` (record as out-of-scope instead).
- [ ] README.md `## Doc gaps observed` is honest.

## Lessons captured (apply to this bundle as well)

These bite hard in decoration-reference tests:

- `-dump-ir` requires `-target <X>` and `-o /dev/null`.
- The IR dump prefixes user IR with a large preamble that
  contains its own decoration spellings (`[BuiltinDecoration]`,
  `[readNone]`, `[targetIntrinsic]` on core-module functions).
  Anchor patterns at user-named top-level symbols so the
  preamble does not false-match. Use `// CHECK:` (anchored at the
  user symbol) followed by `// CHECK-NEXT:` or `// CHECK-SAME:`
  for the decoration.
- Decorations print on the line **immediately preceding** the
  inst they decorate. The recommended pattern is:
  ```
  // CHECK: [decoration(operands)]
  // CHECK-NEXT: [nameHint("user-name")]
  // CHECK-NEXT: func %user-name ...
  ```
  or anchor on the symbol first and use `// CHECK:` directives
  positioned to find the decoration via FileCheck's forward-scan
  semantics. Tests in this bundle prefer the order
  `[nameHint("name")]` immediately before `func %name : ...` —
  `nameHint` is the most reliably-ordered anchor.
- Mangled identifiers replace `_` with `x5F` (e.g. `helper_inline`
  → `%helperx5Finline`). Use `{{[A-Za-z0-9_]+}}` wildcards in
  export-string patterns where the mangled portion is unstable.
- The `[entryPoint(N : Int, "name", "module")]` decoration's
  third operand is the source-file-derived module name (e.g.
  `"test-decorations-foo"` when the file is
  `test-decorations-foo.slang`). FileCheck for the entry-point
  decoration should wildcard the module-name operand
  (`{{.*}}`) so the test is filename-agnostic.
- The `[loopControl(N : Int)]` mode operand is `0` for `[unroll]`
  and `1` for `[loop]`. Pin the integer in the CHECK pattern.
- The `[interpolationMode(N : Int)]` mode operand is an enum
  ordinal that varies across versions — match
  `[interpolationMode({{[0-9]+}} : Int)]` rather than a literal
  number unless the spec-doc commits to the encoding.
- `register(uN)` on a `uniform` global emits warning E39029
  ("D3D register without Vulkan binding or shift") when the
  target is SPIR-V. This is not an error — the IR dump still
  emits the `[HasExplicitHLSLBinding]` decoration — but it does
  pollute stderr. Tests that observe `HasExplicitHLSLBinding`
  must add a `register(uN, space0)` form _plus_ a
  `[[vk::binding(N, S)]]` attribute to avoid the noise, or
  accept the warning (it does not fail the test).
- `[layout(%N)]` references a `Layout` opcode whose ID number is
  unstable. Match `[layout(%{{[0-9]+}})]` — never a literal
  `%174`.
- `[targetIntrinsic(%capSet, "...")]` is the IR's per-target
  intrinsic encoding. The capability-set operand is a `%N`
  reference into the capability table at the bottom of the dump.
  FileCheck for `[targetIntrinsic(%{{[0-9]+}}, "pow")]` rather
  than pinning the cap-set operand.
- The `[readNone]` decoration on core-module pure built-ins
  appears in the preamble of every dump (because the core module
  is always linked). A test that anchors on `[readNone]` without
  first anchoring on a user symbol will match a built-in
  declaration instead. Prefer to skip `readNone` from this
  bundle's representative sample, or anchor it via a user-defined
  `[__readnone]` if the surface is available.
- `[method]` and `[constructor(true)]` attach to struct methods
  and the synthesized `__init` respectively. The `[constructor]`
  decoration's `true` operand is a `Bool` literal.
- The `[semantic("NAME", -1 : Int)]` second operand is the
  semantic-index integer; `-1` means "no explicit index". Tests
  observing `[semantic(...)]` should pin both operands.
