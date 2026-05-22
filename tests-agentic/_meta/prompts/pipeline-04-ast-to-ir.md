# Prompt: tests-agentic/pipeline/04-ast-to-ir/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/pipeline/04-ast-to-ir/`,
anchored to
[`docs/llm-generated/pipeline/04-ast-to-ir.md`](../../../docs/llm-generated/pipeline/04-ast-to-ir.md).

Audience: nightly CI. The bundle exercises the **AST → IR lowering
stage**: that each documented AST family lowers to the IR construct
the doc names (e.g. `FuncDecl` → `IRFunc` containing `IRBlock`s,
`VarDecl` (local) → `IRVar`, `BinaryExpr` → `IRAdd`/`IRMul`/`IREq`),
that structured control-flow lowers to the documented terminators
(`ifElse`/`loop`/`switch`/`return_val`), that block-parameter SSA is
used in place of explicit `phi`, that `InvokeExpr` lowers to `call`,
that `MemberExpr` lowers to `FieldExtract`/`FieldAddress` depending on
rvalue/lvalue context, that generics survive lowering as `IRGeneric`
(specialization is **not** performed by lowering — that's an IR pass),
and that user-defined interface conformances become `IRWitnessTable`s.

This is the **lowering** stage, not the IR pass pipeline or emit. Do
not test optimization passes, downstream legalization, or target-
specific lowering decisions — those belong in
`pipeline/04b-pre-link-passes`, `pipeline/05-ir-passes`, or the
target-pipelines bundles. Where the same opcode catalog is the topic,
defer to `cross-cutting/ir-instructions` for the per-opcode catalog
shape; this bundle is anchored at the **AST-side** of the mapping
("which AST node lowers to which IR opcode").

## The translation rule: claims to observations

The source doc's section
[`#mapping-ast-constructs-to-ir`](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)
is the central table. The testable consequences are:

- **"AST family X lowers to IR construct Y"** — compile to a text
  target with `-dump-ir` and FileCheck for opcode Y in the
  `### LOWER-TO-IR:` stage of the dump. This is the primary mode.
- **"X is structurally encoded as Z"** — e.g. block parameters in
  place of `phi`, structured branches with an explicit join operand.
  Anchor at the function and look for `param %{{.*}}` inside the loop
  header, or for the join-block operand on `ifElse(...)` / `loop(...)`
  / `switch(...)`.
- **"Specialization is deferred to IR passes"** — observable as the
  presence of an `IRGeneric` in the dump and a `call specialize(...)`
  at the call site (rather than an inlined or already-specialized
  body).

### Observable claims (write tests for these)

The doc table in `## Mapping AST constructs to IR` names these
AST → IR pairs. Each row is a candidate claim:

- `ModuleDecl` → `IRModule` (every test exercises this implicitly).
- `FuncDecl` → `IRFunc` containing `IRBlock`s. Observe `func %<name>`
  and `block %{{.*}}` lines in the dump.
- `VarDecl` (global) → `IRGlobalVar`. Observe `let %<name> : ... =
  global_param` (for `uniform` globals — the doc lists `global_param`
  for module-scope uniforms; plain mutable file-scope `static` storage
  is rare in Slang shader code).
- `VarDecl` (local) → `IRVar` inside a block. Observe `let %<name>
  : Ptr(<T>) = var` for a local whose address is taken (struct
  fields, array elements). Per the `_common.md` lessons, trivial
  locals are eliminated; use a struct local so the `var` survives.
- `StructDecl` → `IRStructType` with `IRStructField` children.
  Observe `struct %<Name>` followed by `field(%{{.*}}, <T>)` rows.
- `InterfaceDecl` → `IRInterfaceType` plus per-method requirement
  insts. Observe `interface_req_entry(%{{.*}}, Func(...))` and an
  `interface(...)` line.
- `GenericDecl` → `IRGeneric`. Observe `generic %{{.*}}` followed by
  a parameter list and an inner `func`.
- Structured branches: `IfStmt` → `ifElse(...)`, `WhileStmt` /
  `ForStmt` → `loop(...)`, `SwitchStmt` → `switch(...)`. The join
  point is an **explicit operand** on the terminator.
- `ReturnStmt` → `IRReturn` (spelled `return_val(...)` in the dump,
  with `void_constant` for `void` returns).
- `BinaryExpr` arithmetic / comparison → `IRAdd` / `IRMul` / `IREq`
  etc. (Constant-folding caveat from `_common.md`: feed non-constant
  operands via `uniform`.)
- `InvokeExpr` → `IRCall` (spelled `call %<callee>(...)` in the
  dump).
- `MemberExpr` → `FieldAddress` / `FieldExtract` (spelled
  `get_field_addr` / `get_field` in the dump). lvalue vs rvalue
  distinction matters: write `p.x = ...` to see `get_field_addr`,
  read `p.x` from a value to see `get_field`.
- `LiteralExpr` → constant inst — visible in the dump as the literal
  appearing in-line (e.g. `add(%a, 1 : Int)`).
- Witness tables → `IRWitnessTable` with `witness_table_entry(...)`
  rows.

The doc also says:

- "Phi-style joining is encoded as block parameters (`IRParam` at the
  start of a block) rather than explicit `phi` instructions; branches
  to a block carry the parameter values as arguments." Observe
  `block %{{.*}}(... param %{{.*}})` inside a loop header.
- "Specialization itself is **not** performed during lowering."
  Observe `call specialize(%<generic>, <Type>)(...)` rather than an
  inlined body.
- Entry points carry `entryPoint(...)` decoration (cross-cutting
  with `ir-instructions`; cite that bundle's anchor for the catalog
  side).

### Not testable through slangc (record under `## Untested claims`)

- Internal lowering visitor structure (e.g. which C++ visitor method
  handles which AST node). This is an implementation detail of
  `slang-lower-to-ir.cpp`, not user-observable.
- `IRBuilder` hash-consing decisions — the dump shows the post-
  dedup result, not the decision path.
- The `kIROpFlag_Hoistable` / `kIROpFlag_Global` flag bits — internal
  to `IRInst`.
- Side artefacts on the surrounding `Module` (the entry-point IR
  list, type-conformance bookkeeping, "layout intent" markers).
  These are C++ container contents, not surface-observable through
  slangc directives.
- `generateIRForSpecializedComponentType` and
  `generateIRForTypeConformance` — these are component-type API
  entry points invoked through `Session`, not from the command line.
  A C++ unit test would observe them; an agentic .slang test cannot.
- "Lowering errors flow through `DiagnosticSink`" — true but
  unobservable unless we can name a specific construct that produces
  a diagnostic only at lowering. The doc does not list one.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for the
   unobservable-via-slangc items listed above (the heading is a
   convention shared with `cross-cutting/ir-instructions`).
2. 12 to 25 `.slang` test files. Aim for one observable AST→IR
   mapping per test (or one observable structural-CFG encoding per
   test). Multi-target emit tests are **not** the focus of this
   bundle — most claims are target-independent IR-shape claims, so
   prefer the `-dump-ir` form over per-target SIMPLE directives.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/pipeline/04-ast-to-ir.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off or the test specifically observes the
hand-off boundary):

- `docs/llm-generated/cross-cutting/ir-instructions.md`
- `docs/llm-generated/ir-reference/index.md`
- `docs/llm-generated/ir-reference/types.md`
- `docs/llm-generated/ir-reference/values.md`
- `docs/llm-generated/ir-reference/structure.md`
- `docs/llm-generated/pipeline/03-semantic-check.md`

If you would cite something else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for _verification only_

You may look at these to confirm an opcode spelling (the exact name
as it appears in `-dump-ir`). You may **not** mine them for
behavioral claims the doc does not make.

- `source/slang/slang-lower-to-ir.h`
- `source/slang/slang-lower-to-ir.cpp`
- `source/slang/slang-ir.h`
- `source/slang/slang-ir.cpp`

## Test directives

Lowering claims are best observed through `-dump-ir`. The standard
form used here is:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
```

Per the universal `_common.md` rule: combine `-dump-ir` with
**`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
goes to stdout uncontaminated by target text.

Useful conventions when writing CHECK patterns:

- Anchor on `func %main` (or the user-named function) — the IR dump
  preamble is large (core-module imports, capability sets,
  differentiation glue). The pattern `// CHECK-LABEL: func %main`
  cuts through it.
- Use `{{.*}}` or `%{{.*}}` for SSA IDs (`%3`, `%tmp_42`).
- Match opcode names with an opening paren (`add(`, `call`, `loop(`,
  `ifElse(`, `switch(`, `store(`, `get_field(`, `get_field_addr(`).
- The opcode name in the dump is the **lowercase Lua key** — `add`,
  `var`, `call`, `block`, `param`, `loop`, `ifElse`,
  `unconditionalBranch`, `return_val`, `get_field`,
  `get_field_addr`, `rwstructuredBufferGetElementPtr`, `specialize`,
  `witness_table`, `witness_table_entry`.

A handful of claims also have a stable text-emit consequence (e.g.
the entry-point decoration becoming a target-specific entry-point
keyword). Use multi-target SIMPLE in those cases, but the **default
for this bundle is single `-dump-ir` directive per test**, because
the doc's central claim is "AST-to-IR mapping", which is target-
independent.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `pipeline/04-ast-to-ir.md` (or one of the allowed secondary
      docs listed above).
- [ ] Every `-dump-ir` test uses `-target spirv-asm -dump-ir -o
      /dev/null -stage compute -entry main` (or an equivalent text
      target), per CLAUDE.md.
- [ ] Non-constant operands for arithmetic / comparison claims —
      `uniform` globals or `SV_DispatchThreadID`-derived values —
      so constant folding does not collapse the operator.
- [ ] Outputs escape DCE: write to an `RWStructuredBuffer<T>` so
      the computation is not removed.
- [ ] For `IRVar` observation, use a **struct local** (or array
      local) so the address-of pattern survives initial lowering;
      a plain `int x = a;` does not.
- [ ] CHECK patterns anchored at `func %main` or a user-named
      function — the IR-dump preamble must not match.
- [ ] No test depends on a GPU.
- [ ] No test asserts an opcode's C++ wrapper-struct identity
      (`IRAdd` etc.) — the dump shows the lowercase Lua name.
- [ ] No test asserts on specialized / inlined / optimized IR —
      specialization is an IR pass, not lowering. Observe
      `specialize(...)` at call sites, not a fully-specialized
      function body.
- [ ] No test was written by inspecting an uncovered source line.
      If you find yourself thinking "this would cover the branch at
      `slang-lower-to-ir.cpp:NNNN`", stop and re-read the doc.
- [ ] README.md `## Doc gaps observed` is honest. If you wanted a
      test you could not anchor, write down which claim the doc
      would need to add.

## Lessons captured (apply to this bundle as well)

These are restated from `_common.md` because they bite hard in
lowering tests:

- `-dump-ir` requires `-target <X>` (else compile stops early) and
  `-o /dev/null` (else target text mixes with IR on stdout).
- Constant folding collapses literal arithmetic before any IR is
  emitted — to observe `add(%a, %b)` use `uniform` operands.
- Trivial locals are eliminated during lowering. The `var` opcode
  survives when the local is a **struct** accessed by field address,
  or an **array** indexed elementwise.
- DCE strips locally-unused code before emit (not before
  `### LOWER-TO-IR:`, but the dump's later stages will). Write to
  an `RWStructuredBuffer` so the value escapes — and so the same
  test file can carry an emit-target SIMPLE directive later if
  needed.
- The dump prefixes user IR with a substantial preamble (Annotation,
  capability sets, differentiation witness tables, core module
  symbols). Anchor your FileCheck patterns at `func %<entry>` or a
  user-named callee.
- `entryPoint` and `nameHint` decorations have already been verified
  in the `cross-cutting/ir-instructions` bundle; this bundle should
  not duplicate them. Anchor to the AST-side claim ("a `FuncDecl`
  marked as entry point lowers to `IRFunc` carrying the
  decoration") only if a unique angle is testable.
