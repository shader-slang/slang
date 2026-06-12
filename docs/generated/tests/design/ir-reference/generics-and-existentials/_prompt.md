# Prompt: docs/generated/tests/design/ir-reference/generics-and-existentials/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at
`docs/generated/tests/design/ir-reference/generics-and-existentials/`, anchored to
[`docs/generated/design/ir-reference/generics-and-existentials.md`](../../../design/ir-reference/generics-and-existentials.md).

Audience: nightly CI. This bundle is the **per-opcode reference**
for the IR opcodes that bind generic parameters to concrete types,
look up interface requirements through witness tables, construct
and destructure existential (interface-typed) values, and carry
runtime type information:

- `specialize` (generic application — both function and type
  application);
- `lookupWitness` (the IR encoding of interface dispatch);
- `makeExistential` (the IR form of `CastToInterfaceExpr` — cast
  from concrete type to interface type);
- `extractExistentialType` / `extractExistentialValue` /
  `extractExistentialWitnessTable` (the three projections that
  reverse `makeExistential`);
- `witness_table` and `witness_table_entry` (the dispatch tables
  consumed by `lookupWitness`);
- `interface_req_entry` (the interface-side counterpart of
  `witness_table_entry`);
- `global_generic_param` (a module-scope `type_param T;` declares
  a generic parameter at module level).

This bundle is adjacent to:

- `cross-cutting/ir-instructions` — the category-level view that
  samples each IR family at one anchor. This bundle drills the
  **specialization / existentials** family in detail; it does not
  duplicate the category-level sampling.
- `ir-reference/structure` — the structural / hierarchy opcode
  catalog (`func`, `generic`, `struct`, `class`, `interface`,
  `witness_table` as a structural parent). This bundle anchors the
  **dispatch-side** semantics of `lookupWitness` (it is the
  consumer of a `witness_table` inside a generic body) and the
  **construction/destructuring** semantics of `makeExistential` /
  `extractExistential*`. The structural bundle anchors
  `witness_table`, `witness_table_entry`, `interface_req_entry`,
  and `lookupWitness` from the parent-child shape side; this
  bundle re-anchors them from the operand/result-shape side that
  the dispatch path consumes.
- `ir-reference/types` — the type opcodes that the existential /
  generic opcodes operate on (`BindExistentialsType`,
  `BoundInterface`, `AnyValueType`, `DynamicType`,
  `RTTIPointerType`, `InterfaceType`). This bundle does not test
  type opcodes themselves.

Anchor each test at the opcode that the doc names; do not write
tests that observe opcodes the doc lists as `(synthesized)` unless
you can find a natural Slang surface that produces them at the
LOWER-TO-IR stage. Record any non-natural ones under
`## Untested claims` in `README.md`.

## The translation rule: claims to observations

`generics-and-existentials.md` describes each opcode in tables
grouped by role (`Generic application`, `Witness lookup`,
`Existential construction`, `Existential destructuring`,
`Witness tables and witness facts`, `Runtime type information`,
`Type-flow specialization`, `AnyValue marshalling`). Each row
names an `Opcode`, `C++ wrapper`, `Operands`, `Flags`, `AST origin`,
and `Summary`. The testable consequences at LOWER-TO-IR are:

- **"AST surface X produces IR opcode Y with operand shape Z"** —
  compile to a text target with `-dump-ir` and FileCheck for the
  opcode name and operand pattern in the IR dump. This is the
  primary mode.
- **"Opcode Y's result type is T"** — observable as the
  surrounding `let %r : T = Y(...)` line in the dump.
- **"Opcode Y appears together with Opcode Z in the dispatch
  pattern"** — observable as a sequence of lines in the same
  basic block (e.g. `makeExistential` produces an existential,
  then `extractExistentialType` / `Value` / `WitnessTable`
  reverses it before `lookupWitness` runs).

### Observable claims at LOWER-TO-IR (write tests for these)

The catalog row → claim mapping that is reliably observable from
natural surface code via `-dump-ir` at the first LOWER-TO-IR block:

- **`specialize` on a generic function** — `pickFirst<int>(a, b)`
  lowers to `call specialize(%pickFirst, Int)(%a, %b)` in the
  caller body. The opcode name appears as `specialize` in the
  callee position of `call`.
- **`specialize` on a generic type** — a `Pair<int>` field type
  lowers to `specialize(%Pair, Int)` as the result type of the
  field-typed value.
- **`specialize` carries a witness operand** — `helper<A>(a, x)`
  where `helper<T : I>` is generic-with-constraint lowers to
  `call specialize(%helper, %A, %witnessA)(%a, %x)` — the
  witness flows in as an extra `specialize` argument after the
  type argument.
- **`specialize` is hoistable** — two textually-identical
  `specialize` calls deduplicate; observable indirectly by
  pinning the spelling of one site (the hoistable flag means the
  IR has at most one instance per (base, args) tuple).
- **`lookupWitness` operand shape** — a generic body calling
  `t.run(x)` where `t : T` and `T : I` produces
  `lookupWitness(%witnessParam, %Ix5Frun)` whose two operands
  are `(witnessTable, requirementKey)`.
- **`lookupWitness` result type** — the result is the
  requirement's function type, e.g. `Func(Int, %T, Int)` for an
  `int run(int x)` requirement.
- **`makeExistential` operand shape** — casting a concrete value
  to an interface variable (`IFoo i = a;` where `a : A` and
  `A : IFoo`) produces `makeExistential(%a, %witnessA_IFoo)` —
  the two operands are `(value, witness)`.
- **`makeExistential` result type** — the result is the target
  interface type, observable as `let %i : %IFoo = makeExistential(...)`.
- **`extractExistentialType`** — dispatching through an
  interface-typed value produces `extractExistentialType(%i)` to
  read the packed concrete type; result type is `Type`.
- **`extractExistentialValue`** — dispatching through an
  interface-typed value produces `extractExistentialValue(%i)`
  to read the packed concrete-typed value.
- **`extractExistentialWitnessTable`** — dispatching through an
  interface-typed value produces `extractExistentialWitnessTable(%i)`
  whose result type is `witness_table_t(%I)`.
- **dispatch sequence shape** — interface dispatch on an
  existential follows the pattern `extractExistentialType` →
  `extractExistentialValue` + `extractExistentialWitnessTable` →
  `lookupWitness` → `call`. All four opcodes appear in order
  within the same dispatch site.
- **`witness_table` type shape** — a `struct S : I` produces a
  `witness_table %N : witness_table_t(%I)(%S)` whose type's
  parenthesised pair is `(interface, implementing-type)`.
- **`witness_table_entry` pairs key with satisfying value** —
  each row inside a `witness_table` body is
  `witness_table_entry(%Ix5Freq, %Sx5Freq)` — first operand is
  the requirement key, second is the satisfying value.
- **`interface_req_entry` operand shape** — an interface
  declaration `interface I { int run(int x); }` emits
  `interface_req_entry(%Ix5Frun, Func(Int, this_type(%I), Int))`
  — first operand is the requirement key, second is the
  requirement's function type with `this_type(%I)` as the
  receiver.
- **`interface_req_entry` is the operand of `interface(...)`** —
  the `interface(...)` opcode lists its `interface_req_entry`
  operands; the `let %I : Type = interface(%1)` line names them
  by value-id.
- **`global_generic_param`** — a module-scope `type_param T;`
  lowers to `let %T : Type = global_generic_param`.

### Untested claims (record under the bundle's out-of-scope heading)

The doc explicitly lists most of these opcodes as `(synthesized)`
by later IR passes — they do not appear at the LOWER-TO-IR stage
that this bundle observes.

- **`bind_global_generic_param`** — `(synthesized)` at link time;
  not observable from natural Slang surface at LOWER-TO-IR.
- **`globalValueRef`** — `(synthesized)` to carry references
  across an IR-pass boundary.
- **`makeExistentialWithRTTI`** — `(synthesized)` after
  specialization; `makeExistential` is what the natural surface
  produces.
- **`createExistentialObject`** — `(synthesized)` lower-level
  form used after specialization.
- **`wrapExistential`** — `(synthesized)` by the
  `BindExistentialsType` lowering pass; not produced at
  LOWER-TO-IR.
- **`getValueFromBoundInterface`** — `(synthesized)`.
- **`isNullExistential`** — `(synthesized)`.
- **`extractTaggedUnionTag`** / **`extractTaggedUnionPayload`** —
  `(synthesized)` by the type-flow specialization pass.
- **`rtti_object`** / **`GetSequentialID`** — `(synthesized)` by
  the RTTI-object pass; not observable at LOWER-TO-IR.
- **`GetDynamicResourceHeap`** — `(synthesized)`.
- **`packAnyValue`** / **`unpackAnyValue`** — `(synthesized)` by
  the existential-elimination pass.
- All `### Type-flow specialization` opcodes (`TypeSet`,
  `FuncSet`, `WitnessTableSet`, `GenericSet`, `MakeTaggedUnion`,
  `GetTagFromTaggedUnion`, etc.) — `(synthesized)` by the
  type-flow pass, post-specialization.
- **`thisTypeWitness`** / **`TypeEqualityWitness`** —
  `(synthesized)`.

If you would write a test for an opcode the doc lists as
`(synthesized)`, stop — that's an internal probe, not a
documented surface mapping.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for the
   synthesized-only opcodes listed above.
2. 15 to 25 `.slang` test files. Aim for one observable opcode (or
   one observable operand-shape claim) per test. Group
   tightly-related observations (e.g. the four opcodes that show
   up in an existential-dispatch sequence) into one file when the
   same surface construct gives the cleanest observation.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/ir-reference/generics-and-existentials.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/generated/design/ir-reference/structure.md`
- `docs/generated/design/ir-reference/types.md`
- `docs/generated/design/cross-cutting/ir-instructions.md`

If you would cite anything else, stop and record a doc-gap
finding in `README.md`.

## Test directives

The standard form used here is:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

Per the universal `_common.md` rule: combine `-dump-ir` with
**`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
goes to stdout uncontaminated by target text. Mostly use
`pipeline_stage=lower` in `//META` — these are LOWER-TO-IR
observations.

Anchor patterns at user-named top-level symbols and at the opcode
name (`makeExistential`, `extractExistentialType`,
`lookupWitness`, `specialize`, `witness_table_entry`,
`interface_req_entry`, `global_generic_param`) to cut through the
large IR-dump preamble. Use the `CHECK:` pattern prefix.

Outputs that escape DCE need to write to an
`RWStructuredBuffer<T>` — purely-internal computations are
removed before the dump.

Use `uniform` globals or `SV_DispatchThreadID` to defeat constant
folding when the body of a test invokes a function or uses a
value; literal arithmetic collapses before the dump records it.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `generics-and-existentials.md` (or one of the listed
      secondary docs).
- [ ] Every test uses `-target spirv-asm -dump-ir -o /dev/null
-entry main -stage compute` per CLAUDE.md.
- [ ] Outputs escape DCE: write to an `RWStructuredBuffer<T>` so
      the IR survives.
- [ ] Operands of any in-body call/arithmetic are non-constant
      (`uniform` globals or `SV_DispatchThreadID`) so constant
      folding does not collapse the surface that surfaces the
      opcode.
- [ ] CHECK patterns anchored at a user-named top-level symbol or
      at the opcode name — the IR-dump preamble must not match.
- [ ] No test depends on a GPU.
- [ ] No test asserts a C++ wrapper-struct identity
      (`MakeExistential`, `ExtractExistentialValue`, etc.) — the
      dump shows the camelCase opcode spelling
      (`makeExistential`, `extractExistentialValue`).
- [ ] README.md `## Doc gaps observed` is honest.

## Lessons captured (apply to this bundle as well)

- `-dump-ir` requires `-target <X>` and `-o /dev/null`.
- The IR dump prefixes user IR with a large preamble (capability
  tables, core-module annotations, autodiff witness tables).
  Anchor patterns at the opcode spelling itself or at user-named
  symbols. The first `### LOWER-TO-IR:` block is what we observe;
  subsequent passes (specialization, existential-elimination) are
  not anchored here.
- Mangled identifiers replace `_` with `x5F` (e.g. `IFoo.run` →
  `%IFoox5Frun`, `A.run` → `%Ax5Frun`). Use
  `{{[A-Za-z0-9_]+}}` wildcards rather than literal mangled
  names when feasible.
- `witness_table %N : witness_table_t(%I)(%S) { ... }` is the
  IR-dump spelling. The parenthesised `(%I)(%S)` pair is
  `(interface, implementing-type)`; both are operands of the
  witness-table type, not of the `witness_table` opcode itself.
- `call specialize(%generic, args...)(callArgs...)` is the
  syntax for a specialized-call site at LOWER-TO-IR. The first
  `(...)` is `specialize`'s argument list (the generic args);
  the second `(...)` is the `call`'s argument list (the runtime
  arguments). Pin the spelling carefully.
- `extractExistentialType(%i)` has result type `Type` (not the
  concrete type — the concrete type is the _value_ it returns).
  `let %T : Type = extractExistentialType(%i)`.
- The dispatch sequence at LOWER-TO-IR is:
  `let %t : Type = extractExistentialType(%i)` →
  `let %v : %t = extractExistentialValue(%i)` →
  `let %w : witness_table_t(%I) = extractExistentialWitnessTable(%i)` →
  `let %fn : Func(...) = lookupWitness(%w, %Ix5Freq)` →
  `let %r : RetT = call %fn(%v, %args)`.
- `LoadFromUninitializedMemory` shows up for default-initialized
  locals on the LOWER-TO-IR dump; this is not an existential
  opcode and tests should not assert on it.
