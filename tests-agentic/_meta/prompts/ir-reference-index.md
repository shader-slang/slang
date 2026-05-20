# Prompt: tests-agentic/ir-reference/index/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/ir-reference/index/`,
anchored to
[`docs/llm-generated/ir-reference/index.md`](../../../docs/llm-generated/ir-reference/index.md).

Audience: nightly CI. The bundle exercises the **subtree-level
orientation** claims made by `ir-reference/index.md`. The index
doc is intentionally narrow: most of its content is _pointers_ to
peer pages (`types`, `values`, `structure`, `control-flow`,
`generics-and-existentials`, `resources-and-atomics`,
`differentiation`, `decorations`, `metadata`, `misc`). The
per-family opcode catalogs belong to the peer bundles. This
bundle therefore must be **small** and focus on the
**cross-cutting IR claims** the index doc makes that no single
peer page is responsible for.

The bundle target is **3–8 tests**. If you cannot anchor a claim
to text in the index doc itself, route it to a peer bundle via
`## Out of scope` (e.g. `see ir-reference/structure`) and **do
not** duplicate a peer claim here.

## What counts as a cross-cutting index claim

A claim is in scope for this bundle iff the index doc itself
asserts it. The index doc's load-bearing claims are:

1. **Family taxonomy** (`#family-taxonomy`): every concrete opcode
   belongs to exactly one of the ten families, and a small set of
   opcodes appears in two pages because they play two roles. The
   observable corollary at the IR level is: a natural surface
   construct lowers to opcodes from the family the index assigns
   to it (e.g. arithmetic → values, branches → control-flow,
   `struct` decl → structure, `[shader("...")]` → decorations).
2. **Family hierarchy is sourced from the Lua catalog**
   (`#name-resolution`-style "Every concrete opcode declared in
   slang-ir-insts.lua appears in a family page"). The observable
   corollary is: opcode names that appear in the index's table
   actually appear in IR dumps of natural code.
3. **AST-to-IR mapping** (`#how-ast-nodes-lower-to-ir`): each
   family page's `AST origin` column comes from `visit*` member
   functions in `slang-lower-to-ir.cpp`. The observable corollary
   is: a named AST construct (e.g. `VarDecl`, `InfixExpr`) emits
   the named IR opcode (`Var`, `Add`/`Sub`/`Mul`) in a `-dump-ir`
   text emit.
4. **Synthesized / no-AST-origin opcodes**
   (`#how-ast-nodes-lower-to-ir`): "Many opcodes have no direct
   AST source: they are produced by IR passes ... and may even be
   retired before code emission." The observable corollary is:
   IR-pass-produced opcodes (e.g. `specialize` after
   specialization runs) appear in a `-dump-ir` even though no
   visitor put them there.
5. **Decorations are attached to instructions, not a separate
   instruction stream** (`#family-taxonomy` row for the
   Decoration family; reinforced in the `decorations.md` link).
   Observable as `[decorationName(...)]` appearing on the same
   IR line as the host opcode in a dump.
6. **Cross-cutting topics** (`#cross-cutting-topics`): the index
   names `ir-instructions.md` as the canonical conventions doc
   (schema, op-flag bits, hoistable/global dedup) and
   `04-ast-to-ir.md` as the lowering-pipeline doc. These are
   pointer rows and **not** an in-scope claim source for this
   bundle; route observations to those subtrees via
   `## Out of scope`.

Pick **3–8** claims from items 1–5. Prefer claims that exercise
the **composition** of two or more families in one IR dump,
since the per-family details belong to peer bundles.

## What is NOT in scope for this bundle

The index doc explicitly delegates these to peer pages. Route
them to peer bundles in `## Out of scope`; do not write tests
here:

- Per-opcode shape and operand catalog for type opcodes
  (`Type`, `IntType`, `StructuredBufferType`, …) — see
  `ir-reference/types`.
- Per-opcode catalog for arithmetic, conversions, memory ops,
  aggregate constructors, constexpr arithmetic — see
  `ir-reference/values`.
- `module`, `func`, `generic`, `global_var`, `global_param`,
  `struct`, `interface`, `witness_table`, `key`, `field`,
  `lookupWitness` (structural role) — see `ir-reference/structure`.
- `block`, `param` (block-parameter / phi role), branches,
  terminators, `Require*` markers — see
  `ir-reference/control-flow`.
- `specialize`, existential pack/unpack, RTTI, type-flow
  dispatchers — see `ir-reference/generics-and-existentials`.
- Image/buffer/sampler ops, atomics, barriers, raytracing,
  wave intrinsics, cooperative-matrix/vector — see
  `ir-reference/resources-and-atomics`.
- Differential pairs, autodiff, reverse-mode contexts — see
  `ir-reference/differentiation`.
- Decoration catalog (the ~180 individual decoration opcodes) —
  see `ir-reference/decorations`. The index bundle may anchor
  the **fact** that decorations attach to instructions, but
  must not catalog individual decoration semantics here.
- `Layout`, `Attr`, `Debug*`, `SPIRVAsmOperand` — see
  `ir-reference/metadata`.
- `nop`, `Unrecognized`, type queries, size/alignment, storage
  casts, liveness markers, descriptor heaps, kernel launch —
  see `ir-reference/misc`.

If you find yourself writing a test whose sole purpose is a
single-family per-opcode observation, stop and route it to the
peer bundle.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`,
   including a `## Out of scope` section listing the peer
   bundles for the topics not covered here.
2. **3–8 `.slang` tests**, each anchored to an anchor in the
   index doc (`#ir-reference`, `#family-taxonomy`,
   `#how-ast-nodes-lower-to-ir`). The `#pages`,
   `#cross-cutting-topics`, and `#how-to-navigate` anchors are
   pure pointer sections; do not cite them as the primary
   anchor.
3. Coverage rules:
   - At least one **family-taxonomy positive** test: a natural
     surface construct produces opcodes from the family the
     index assigns to it, observable in `-dump-ir`.
   - At least one **AST-to-IR mapping** test: a named AST
     construct lowers to the named opcode (e.g. arithmetic →
     `add`/`mul`, var decl → `var`), observable in `-dump-ir`.
   - At least one **decorations-attach-to-instructions** test:
     a decoration appears on the same IR line as its host
     opcode in `-dump-ir` (the host can be any family; the
     test is about the attachment claim, not any one
     decoration's semantics).
   - **No more than 2 tests** for any single index anchor; the
     bundle is breadth-over-depth.

4. Naming: `<composition>-<axis>.slang`, e.g.
   `family-taxonomy-arithmetic-is-values.slang`,
   `ast-mapping-var-decl-emits-var.slang`,
   `decoration-attaches-to-host-inst.slang`,
   `synthesized-specialize-from-pass.slang`. Avoid name
   collisions with sibling IR-reference bundles.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/ir-reference/index.md`

Secondary (allowed citations; use sparingly and only when the
index doc explicitly references them):

- `docs/llm-generated/cross-cutting/ir-instructions.md`
- `docs/llm-generated/pipeline/04-ast-to-ir.md`
- `docs/llm-generated/pipeline/05-ir-passes.md`

If you would cite anything else, stop and record a doc-gap
finding in `BUNDLE.md`.

## Test directives

Cross-cutting IR-orientation claims are observed almost
exclusively by IR dump. Use `-dump-ir` with a text target and
`-o /dev/null` per `_common.md`:

- Family-taxonomy / AST-to-IR mapping → `//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null`
  (or another text target). Pin opcode names with FileCheck.
- Decoration-attachment → same form, FileCheck for the
  `[decoration(...)]` token on the same line as the host opcode
  name.
- Synthesized-opcode → same form, observe an opcode produced by
  an IR pass (e.g. `specialize` after specialization).

Do **not** use any GPU-only directive. Do **not** use
`COMPARE_COMPUTE` for runtime-value claims in this bundle — the
index doc is not about runtime values; it is about IR shape.

## Sibling-bundle anti-duplication

Before writing each test, confirm the observation is not
already made by the sibling bundles under
`tests-agentic/ir-reference/`. If a sibling already exercises
the same exact per-family opcode catalog row, do **not**
duplicate. Either skip the test or change the angle to be
specifically about **composition across families** or
**index-level taxonomy claims**.

Record the duplications you intentionally avoided in
`BUNDLE.md` under `## Sibling-bundle overlap`.

## Drop policy

If you cannot anchor a candidate test to text in the index doc
after **3 attempts** at re-reading the doc, drop the test and
record the would-be claim in `BUNDLE.md` under `## Doc gaps
observed`. The bundle's purpose is orientation; failing to
find a testable index-level claim is itself a useful signal
about the doc.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Bundle has 3–8 tests; not fewer, not more.
- [ ] Every test's `doc_ref` anchor exists in `index.md` (not
      `#pages`, not `#cross-cutting-topics`, not
      `#how-to-navigate`).
- [ ] At least one family-taxonomy test exists.
- [ ] At least one AST-to-IR mapping test exists.
- [ ] At least one decoration-attachment test exists.
- [ ] No test duplicates a sibling-bundle opcode-catalog row.
- [ ] `BUNDLE.md` has `## Out of scope` listing the peer
      bundles that own each delegated topic.
- [ ] `BUNDLE.md` has `## Sibling-bundle overlap` listing
      intentionally-avoided peer claims.
- [ ] Every `-dump-ir` directive pairs with a text target and
      `-o /dev/null` (per `_common.md`).
- [ ] No GPU-only directive.
