# Prompt: docs/generated/tests/design/ir-reference/misc/

See [`_common.md`](../../../_meta/prompts/_common.md) for universal rules and
[`_claims.md`](../../../_meta/prompts/_claims.md) for the claim methodology.
Those rules apply to this bundle and override nothing here unless explicitly
noted.

## Target

Produce the test bundle at `docs/generated/tests/design/ir-reference/misc/`,
anchored to
[`docs/generated/design/ir-reference/misc.md`](../../../../design/ir-reference/misc.md).

Audience: nightly CI. This is the **catch-all opcode catalogue** — the opcodes
that no sibling `ir-reference/` page claims. Because the page is a catalogue
rather than a narrative, the coverage strategy follows the doc's own producer
column: the doc names a concrete producer for every row (a `visit*` in
`slang-lower-to-ir.cpp`, a core-module `__intrinsic_op` declaration, a named IR
pass, or **no producer at HEAD**).

The rule that follows:

- Every row whose producer is reachable from portable Slang source gets a test.
- Every row produced only by an IR pass, only by the host/Torch binding path, or
  by nothing at all is enumerated under `## Untested claims` with a reason.

## The translation rule: claims to observations

Three observation points, chosen by what the claim is _about_.

**1. The platform-neutral IR — for "this opcode exists with this shape".**

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
```

Anchor patterns at a user-named function and keep operands symbolic — behind a
generic, or read from a thread id — so neither the constant folder nor DCE
removes the instruction before the CHECK sees it.

**2. The final text emit — for claims the doc states in terms of generated
code.** The work-graph `Barrier` named constants, the `base.Get(index)` record
accessor, the untyped descriptor-heap casts that "should never reach target
code", and the folded `getStringHash` value are all emit-level claims. Fan these
out over every target that can express them (`hlsl`, `spirv-asm`, `metal`,
`wgsl`) rather than asserting them on one.

**3. A computed result — for value-level claims.** Which arm a `PackBranch`
selects, what the type predicates answer, whether two hashes agree: these are
questions about a _result_, not a spelling. Use

```
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
```

with a `//TEST_INPUT:ubuffer(...)` output buffer, or `//TEST:INTERPRET(filecheck=CHECK):`
for a `printf`-shaped check.

### Anchors and what to write for each

| Anchor                                                                                                    | What to test                                                                                                                                                  |
| --------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `#system-opcodes`                                                                                         | The always-present module/system opcodes a trivial compile produces.                                                                                          |
| `#capability-sets`                                                                                        | Capability-set opcodes reachable from a `[require(...)]`-style source shape.                                                                                  |
| `#tensor-and-runtime-helpers`                                                                             | Only the rows reachable without the Torch/host binding path.                                                                                                  |
| `#pack-and-expansion`, `#expand-and-each`                                                                 | Variadic generic expansion: `Expand` / `Each` shape in the dump, and the expanded result value.                                                               |
| `#packbranch`                                                                                             | The parent/child shape **and** which arm is selected at runtime.                                                                                              |
| `#makewitnesspack`                                                                                        | The witness pack produced for a variadic conformance.                                                                                                         |
| `#type-queries-and-predicates`, `#istype`                                                                 | Each predicate's dump shape and its answer as a computed value.                                                                                               |
| `#size-alignment-count`                                                                                   | `sizeof` / `alignof` / `countof` opcodes and their folded values.                                                                                             |
| `#storage-type-legalization-casts`, `#storage--logical-casts`                                             | The cast opcodes legalization introduces, observed in the dump.                                                                                               |
| `#variable-struct-wrapping-legalization`                                                                  | The wrapper struct legalization introduces.                                                                                                                   |
| `#annotations`, `#annotation`                                                                             | The `Annotation` opcode and its operands.                                                                                                                     |
| `#liveness-markers`                                                                                       | Liveness range markers where a portable source shape produces them.                                                                                           |
| `#string-hashing`, `#getstringhash`                                                                       | The opcode in the dump **and** the folded hash value; two equal strings hash equal.                                                                           |
| `#kernel-launch`, `#cudakernellaunch`                                                                     | Only what is reachable without the CUDA host path.                                                                                                            |
| `#work-graph-records-and-barrier-flags`, `#getenumbarriermemorytypeflags-and-getenumbarriersemanticflags` | The **named constants** in emitted HLSL — never a hard-coded integer (see the HLSL named-constant rule in `CLAUDE.md`). Needs `-stage node -profile lib_6_8`. |
| `#untyped-descriptor-heap-handle-casts`                                                                   | That the cast does not survive into target code.                                                                                                              |
| `#compiler-dictionary-and-late-capability-requirements`, `#compilerdictionaryentry`                       | The dictionary entry shape where reachable.                                                                                                                   |
| `#coverage-gaps-against-sibling-pages`                                                                    | Nothing — this is a process section; record it as untested.                                                                                                   |

### Not testable here (record under `## Untested claims`)

- Rows whose only producer is an **IR pass** that runs after the last
  dump-observable point.
- Rows produced only by the **host / Torch binding path** (`removeTorchKernels`
  territory) — they need a Python/Torch build, not a `//TEST` directive.
- Rows with **no producer at HEAD** — state that explicitly; a negative test
  ("this opcode appears in no dump") is worth writing where the doc claims
  dormancy, but the claim that it _would_ mean something is not testable.
- `#family-hierarchy` claims about abstract grouping entries — the consequence is
  the shape of a C++ range check.
- `#coverage-gaps-against-sibling-pages` — a statement about this documentation
  set, not about compiler behavior.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. 25 to 40 `.slang` files. The bundle sits at its manifest `size_cap_files` of
   30; raise the cap in `_meta/manifest.yaml` in the same change if new claims
   push past it.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/ir-reference/misc.md`

Secondary (allowed citations; only where the primary doc hands off):

- `docs/generated/design/cross-cutting/ir-instructions.md`
- `docs/generated/design/pipeline/04-ast-to-ir.md`
- `docs/generated/design/ir-reference/types.md`
- `docs/generated/design/ir-reference/values.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md` instead.

## Source files you may consult for _verification only_

- `source/slang/slang-ir-insts.lua`
- `source/slang/slang-ir-insts.h`
- `source/slang/slang-ir.h`, `source/slang/slang-ir.cpp`
- `source/slang/slang-lower-to-ir.cpp`

## Sibling boundaries

`cross-cutting/ir-instructions` owns the schema, op-flag and hoistable/parent
conventions; `ir-reference/types` and `ir-reference/values` own the type and
value opcodes; `ir-reference/metadata` owns layout/attr/debug/asm records. A
claim that belongs to a sibling page is that page's, even when a `misc` opcode
appears in the same dump.

## Lessons captured for this bundle

- **Keep operands symbolic.** A constant operand is folded before the dump; read
  the operand from a thread id or hide it behind a generic parameter.
- **Anchor at a user-named function.** The dump contains the whole linked module;
  an unanchored pattern can match a core-module instruction instead.
- **Named constants, never integers.** Barrier flags and other target enums must
  be checked as their emitted _names_; asserting the integer bakes in a mapping
  the downstream compiler owns.
- **Value questions need value tests.** "Which arm runs" and "do these hash
  equal" cannot be answered from a spelling — use `COMPARE_COMPUTE -cpu` or
  `INTERPRET`.
- **Some opcodes are legitimately absent.** When the doc says a row has no
  producer, the honest test is that it appears in no dump.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every `doc_ref` resolves to an anchor in `ir-reference/misc.md` (or a
      listed secondary doc), and every `doc_section_digest` is current.
- [ ] Every catalogue row is either covered by a test or listed under
      `## Untested claims` with a reason — the page is a catalogue, so coverage
      is measured row by row.
- [ ] Emit-level claims are fanned out across the targets that can express them.
- [ ] Value-level claims use `COMPARE_COMPUTE -cpu` or `INTERPRET`, not a text
      match.
- [ ] No test depends on a GPU, a CUDA toolchain, or a Torch build.
- [ ] `## Doc gaps observed` records rows whose producer the doc names but which
      no portable source shape can reach.
