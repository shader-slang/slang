# Prompt: docs/generated/tests/ir-reference/structure/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/ir-reference/structure/`,
anchored to
[`docs/generated/design/ir-reference/structure.md`](../../../docs/generated/design/ir-reference/structure.md).

Audience: nightly CI. This bundle is the **per-opcode reference**
for the structural / hierarchy opcodes that organize an IR module:

- `module` (the root container, observable implicitly as the
  preamble that wraps every dump);
- `func` (function-shaped parent owning blocks; signature carried
  on its `Func(...)` result type);
- `generic` (function-shaped parent whose single block computes a
  type-level value and ends with `return_val(Func(...))`);
- `param` _used in its structural role_ — function-scope params on
  the entry block of a `func` or `generic` (the block-parameter /
  phi-replacement role is anchored in `ir-reference/control-flow`,
  not here);
- `global_var`, `global_param`, `globalConstant` (module-scope
  state);
- `struct` and `class` parents owning `field` and `key` children;
- `interface` and `interface_req_entry` (interface declarations and
  their requirement slots);
- `witness_table` and `witness_table_entry` (the dispatch tables
  that satisfy interface requirements);
- `lookupWitness` (the structural consumer of a `witness_table`
  inside a generic body, used to look up a satisfying value by
  requirement key);
- `key` / `StructKey` (the identity opcode that addresses fields
  and interface requirements across compilation units).

This bundle is adjacent to:

- `cross-cutting/ir-instructions` — the category-level view that
  samples each IR family at one anchor. This bundle drills the
  **structural** family in detail; it does not duplicate the
  category-level sampling.
- `ir-reference/values` — the value-producing opcode catalog
  (`add`, `mul`, `FieldExtract`, `FieldAddress`, ...). Tests in
  this bundle observe `field` / `key` _structurally_ (i.e. as
  children of a `struct` parent), not as operands of a field-access
  opcode in a function body.
- `ir-reference/control-flow` — the block / param / terminator
  family. This bundle uses `param` only in its function-signature
  role (the entry block of a `func` carrying the function's
  declared parameters) and does not anchor block-parameter / phi
  observations here.
- `ir-reference/generics-and-existentials` — `specialize`,
  existential-extract, and the dispatch-side semantics of
  `lookupWitness`. This bundle anchors `lookupWitness` from the
  _structural_ side (it's the consumer of a `witness_table` and
  carries a `key` selector); deeper dispatch-side semantics live
  in the existentials bundle.

Anchor each test at the IR structural opcode that the doc names;
do not write tests that observe opcodes the doc lists as
`(synthesized)` or marks without an `AST origin` (`SymbolAlias`,
`indexedFieldKey`, `thisTypeWitness`, `TypeEqualityWitness`,
`global_hashed_string_literals`, `global_generic_param`) unless
you can find a natural surface that produces them at the
LOWER-TO-IR stage. Record any non-natural ones under
`## Untested claims` in `README.md`.

## The translation rule: claims to observations

`structure.md` describes each structural opcode in tables grouped
by role (`Module`, `Functions and generics`, `Global state`,
`Struct internals`, `Interface internals`, `Witness tables and
witness facts`, `Symbol aliasing`). Each row names an `Opcode`,
`C++ wrapper`, `Operands`, `Flags`, `AST origin`, and `Summary`.
The testable consequences are:

- **"AST declaration X produces structural opcode Y with parent-
  child shape Z"** — compile to a text target with `-dump-ir` and
  FileCheck for the opcode name and its expected child shape in
  the IR dump. This is the primary mode.
- **"The parent opcode owns these children"** — observable as the
  emitted IR-dump form `parent %name : Type { child(...) child(...) }`
  (e.g. `struct %P { field(%x, Int) field(%y, Int) }`,
  `witness_table %t : witness_table_t(%I)(%S) { witness_table_entry(...) }`).
- **"This opcode carries this kind of decoration"** — `key` carries
  `[export("key__...")]` linkage; `func` carries `[nameHint("...")]`
  / `[export("...")]`; `global_var` and `global_param` carry their
  module-scope linkage decorations.

### Observable claims (write tests for these)

The catalog row → claim mapping that is reliably observable from
natural surface code via `-dump-ir`:

- **`func` parent** — every Slang function (entry point or
  user-defined) emits a `func %name : Func(<retT>, <paramT>...) { ... }`
  parent. The function signature is on the `Func(...)` type, and
  the entry block's `param` children match the declared
  parameters in order.
- **`func` body shape** — the entry block contains the function's
  `Param`s and ends with `return_val(...)`; user-named helpers
  give the dump a stable anchor (`func %helper`) for FileCheck.
- **`generic` parent** — a generic function `T foo<T>(T x)` emits a
  `generic %N : Generic { block %M(param %T : type_t, ...): ... return_val(Func(...)) }`.
  The body is single-block and ends with a `return_val` whose
  operand is the type-level result. (The doc text says "ends with
  `yield`"; observe the actual IR-dump spelling — `return_val(Func(...))`
  at the LOWER-TO-IR stage — and record any discrepancy as a doc
  gap.)
- **`global_var`** — a `static int g = 0;` at module scope lowers
  to `global_var %g : Ptr(Int)` with a one-block initializer body
  ending in `return_val(<init>)`.
- **`global_param`** — a `uniform T x;` at module scope lowers to
  `let %x : T = global_param`.
- **`globalConstant`** — a `static const int K = 42;` at module
  scope lowers to `let %K : Int = globalConstant(42 : Int)`.
- **`struct` parent + `field` children + `key`** — a `struct P { int x; int y; }`
  emits `struct %P : Type { field(%x, Int) field(%y, Int) }`, and
  each field name `x`, `y` becomes a separate `let %x : _ = key`
  declaration with `[export("key__...")]` linkage. The
  `field`-`key` pairing is the structural anchor.
- **`class` parent** — a `class C { int n; }` emits
  `class %C : Type { field(%n, Int) }` with the same `field`-`key`
  shape as `struct`. The opcode spelling differs (`class` vs
  `struct`); tests should pin the spelling.
- **`interface` parent + `interface_req_entry`** — an
  `interface I { int run(int x); }` emits
  `let %1 : _ = interface_req_entry(%Ix5Frun, Func(Int, this_type(%I), Int))`
  followed by `let %I : Type = interface(%1)`. The
  `interface_req_entry` opcode is the requirement slot; its
  first operand is the `requirementKey` and its second operand is
  the requirement type.
- **`witness_table` parent + `witness_table_entry` children** — a
  `struct S : I { int run(int x) { ... } }` emits
  `witness_table %t : witness_table_t(%I)(%S) { witness_table_entry(%Ix5Frun, %Sx5Frun) }`
  pairing the same `requirementKey` with the concrete
  implementing function value.
- **`lookupWitness` structural consumer** — a generic
  `int helper<T : I>(T t, int x) { return t.run(x); }` lowers,
  inside the generic body, to
  `let %V : Func(...) = lookupWitness(%witnessParam, %Ix5Frun)`;
  the structural shape is `(witnessTable, requirementKey)`.
- **`call` of a `func`** — the doc's `Functions and generics` row
  lists `call` as part of the structural family
  (`callee, args...`); observable as
  `let %r : Int = call %helper(%a, %b)` in the caller body. This
  drills the call-and-callee structural link.

### Untested claims (record under the bundle's out-of-scope heading)

- **`module` / `ModuleInst`** — observable only as the implicit
  preamble that wraps every dump. There is no anchor inside an
  IR dump that uniquely identifies the module instruction in a
  FileCheck-able way without coupling to dump preamble formatting.
  Record as out of scope.
- **`SymbolAlias`** — `(synthesized as part of linking)` per the
  doc; not observable at LOWER-TO-IR.
- **`indexedFieldKey`** — `(synthesized)`; no natural surface.
- **`thisTypeWitness`** — `(synthesized inside InterfaceDecl
  lowering)`; not portably observable in dump form.
- **`TypeEqualityWitness`** — `(synthesized)`.
- **`global_hashed_string_literals`** — `(synthesized)` container
  for the hashed-string-literal pool.
- **`global_generic_param`** — a `GenericTypeParamDecl` at module
  level (not nested under a `generic`) is non-trivial to produce
  from natural Slang surface; out of scope unless a clean surface
  is found.

If you would write a test for an opcode the doc lists as
`(synthesized)`, stop — that's an internal probe, not a
documented surface mapping.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for the
   unobservable-via-slangc items listed above.
2. 15 to 25 `.slang` test files. Aim for one observable opcode (or
   one observable parent/child structural pair) per test. Group
   tightly-related observations (e.g. `field`-and-`key` pairing
   under a `struct` parent) into one file when the same surface
   construct gives the cleanest single-file observation.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/ir-reference/structure.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/generated/design/cross-cutting/ir-instructions.md`
- `docs/generated/design/ir-reference/types.md`
- `docs/generated/design/ir-reference/generics-and-existentials.md`
- `docs/generated/design/pipeline/04-ast-to-ir.md`

If you would cite anything else, stop and record a doc-gap
finding in `README.md`.

## Test directives

Structural-opcode claims are best observed through `-dump-ir`
against the IR dump. The standard form used here is:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

Per the universal `_common.md` rule: combine `-dump-ir` with
**`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
goes to stdout uncontaminated by target text. Mostly use
`pipeline_stage=lower` in `//META` — these are LOWER-TO-IR
observations.

Anchor patterns at the user-named top-level symbol the test
observes (`struct %P`, `class %C`, `func %helper`, `func %main`,
`let %gMut`, `witness_table %t`) to cut through the large IR-dump
preamble. Use the `CHECK:` pattern prefix.

Outputs that escape DCE need to write to an
`RWStructuredBuffer<T>` — purely-internal computations are
removed before the dump. For structural-opcode observations the
top-level declaration is observed before any DCE pass would have
elided it, but keep an entry-point that consumes the declaration
so the linker keeps it alive.

Use `uniform` globals or `SV_DispatchThreadID` to defeat constant
folding when the body of a structural test invokes a function or
uses a value; literal arithmetic collapses before the dump
records it.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ir-reference/structure.md` (or one of the listed secondary
      docs).
- [ ] Every test uses `-target spirv-asm -dump-ir -o /dev/null
      -entry main -stage compute` per CLAUDE.md.
- [ ] Outputs escape DCE: write to an `RWStructuredBuffer<T>` so
      the structural declaration stays linked.
- [ ] Operands of any in-body call/arithmetic are non-constant
      (`uniform` globals or `SV_DispatchThreadID`) so constant
      folding does not collapse the surface that surfaces the
      declaration.
- [ ] CHECK patterns anchored at a user-named top-level symbol —
      the IR-dump preamble must not match.
- [ ] No test depends on a GPU.
- [ ] No test asserts a C++ wrapper-struct identity (`IRFunc`,
      `IRStructType`, `IRInterfaceType`, etc.) — the dump shows
      the lowercase opcode spelling (`func`, `struct`, `interface`).
- [ ] README.md `## Doc gaps observed` is honest.

## Lessons captured (apply to this bundle as well)

These bite hard in structural-opcode observation tests:

- `-dump-ir` requires `-target <X>` and `-o /dev/null`.
- The IR dump prefixes user IR with a large preamble (capability
  tables, core-module annotations). Anchor patterns at user-named
  symbols (`struct %P`, `func %helper`, `let %gConst`).
- Mangled identifiers replace `_` with `x5F` (e.g. `IFoo.run` →
  `%IFoox5Frun`). Use `{{[A-Za-z0-9_]+}}` wildcards rather than
  literal mangled names in CHECK patterns when feasible.
- A user-named class/struct/global keeps its source identifier as
  the dump's `%name`. A function with a method-like dotted name
  emits with `x5F` substitution: `Doubler.compute` →
  `%Doublerx5Fcompute`. Pin the un-dotted form for top-level
  symbols and use wildcards for dotted method functions.
- `witness_table %t : witness_table_t(%I)(%S) { ... }` is the IR-
  dump spelling. The parenthesised `(%I)(%S)` pair is `(interface)
  (implementing type)`; both are operands of the witness-table
  type, not of the `witness_table` opcode itself.
- The doc says `generic` ends with `yield`; the actual LOWER-TO-IR
  dump shows `return_val(Func(...))` (the `yield` opcode is the
  internal spelling; lowering emits a `return_val` of the type-
  level result). Either anchor on `return_val(Func` in a generic
  body or treat the divergence as a doc gap.
- Trivial `static int g = 0; ... g + 1;` may be inlined away by
  the time later IR passes run. The LOWER-TO-IR-stage dump (the
  first `### LOWER-TO-IR:` block) preserves the `global_var`
  declaration; that is the section to FileCheck.
- `static const int K = 42;` emits as `let %K : Int = globalConstant(42 : Int)`,
  not as a plain `let %K : Int = 42 : Int`. Pin the `globalConstant`
  spelling so a future constant-propagation change is caught.
