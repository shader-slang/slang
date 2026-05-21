# Prompt: tests-agentic/ir-reference/values/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/ir-reference/values/`,
anchored to
[`docs/llm-generated/ir-reference/values.md`](../../../docs/llm-generated/ir-reference/values.md).

Audience: nightly CI. The bundle exercises the **per-opcode catalog**
of the IR value-producing family — literals (`integer_constant`,
`float_constant`, `boolConst`, `void_constant`, `string_constant`,
the inline-payload encoding), arithmetic and bitwise
(`add`/`sub`/`mul`/`div`/`irem`/`frem`/`neg`/`shl`/`shr`/`and`/`or`/`xor`/`bitnot`/`not`/`bitfieldExtract`/`bitfieldInsert`),
logical short-circuit (`logicalAnd`/`logicalOr`) and branch-free `select`,
comparison (`cmpEQ`/`cmpNE`/`cmpLT`/`cmpLE`/`cmpGT`/`cmpGE`),
conversions (`intCast`/`floatCast`/`castIntToFloat`/`castFloatToInt`/`bitCast`/`castToVoid`),
memory (`var`/`globalConstant`/`load`/`store`/`get_field`/`get_field_addr`/`getElement`/`getElementPtr`/`swizzle`/`swizzledStore`),
aggregate constructors (`makeVector`/`makeMatrix`/`makeArray`/`makeStruct`/`makeTuple`),
and the optional helpers (`makeOptionalValue`/`makeOptionalNone`/`optionalHasValue`/`getOptionalValue`).

This bundle is the **per-opcode reference** for value opcodes. It is
adjacent to:

- `cross-cutting/ir-instructions` — the category-level view (one
  test per family). That bundle already covers `add`, `sub`, `mul`,
  `div`, `cmpGT`, `intCast`, `var`+`load`+`store`+`get_field_addr`,
  `swizzle`, and `rwstructuredBufferGetElementPtr` at a sample
  level; this bundle drills into the rest of the catalog without
  duplicating those.
- `ir-reference/types` — every value has a result type. This bundle
  tests the **value-producing opcodes**; it does not assert anything
  about the type-opcode catalog beyond what naturally appears in
  result-type columns of the value lines.
- `pipeline/04-ast-to-ir` — the AST-side mapping ("which AST node
  lowers to which IR opcode"). This bundle is anchored at the
  IR-value side, not the AST-node side.

Anchor each test at the IR-value opcode that the doc names; do not
write tests that observe IR-pass-introduced opcodes
(`constexpr*`, `defaultConstruct` produced by IR passes,
`makeUInt64` produced by literal lowering) unless the doc explicitly
describes a surface that triggers them on the LOWER-TO-IR stage.

## The translation rule: claims to observations

`values.md` lists every value opcode in tables grouped by family,
each row giving an `Opcode`, `C++ wrapper`, `Operands`, `Flags`,
`AST origin`, and `Summary`. The testable consequences are:

- **"AST construct X produces IR opcode Y"** — compile to a text
  target with `-dump-ir` and FileCheck for the opcode name (and its
  operand shape) in the IR dump. This is the primary mode.
- **"Opcode Y has a stable text-emit on target T"** — for value
  opcodes that lower predictably to operator syntax (vector swizzle
  to `.xyz`, arithmetic to `+`/`-`/`*`/`/`), use a target-specific
  emit test. Most value opcodes are target-independent at the IR
  level (lowering happens later); use `-dump-ir` as the primary
  mode and add multi-target emit tests only where the doc claims a
  per-target consequence.

### Observable claims (write tests for these)

The catalog row → claim mapping:

- **Integer literals** are emitted inline on every use as
  `<value> : <Type>` (the doc's "inline payload" claim). A literal
  `42` used in `add(%a, 42 : Int)` is the observation.
- **Float / Bool / Void literals** appear similarly: `1.5 : Float`,
  `true`/`false`, and `void_constant` as the operand of
  `return_val` for a `void` function.
- **`add`/`sub`/`mul`/`div`** (cross-cutting bundle already covers
  the `int` form; this bundle drills into `irem`, `frem`, `neg`,
  `shl`, `shr`, `and`, `or`, `xor`, `bitnot`, `not` which the
  catalog lists but the cross-cutting bundle does not).
- **`bitfieldExtract`/`bitfieldInsert`** — surface-level
  `bitfieldExtract(v, offset, count)` and `bitfieldInsert(v, ins,
  offset, count)` builtins.
- **`select`** — the branch-free conditional. Use the
  `select(c, x, y)` builtin (the ternary `?:` lowers to short-
  circuit `ifElse`, **not** `select`).
- **Comparisons** `cmpEQ`/`cmpNE`/`cmpLT`/`cmpLE`/`cmpGE` (the
  cross-cutting bundle covers `cmpGT`). One test per opcode in a
  group is too noisy; group three or more comparisons together.
- **Conversions** `intCast` (cross-cutting covers it; this bundle
  drills into the **boundary** cases: signed→unsigned at the same
  width, narrowing), `floatCast`, `castIntToFloat`,
  `castFloatToInt`, `bitCast`, `castToVoid`.
- **Memory** `var` + `load` + `store` + `get_field_addr` (cross-
  cutting covers the local-struct form). This bundle adds
  `get_field` (rvalue field read from a value, not a pointer),
  `getElement` (rvalue indexed read from an aggregate value), and
  `getElementPtr` (lvalue indexed read from an aggregate pointer).
- **`swizzle`** (cross-cutting covers single-component); this
  bundle adds multi-component `swizzle(%v, 0 : Int, 1 : Int, 2 : Int)`
  and `swizzledStore` for vector-lvalue partial writes.
- **`globalConstant`** — module-scope `static const T K = ...;`
  surfaces as `let %K : <T> = globalConstant(<value>)`. The doc
  cross-references `structure.md` for the container side, but the
  opcode itself is listed in `values.md`'s Memory table.
- **Aggregate constructors** — `makeVector(%a, %b, ...)` from a
  `floatN(...)` expression; `makeMatrix(...)` from a
  `floatRxC(...)` constructor. `makeArray` and `makeStruct` are
  **not** reliably observable from natural surface code on the
  LOWER-TO-IR stage: array initializers stream straight into element
  stores, and struct initializers lower to a synthesized `$init`
  function call. Record this as a doc gap (the doc lists `makeArray`
  and `makeStruct` with surface origins that don't actually produce
  them in the first IR dump stage).
- **`makeTuple`** — `makeTuple(a, b)` surface lowers to
  `makeTuple(...)` of `tuple_type(...)`. Tuple member access
  (`t._0`) lowers to `swizzle(%t, 0 : Int)`, *not* the
  `getTupleElement` opcode the doc lists in the Aggregate
  Constructors table — record as a doc gap.
- **`makeOptionalValue`** / **`makeOptionalNone`** /
  **`optionalHasValue`** / **`getOptionalValue`** — the Result /
  Optional / Conditional row family is observable via `Optional<T>`
  with `return x;` / `return none;` and `o.hasValue` / `o.value`
  accessors.
- **`OutParam`** — a function `void helper(out int o)` has a
  parameter of IR type `OutParam(Int)`; the caller passes a
  `Ptr(Int)` from a local `var`. The doc lists `outImplicitCast`
  as the conversion opcode applied to `out` arguments, but the
  natural lowering does **not** emit `outImplicitCast` for a
  same-type passthrough — the cast appears only when the caller's
  argument type differs from the parameter's declared type. Record
  the "implicit cast when types differ" case as a doc gap if you
  cannot construct a portable surface for it.

### Out of scope (record under the bundle's out-of-scope heading)

- **`constexpr*` opcodes** (`constexprAdd`/`constexprMul`/etc.) —
  produced by the IR's compile-time integer evaluator when an
  `IntVal` (such as a `PolynomialIntVal` from an array-size
  expression) needs to be lowered. The doc lists them but does not
  name a surface that reliably surfaces them at LOWER-TO-IR; they
  are typically interspersed with non-`constexpr` arithmetic.
- **`Poison`** and **`LoadFromUninitializedMemory`** — the doc
  describes their semantics, but the dump shows `Poison` only as a
  preamble symbol declaration (`Poison` at the top of the IR), not
  as an active operand. A reading-an-uninitialized-local diagnostic
  is a frontend check (a negative test), not a positive observation
  of the opcode.
- **`alloca`** — the doc lists it for dynamic-size stack allocation
  but does not name a portable Slang surface that triggers it; the
  natural runtime-array surface lowers via other paths on most
  targets.
- **`copyLogical`, `assumeAddress`, `getAddr`, `getOffsetPtr`,
  `getManagedPtrWriteRef`, `ManagedPtrAttach`, `ManagedPtrDetach`,
  `getNativePtr`, `getNativeStr`, `makeString`, `allocObj`,
  `CUDA_LDG`** — all marked "(synthesized)" or host-side in the
  doc. The doc does not name a portable shader-language surface
  that reliably produces them on the LOWER-TO-IR stage.
- **`matrixReshape`/`vectorReshape`/`makeArrayFromElement`/
  `MakeVectorFromScalar`/`makeMatrixFromScalar`/`makeCoopVector`/
  `makeCoopVectorFromValuePack`/`makeCoopMatrixFromScalar`/
  `makeTargetTuple`/`SumVectorElements`/`SumMatrixElements`/
  `updateElement`** — all marked "(synthesized)" or "implicit
  lowering" in the doc; the natural shader-language surface that
  triggers each is unclear from the doc alone.
- **`makeResultValue`/`makeResultError`/`isResultError`/
  `getResultValue`/`getResultError`** — `Result<T,E>` is in the
  core module but the doc does not state a portable, terse surface
  for constructing both arms in one shader.
- **`extractTaggedUnionTag`/`extractTaggedUnionPayload`** —
  produced by the existential-elimination pass; the doc itself
  cross-references `generics-and-existentials.md`.
- **`ReinterpretOptional`/`unmodified`/`reinterpret`/`PtrCast`/
  `CastPtrToBool`/`CastPtrToInt`/`CastIntToPtr`/`CastEnumToInt`/
  `CastIntToEnum`/`EnumCast`/`CastUInt2ToDescriptorHandle`/
  `CastDescriptorHandleToUInt2`/`BuiltinCast`** — most are
  "(synthesized)" or host-side. Enum cast is doable via an
  `enum`; pointer-int casts and reinterpret-optional are not
  portably observable.
- **`makeConditionalValue`/`getConditionalValue`** — `Conditional`
  is "(synthesized)" per the doc.
- **`makeUInt64`** — produced by literal lowering on targets that
  don't have a `uint64` literal; the doc lists it with both AST
  origins as `MakeUInt64Expr` and "synthesized". Natural surface
  `uint64_t x = 0xDEAD...uL;` does not emit `makeUInt64` in the
  observed LOWER-TO-IR dump.

If you find yourself thinking "this would verify the
deduplication-by-hash for `IntLit 42`", stop — that's an internal
probe.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Out of scope (no-GPU runner)` as the home for the
   unobservable-via-slangc items listed above (the heading is a
   convention shared with `cross-cutting/ir-instructions`,
   `ir-reference/types`, and `pipeline/04-ast-to-ir`).
2. 18 to 35 `.slang` test files. Aim for one observable opcode (or
   one observable per-target lowering) per test. Group three or
   more closely-related opcodes (e.g. the four comparison opcodes
   not already covered by `cross-cutting/ir-instructions`) into one
   file rather than emitting four single-opcode files.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/ir-reference/values.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off or the test specifically observes a
cross-bundle boundary):

- `docs/llm-generated/cross-cutting/ir-instructions.md`
- `docs/llm-generated/pipeline/04-ast-to-ir.md`
- `docs/llm-generated/ast-reference/expressions.md`
- `docs/llm-generated/ir-reference/types.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for verification only

You may look at these to confirm an opcode spelling (the exact name
as it appears in `-dump-ir` output). You may **not** mine them for
behavioral claims the doc does not make.

- `source/slang/slang-ir-insts.lua` — canonical opcode list.
- `source/slang/slang-ir-insts.h` — C++ wrapper structs.
- `source/slang/slang-ir.h`, `slang-ir.cpp` — `IRInst` + builder.
- `source/slang/slang-lower-to-ir.cpp` — AST-to-IR lowering.

The opcode spelling in the IR dump is the **lowercase Lua entry
key** for most value/control-flow opcodes: `add`, `sub`, `mul`,
`div`, `irem`, `frem`, `neg`, `shl`, `shr`, `and`, `or`, `xor`,
`bitnot`, `not`, `cmpEQ`, `cmpNE`, `cmpLT`, `cmpLE`, `cmpGT`,
`cmpGE`, `intCast`, `floatCast`, `castIntToFloat`,
`castFloatToInt`, `bitCast`, `castToVoid`, `var`, `globalConstant`,
`load`, `store`, `get_field`, `get_field_addr`, `getElement`,
`getElementPtr`, `swizzle`, `swizzledStore`, `select`, `makeVector`,
`makeMatrix`, `makeTuple`, `makeOptionalValue`, `makeOptionalNone`,
`optionalHasValue`, `getOptionalValue`, `bitfieldExtract`,
`bitfieldInsert`, `return_val`. Literals appear **inline** on each
use as `<value> : <Type>` (`42 : Int`, `1.5 : Float`, `true`,
`false`, `void_constant`), never as a separately-named
`let %N : <T> = integer_constant(...)` line.

## Test directives

Value-opcode claims are best observed through one of two mechanisms.

1. **`-dump-ir` against the IR dump** (per-opcode appearance). The
   standard form used here is:

   ```
   //TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
   ```

   Per the universal `_common.md` rule: combine `-dump-ir` with
   **`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
   goes to stdout uncontaminated by target text.

   Anchor patterns at `func %main` (or a user-named helper) to cut
   through the large IR-dump preamble. Use the `IR:` pattern prefix
   so the same file can also carry text-emit directives later.

2. **Multi-target emit** for value-opcode lowerings that are
   target-divergent. Vector swizzle, vector arithmetic, and
   constant-emission produce different code on each text-emit
   target; use one `//TEST:SIMPLE(filecheck=CHECK<target>):-target
   <T> ...` directive per feasible target, with a distinct CHECK
   prefix and per-target pattern. The bundle's source doc does not
   claim a uniform text-emit rendering across all targets; the test
   asserts the per-target text individually.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ir-reference/values.md` (or one of the listed secondary
      docs).
- [ ] Every `-dump-ir` test uses `-target <text-target> -dump-ir
      -o /dev/null -stage compute -entry main` per CLAUDE.md.
- [ ] Outputs escape DCE: write to an `RWStructuredBuffer<T>` so
      the value observation survives to the dump's first stage.
- [ ] Operands are non-constant (`uniform` globals or values
      derived from `SV_DispatchThreadID`) so constant folding does
      not collapse the operator.
- [ ] CHECK patterns anchored at `func %main` or a user-named
      function — the IR-dump preamble must not match.
- [ ] No test depends on a GPU.
- [ ] No test asserts a C++ wrapper-struct identity (`IRAdd`,
      `BoolLit`, `IRSwizzleSet`, etc.) — the dump shows the
      lowercase Lua name.
- [ ] No test asserts on `constexpr*` opcodes — those are
      IR-pass-introduced and not part of the LOWER-TO-IR catalog.
- [ ] No test was written by inspecting an uncovered source line.
      If you find yourself thinking "this would cover the branch
      at `slang-lower-to-ir.cpp:NNNN`", stop and re-read the doc.
- [ ] README.md `## Doc gaps observed` is honest. If you wanted a
      test you could not anchor, write down which claim the doc
      would need to add.

## Lessons captured (apply to this bundle as well)

These are restated from `_common.md` because they bite hard in
value-opcode observation tests:

- `-dump-ir` requires `-target <X>` (else compile stops early) and
  `-o /dev/null` (else target text mixes with IR on stdout).
- Constant folding collapses arithmetic on literals. To observe
  `add(%a, %b)` or any binary opcode, both operands must be non-
  constant: read them from `uniform` globals or compute them from
  `SV_DispatchThreadID`.
- The IR dump prefixes user IR with a substantial preamble
  (`Poison` declaration, core-module imports, capability sets,
  differentiation witness tables, autodiff glue). Anchor
  patterns at `func %main` or a user-named callee.
- `&&` / `||` (short-circuit) **do not** lower to `logicalAnd` /
  `logicalOr`; they lower to a pair of `ifElse` blocks joining at a
  `param`. Use the `select(c, x, y)` builtin to observe `select`;
  no portable surface in this bundle reliably produces
  `logicalAnd` / `logicalOr`.
- The `?:` ternary lowers to a structured `ifElse(...)` plus
  block-parameter, **not** to the `select` opcode. The doc says
  `select` "lowers from `SelectExpr` and ternary `?:`" but the
  observed lowering of `?:` uses `ifElse`; record as a doc gap.
- CUDA factors `__ldg(&uniform)` reads into temporaries, splitting
  compound expressions on uniform operands. To observe a binary
  expression on CUDA, derive operands from `SV_DispatchThreadID`
  rather than from `uniform` globals.
- Trivial scalar locals are eliminated during lowering. The `var`
  opcode survives in IR when the local is a struct accessed by
  field address (or an array indexed elementwise).
- Literals appear **inline** on every use (`42 : Int`,
  `3.14 : Float`, `true`, `false`, `void_constant`); there is no
  separately-named `let %N : Int = integer_constant(...)` line.
  Anchor the literal observation by reading the operand of the
  surrounding opcode, e.g. `add(%a, 42 : Int)` or
  `return_val(void_constant)`.
