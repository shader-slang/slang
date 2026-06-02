# Prompt: docs/generated/tests/ir-reference/control-flow/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/ir-reference/control-flow/`,
anchored to
[`docs/generated/design/ir-reference/control-flow.md`](../../../design/ir-reference/control-flow.md).

Audience: nightly CI. The bundle exercises the **per-opcode catalog**
of the IR control-flow family — `block` (basic block parent), `param`
(block parameters used as Slang IR's `phi` replacement), the
`TerminatorInst` family (`return_val`, `unconditionalBranch`, `loop`,
`conditionalBranch`, `ifElse`, `switch`, `missingReturn`,
`unreachable`), and related opcodes (`discard`).

This bundle is the **per-opcode reference** for control-flow opcodes.
It is adjacent to:

- `cross-cutting/ir-instructions` — the category-level view (one
  test per family). Anything the category bundle exercises at a
  sample level is not duplicated here; this bundle drills into the
  catalog drillable from natural Slang surface code.
- `ir-reference/values` — the value-producing opcode catalog. Some
  tests in this bundle incidentally observe value opcodes (`add`,
  `cmpLT`, `cmpGT`, `cmpEQ`) as IR operands of the control-flow
  observation, but the anchor is the control-flow opcode.
- `pipeline/04-ast-to-ir` — the AST-side mapping ("which statement
  lowers to which terminator"). This bundle is anchored at the
  IR-terminator side, not the AST-statement side.

Anchor each test at the IR control-flow opcode that the doc names;
do not write tests that observe IR-pass-introduced opcodes
(`conditionalBranch` rarely surfaces from the front-end — record as
a doc gap rather than fabricating a synthetic surface).

## The translation rule: claims to observations

`control-flow.md` lists every control-flow opcode in tables grouped
by family, each row giving an `Opcode`, `C++ wrapper`, `Operands`,
`Flags`, `AST origin`, and `Summary`. The testable consequences are:

- **"AST statement X produces IR terminator Y with operand shape Z"** —
  compile to a text target with `-dump-ir` and FileCheck for the
  opcode name and its operand shape in the IR dump. This is the
  primary mode.
- **"Block parameters encode SSA values flowing between blocks"** —
  observable from a `for`/`while`/`do`/`switch` body whose induction
  variable or per-arm result becomes a `param %X : <T>` line on the
  successor block, with the predecessor's `unconditionalBranch` or
  `loop` carrying the matching argument operand. The "Slang IR
  replaces `phi` with `param`" claim is the centerpiece of the
  doc's `## block-and-param` note.

### Observable claims (write tests for these)

The catalog row → claim mapping that is reliably observable from
natural surface code via `-dump-ir`:

- **`block` parent** — observable implicitly in every IR dump as
  the `block %N:` line that introduces each basic block; tested
  whenever a multi-block construct (if/loop/switch) is dumped.
- **`param` (block parameter)** — for-loop and while-loop induction
  variables surface as `param %s : Int` and `param %i : Int` on the
  loop header block. Switch fall-through results that merge in the
  break block surface as `param %r : Int` on the break block.
- **`return_val(%v)`** — `return expr;` in a value-returning
  function terminates the block with `return_val(%v)` where `%v` is
  the returned value.
- **`return_val(void_constant)`** — a falling-off-the-end (or
  explicit `return;`) of a `void` function terminates with
  `return_val(void_constant)` (the unique `VoidLit`).
- **`unconditionalBranch(%target)`** — a `break;` jumps to the
  loop's break label; an unguarded `continue;` jumps to the
  continue label; the end of an `if` arm jumps to the merge block.
- **`unconditionalBranch(%target, %arg, ...)`** — the predecessor
  side of an SSA block-parameter handoff. `switch` arms that
  produce a value supply it as the operand of the branch into the
  break block; loop bodies supply updated induction values as
  branch operands.
- **`loop(%body, %break, %continue, ...)`** — `for` / `while` /
  `do`-`while` all lower to a `loop` terminator with the three
  structured-join operands. Initial values of loop-induction
  variables appear as trailing operands.
- **`ifElse(%cond, %t, %f, %merge)`** — `if (...) ... else ...`
  lowers to `ifElse(...)` with the explicit `mergeBlock` as the
  fourth operand, even when one arm is empty.
- **`switch(%val, %break, %default, val1, %case1, ...)`** —
  `switch` statement lowers to `switch(...)` with the break label
  second, default label third, and `(caseValue, caseBlock)` pairs
  for each case.
- **`unreachable`** — a `switch` whose every arm `return`s leaves
  the post-switch break block unreachable; emitted as the
  terminator of the synthesized merge block.
- **`discard`** — fragment-shader `discard` lowers to a `discard`
  instruction that sits **inside** a block (it is not the
  terminator); the block's real terminator (an
  `unconditionalBranch` to the post-`if` merge) follows.

### Untested claims (record under the bundle's out-of-scope heading)

- **`conditionalBranch`** — the doc lists this as `(synthesized)`
  with no AST origin; in practice the front-end emits structured
  `ifElse` for every `if`. Record as a doc gap rather than
  fabricating a synthetic surface.
- **`yield`** — terminates the body of a `generic` value;
  observable only inside generic-instantiation IR, which the
  surface-language test cannot trigger without writing a synthetic
  module that exposes the post-lowering IR. Out of scope.
- **`tryCall`** / **`throw`** — error-flow opcodes, lower from
  `TryExpr` and `ThrowStmt`. Slang's `throw` / `try` surface has
  evolved; even when the surface compiles, the dump-IR view may
  show ordinary-`call` lowering. Out of scope for this bundle;
  exercise via `cross-cutting/ir-instructions` if at all.
- **`defer`** — `DeferStmt` lowers to `defer`, but the natural
  surface form is non-trivial to write portably across stage /
  target requirements. Out of scope.
- **`targetSwitch`** — synthesized by the multi-target lowering;
  no natural Slang surface produces it on the LOWER-TO-IR stage.
- **`missingReturn`** — synthesized by lowering when a non-`void`
  function falls off the end without `return`; the natural surface
  is rejected as a diagnostic before IR is dumped, so this opcode
  is not portably observable.
- **`gpuForeach`** — emitted by host-shader lowering only; no
  natural shader-language surface produces it.
- **`GenericAsm`** / **`RequirePrelude`** /
  **`RequireTargetExtension`** / **`RequireComputeDerivative`** /
  **`StaticAssert`** / **`Printf`** /
  **`RequireMaximallyReconverges`** / **`RequireQuadDerivatives`** —
  per-doc, these are either `(synthesized)` or backend hints;
  no portable surface anchors them at LOWER-TO-IR.

If you find yourself thinking "this would verify the
critical-edge-forbidden invariant on `conditionalBranch`", stop —
that's an internal probe and the opcode itself isn't surfaced.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for the
   unobservable-via-slangc items listed above.
2. 12 to 22 `.slang` test files. Aim for one observable opcode (or
   one observable per-AST-statement lowering) per test. Group two
   or three closely-related observations (e.g. `break` and
   `continue` together) into one file when the same surface
   construct gives the cleanest single-file observation.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/ir-reference/control-flow.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off or the test specifically observes a
cross-bundle boundary):

- `docs/generated/design/cross-cutting/ir-instructions.md`
- `docs/generated/design/ir-reference/values.md`
- `docs/generated/design/pipeline/04-ast-to-ir.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Test directives

Control-flow-opcode claims are best observed through `-dump-ir`
against the IR dump. The standard form used here is:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

Per the universal `_common.md` rule: combine `-dump-ir` with
**`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
goes to stdout uncontaminated by target text.

Anchor patterns at `func %main` (or a user-named helper such as
`func %helper`) to cut through the large IR-dump preamble. Use the
`CHECK:` pattern prefix.

Use `uniform` globals or `SV_DispatchThreadID` to defeat constant
folding on condition operands. Without this, `if (1 < 2)` collapses
to a single block and the `ifElse` opcode never appears.

Use `static const int K = ...;` only for switch case **labels**,
which must be compile-time constants — never as a switch
scrutinee, which would collapse to a single arm.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ir-reference/control-flow.md` (or one of the listed
      secondary docs).
- [ ] Every test uses `-target spirv-asm -dump-ir -o /dev/null
      -entry main -stage compute` (or `-stage fragment` for
      `discard`) per CLAUDE.md.
- [ ] Outputs escape DCE: write to an `RWStructuredBuffer<T>` so
      the value observation survives.
- [ ] Operands are non-constant (`uniform` globals or values
      derived from `SV_DispatchThreadID`) so constant folding does
      not collapse the conditional or loop.
- [ ] CHECK patterns anchored at `func %main` or a user-named
      function — the IR-dump preamble must not match.
- [ ] No test depends on a GPU.
- [ ] No test asserts a C++ wrapper-struct identity (`IRLoop`,
      `IRIfElse`, etc.) — the dump shows the lowercase opcode
      spelling.
- [ ] README.md `## Doc gaps observed` is honest.

## Lessons captured (apply to this bundle as well)

These bite hard in control-flow-opcode observation tests:

- `-dump-ir` requires `-target <X>` and `-o /dev/null`.
- Constant folding collapses conditions on literals. Loop
  conditions, `if` conditions, and `switch` scrutinees must be
  derived from `uniform` globals (or `SV_DispatchThreadID`) to keep
  the structured terminator alive.
- The IR dump prefixes user IR with a large preamble. Anchor
  patterns at `func %main` or a user-named callee.
- A `do`-`while` lowering inverts the test (the IR computes
  `not(%cond)` and branches to break on true), so a test for the
  do-while shape should not assert the predicate text without that
  inversion.
- A `switch` whose every arm returns leaves the synthesized
  post-switch break block with a trivial `unreachable` terminator.
- `discard` is **not** a terminator: it appears as an ordinary
  instruction line inside an `if`-arm block, immediately followed
  by an `unconditionalBranch` (to the post-`if` merge) that is the
  block's real terminator.
- `break;` and `continue;` lower to `unconditionalBranch`s to the
  loop's `breakLabel` and `continueLabel` respectively; the labels
  are the second and third operands of the loop's `loop(...)`
  terminator. Use the same label number in a CHECK pattern to
  cross-anchor "the break in this body goes to *this* loop's
  break".
