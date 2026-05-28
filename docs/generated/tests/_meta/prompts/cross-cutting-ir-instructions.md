# Prompt: docs/generated/tests/cross-cutting/ir-instructions/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/cross-cutting/ir-instructions/`,
anchored to
[`docs/generated/design/cross-cutting/ir-instructions.md`](../../../docs/generated/design/cross-cutting/ir-instructions.md).

Audience: nightly CI. The bundle exercises the **observable shape of the
Slang IR** at the `LOWER-TO-IR` stage — that the documented per-family
opcodes (arithmetic, memory, control-flow, structure, specialization,
decorations, resource/IO) appear in the IR dump for the obvious source
construct that ought to produce them, and that those same constructs
lower to predictable code on text-emit targets. It does **not** test the
behaviour of individual IR passes (those belong to
`pipeline/05-ir-passes`), nor the per-family opcode catalog in detail
(those belong to the `ir-reference/*` bundles).

## The translation rule: claims to observations

`ir-instructions.md` describes **categories** of IR instructions —
types, values (arithmetic/conversion), memory (`var`/`load`/`store`),
control flow (`block`/`param`/`ifElse`/`loop`/`return_val`), function
and module structure (`func`/`generic`/`global_param`/`witness_table`),
specialization (`specialize`/`lookupWitness`/`makeExistential`),
decorations (`NameHintDecoration`/`EntryPointDecoration`/...), and
resource opcodes (`structuredBufferLoad` etc.). It uses **representative,
not exhaustive** tables. The testable consequences are:

- **"X produces opcode Y"** — compile to a text target with `-dump-ir`
  and FileCheck for the opcode name in the IR dump (typically the
  `LOWER-TO-IR` stage which is the first dumped stage). This is the
  primary mode.
- **"X lowers to predictable code on target T"** — compile to target
  `T` and FileCheck the emitted code for an unambiguous substring. Use
  this when the IR-level instruction has a stable rendering on multiple
  text-emit targets (e.g. `add` → `+` on every C-like target; an
  `entryPoint` decoration → a target-specific entry-point keyword).

Concretely:

### Observable claims (write tests for these)

- **Arithmetic opcodes** (`add`/`sub`/`mul`/`div`) appear in the IR
  dump when two non-constant operands are combined. Use `uniform`
  globals to defeat constant folding, then dump IR with
  `-target spirv-asm -dump-ir -o /dev/null` and FileCheck for
  `sub(%`/`div(%`/etc. on a line.
- **Comparison opcodes** (`cmpGT`, `cmpLT`, ...) appear in the IR for
  the obvious source comparison.
- **Conversion opcodes** (`intCast`, `floatCast`, `bitCast`) appear
  for an explicit constructor-style cast.
- **Memory opcodes** (`var`, `load`, `store`, `get_field`,
  `get_field_addr`, `rwstructuredBufferGetElementPtr`) appear in IR
  for the obvious source constructs (local variable; assignment to
  struct field; index into a buffer).
- **Control-flow opcodes** (`block`, `param`, `ifElse`,
  `unconditionalBranch`, `loop`, `return_val`) appear in IR for an
  `if/else`, a `for`, and a `return`.
- **Function/module structure** (`func`, `generic`, `global_param`,
  `struct`, `interface`, `witness_table`, `witness_table_entry`)
  appear for the obvious source declarations.
- **Specialization** (`specialize(%generic, %T)`) appears in IR when
  a generic function is called with a concrete type argument.
- **Decorations**: `[entryPoint(...)]` decorates the entry-point
  function. `[nameHint("...")]` decorates user-named declarations.
- **Cross-target emit** for IR claims that lower predictably:
  - `add` on `int` lowers to `+` on HLSL / GLSL / Metal / WGSL / CUDA
    / CPP. Use the multi-backend pattern.
  - `entryPoint` decoration lowers to a target-specific entry-point
    keyword (`[shader("compute")]`-equivalent decoration / SPIR-V
    `OpEntryPoint`). Pin to one or two targets per test.

### Not testable through slangc (record under `## Untested claims`)

- IR-internal **flag-bit layout** (`kIROpFlag_Hoistable`,
  `kIROpFlag_Global`, `kIROpFlag_Parent`, `kIROpFlag_UseOther`) —
  internal to `IRInst`'s op encoding.
- Whether a specific opcode **is hoisted** by the deduplication
  machinery (the IR dump shows the post-hoist result, not the
  hoisting decision).
- The contiguous **opcode-range allocation** that lets
  `as<IRBasicType>()` be a single integer compare — entirely
  internal to FIDDLE / `slang-ir-insts-enum.h.fiddle`.
- **Module-version bumps** required when inserting a new opcode in
  the middle of an existing range — a build-system / serialization
  invariant (`cross-cutting/serialization` bundle covers user-facing
  module deserialization claims).
- The **`UseOther` opcode-bit-packing** (`kIROpMeta_OtherShift = 10`).
- The **`IRBuilder` deduplication** path for hoistable instructions —
  observable only by comparing two uses of the same `vector(Int, 3)`
  type, which is not surface-visible.
- The "**add a new opcode**" workflow steps — a developer guide, not
  a user-observable behavior.

If you find yourself thinking "this would verify that the IR builder
deduplicates the second `vector(Int, 3)`", stop — that is an internal
implementation probe.

## Required structure

1. `README.md` with the structure named in `_common.md`. The
   `## Untested claims` section here doubles as the
   place to list claims that aren't observable via slangc at all (IR
   flag bits, hoisting decisions, opcode-range layout, build-time
   FIDDLE wiring, module-versioning bumps).
2. 10 to 25 `.slang` test files. Aim for one observable opcode per
   test (or one observable cross-target lowering per test). Quality
   over quantity — internal IR claims should be skipped, not paraphrased.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/cross-cutting/ir-instructions.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off):

- `docs/generated/design/ir-reference/index.md`
- `docs/generated/design/ir-reference/structure.md`
- `docs/generated/design/ir-reference/types.md`
- `docs/generated/design/ir-reference/values.md`
- `docs/generated/design/ir-reference/control-flow.md`
- `docs/generated/design/pipeline/04-ast-to-ir.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm an opcode spelling (the exact
name as it appears in `-dump-ir` output), but you may **not** mine them
for behavioral claims that the doc does not make. The opcode-name
spelling in IR dumps generally matches the Lua entry key (lowercase
`add`/`sub`/`mul`/`div`/`store`/`load`/`var`/`block`/`param`/`loop`,
camelCase or special-case for some such as `rwstructuredBufferGetElementPtr`,
`ifElse`, `unconditionalBranch`).

- `source/slang/slang-ir-insts.lua` — canonical opcode list.
- `source/slang/slang-ir-insts.h` — C++ wrapper structs.
- `source/slang/slang-ir.h`, `.cpp` — `IRInst` + builder.

## Test directives

IR-instruction claims are best observed through one of two mechanisms.

1. **`-dump-ir` against the IR dump**. The CLAUDE.md rule:

   > Always combine `-dump-ir` with `-target` (otherwise compilation
   > stops early) and `-o <file>` (otherwise target code mixes with IR
   > on stdout).

   The standard form used here is:

   ```
   //TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
   ```

   `-o /dev/null` ensures the target text is discarded and only the IR
   dump (which goes to stdout) is FileChecked. Pattern lines should
   use the `IR:` prefix (or any unique tag) so the same file can also
   carry a non-dump-ir target test. Anchor FileCheck patterns to
   `### LOWER-TO-IR:` first, then match opcodes that follow.

   Useful conventions when writing CHECK patterns against IR dumps:

   - Use `{{.*}}` for variable IDs (`%3`, `%tmp_42`) and address-like
     SSA suffixes.
   - Match the opcode name plus an opening paren or whitespace
     (`add(`, `store(`, `loop(`, `ifElse(`) so the pattern is
     unambiguous; bare `add` would also match `Pad`, `cmpAdd`, etc.
   - Anchor structural patterns (`block %{{.*}}`, `func %main`) when
     verifying control-flow or function structure.

2. **Cross-target emit**. For an IR instruction that lowers to
   predictable code on a specific target, use a regular text-target
   compile + FileCheck.

   ```
   //TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
   //TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
   //TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
   ```

   The multi-backend rule from `_common.md` applies STRONGLY here. If
   the doc claims an IR opcode has stable text emit on multiple
   targets, USE every feasible text-emit target.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `cross-cutting/ir-instructions.md` (or one of the listed
      secondary docs).
- [ ] Every `-dump-ir` test uses `-target <text-target> -dump-ir -o
      /dev/null` per CLAUDE.md (target needed so compilation does not
      stop early, `-o /dev/null` so target text does not mix with the
      IR dump on stdout).
- [ ] Constructor-style casts (`int(x)`, `float(y)`) — not C-style
      `(int)x`.
- [ ] Where the doc names a target-independent IR instruction (e.g.
      `add`/`sub`/`mul`/`div`/`block`/`param`/`return_val`/`store`),
      the test uses the `-dump-ir` mode rather than a single-target
      emit, because the IR is the same regardless of target.
- [ ] Where the doc names a per-target emit consequence, use a
      multi-backend test (one CHECK block per target) — not a single
      target.
- [ ] No test asserts that an opcode has a specific FIDDLE-generated
      enum value or that it falls in a specific contiguous range.
- [ ] No test depends on a GPU.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-ir.cpp:NNNN`", stop and re-read the doc.

## Lessons captured from the pilot bundles

These apply here too:

- Constant folding aggressively collapses arithmetic on literals
  (`add(1 : Float, 2 : Float)` shows as a pre-folded value). To
  reliably observe `add(%a, %b)` in the IR dump, feed the arithmetic
  **non-constant** operands — `uniform` globals or values derived
  from `SV_DispatchThreadID`.
- The IR dump includes a large preamble of `Annotation(...)`, core-
  module imports, capability sets, and differentiation glue. Anchor
  your FileCheck patterns to the user-named function or `nameHint`
  decoration to avoid false positives. `func %main` or
  `func %userFunc` is a reliable cursor.
- `-dump-ir` writes the IR to stdout while target output normally
  also goes to stdout. Always include `-o /dev/null` so the IR is the
  only thing on stdout that FileCheck sees.
- The opcode name in the IR dump generally matches the **lowercase
  Lua entry key**, not the C++ wrapper struct name. The doc cites
  both: `add` / `Add`, `var` / `IRVar`, `get_field` / `FieldExtract`.
  Match the lowercase spelling.
