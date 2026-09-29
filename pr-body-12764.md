## Motivation

shader-slang/slang#12764 asks for targeted **migration diagnostics** that teach the *intentional*
differences between HLSL/C++ and native Slang, rather than leaving a porting user with a
context-free error or an ambiguity cascade. This PR delivers the "feasible-now" diagnostic slice —
three patterns from the issue's inventory that the compiler can already recognize safely, each of
which today emits a confusing multi-error cascade:

**① Redundant `struct` forward declaration.** Slang has order-independent lookup, so a C/HLSL
forward declaration is unnecessary:
```slang
struct Item;                 // redundant in Slang
struct Item { uint value; }
Item makeItem(uint v) { ... }
```
Today this reports *two* errors: `error[E30200] conflicting declaration`, and then — because both
declarations remain visible at the use site — `error[E39999] ambiguous reference to 'Item'` with two
`candidate: struct Item` notes. The second error is a pure consequence of the first.

**③ HLSL/C++ `operator[]`.** Slang spells an indexing operator as `__subscript`:
```slang
int operator[](int index) { ... }
```
Today: `error[E20008] invalid operator '['` *and* a follow-on `error[E20001] unexpected ']'`, with no
hint about the Slang spelling.

**⑧ Value-dependent array extent hitting fixed-extent overloads.**
```slang
void decode(Data v[1]) {}  void decode(Data v[2]) {}  void decode(Data v[3]) {}
void getData<let N : uint>(inout Data values[N]) { decode(values); }
```
The generic body is checked while `N` is still abstract, so `values` has type `Data[int(N)]` and no
fixed-extent `decode` overload applies: `error[E39999] no overload for 'decode' applicable to
arguments of type (Data[int(N)])`. The error is correct but gives no clue *why* it is intrinsic to
pre-specialization generic-body checking.

## Proposed solution

Improve the message and remove the dependent cascade for each pattern, **without changing what the
compiler accepts or rejects** — all three remain compile errors (issue #2557 already declined struct
forward declarations and non-inline methods *as language features*; this is the complementary "teach
the difference" path). This keeps the change `pr: non-breaking`.

- **①** In `SemanticsVisitor::checkRedeclaration`, detect a non-generic struct-vs-struct pair where
  exactly one side is a body-less forward declaration and the other is a definition. Emit one root
  diagnostic (`E30203`) at the redundant forward declaration with a note pointing at the complete
  declaration (both order-neutral — the note locates the definition regardless of source order),
  then mark the forward declaration `hiddenFromLookup` so later references resolve to the single
  surviving declaration and never become ambiguous. This suppresses the `E39999` cascade at its
  cause rather than filtering the symptom downstream. Generic structs are excluded (lookup finds the
  enclosing `GenericDecl`, not the hidden inner `StructDecl`) and link-time aliases are excluded
  (they are body-less but not forward declarations).
- **③** Add a `TokenType::LBracket` arm to `ParseDeclName`'s operator switch that consumes the
  matching `]` (killing the "unexpected ']'" parse error) and emits one targeted diagnostic
  (`E20021`) pointing at `__subscript`.
- **⑧** In the zero-applicable-candidate path of `ResolveInvoke`, attach an explanatory note
  (`E40021`) naming the two supported native formulations — but only when a single rejected argument
  slot pairs a non-literal-extent (generic value parameter) array *actual* with a literal-extent
  array *parameter*, read from that one candidate's `argMismatchActualType`/`argMismatchExpectedType`.
  Pairing per-slot ensures the note never fires on an unrelated overload mismatch that merely
  happens to involve arrays.

## Change summary

| File | Change |
| --- | --- |
| `source/slang/slang-diagnostics.lua` | New diagnostics: `redundant-struct-forward-declaration` (E30203, err + note), `operator-subscript-should-use-subscript` (E20021, err), `array-argument-extent-is-generic-value-parameter` (E40021, standalone_note). |
| `source/slang/slang-check-decl.cpp` | `checkRedeclaration`: struct forward-decl-vs-definition branch → one root diagnostic + hide the forward decl from lookup. |
| `source/slang/slang-lookup.cpp` | `_lookUpDirectAndTransparentMembers`: skip a **non-local** `hiddenFromLookup` member in the semantic branch. |
| `source/slang/slang-parser.cpp` | `ParseDeclName`: `LBracket` arm → consume `]`, emit the `__subscript` diagnostic. |
| `source/slang/slang-check-overload.cpp` | New `isArrayWithNonLiteralExtent` helper + generic-array-extent migration note in the no-applicable-overload path. |
| `tests/diagnostics/struct-forward-declaration-12764.slang` | ① regression: asserts E30203 + note and no E39999 cascade. |
| `tests/diagnostics/struct-forward-declaration-generic-12764.slang` | ① negative: a generic struct forward decl does not take the E30203 path. |
| `tests/diagnostics/operator-subscript-12764.slang` | ③ regression: asserts E20021, no E20008/E20001. |
| `tests/diagnostics/generic-array-extent-overload-12764.slang` | ⑧ regression: asserts the E40021 note follows E39999, plus a negative scalar-parameter case that must not get the note. |

## Concepts and vocabulary

- **`AggTypeDecl::hasBody`** — set to `false` only by the parser's `struct Name;` semicolon path. A
  body-bearing `struct Name { ... }` and a **link-time alias** `struct Name : IBar = Baz;` both keep
  the default `true`; the alias is excluded from ① by testing `aliasedType.exp`.
- **`Decl::hiddenFromLookup`** — a flag previously honored only for block-scope locals (via
  `_isUncheckedLocalVar`, gated on `isLocalVar`) for in-order visibility. ① reuses it for a
  *permanent* hide of a redundant forward declaration; the new lookup skip is gated on
  `!isLocalVar` so the two uses never interfere.
- **Redeclaration check timing** — `checkForRedeclaration` runs at `DeclCheckState::ReadyForReference`,
  and module checking advances *all* declarations phase-by-phase, so the hide decision is in place
  before any use-site lookup runs.
- **`argMismatchExpectedType`** — per-candidate record of the parameter type an argument failed to
  match; ⑧ reads it to confirm a rejected candidate genuinely expected a literal-extent array.

## Process report

**① `checkRedeclaration` branch (slang-check-decl.cpp).** The producing shape is two `StructDecl`s
with the same name in one container, exactly one of which has `hasBody == false`. That is a valid,
intentional input shape: the parser deliberately accepts `struct Item;` (semicolon path,
`hasBody=false`) and a subsequent `struct Item { ... }`. The prior code fell through to the generic
`Redeclaration` (E30200), and — because both decls stayed visible — the use site built an
`OverloadedExpr` and `diagnoseAmbiguousReference` fired E39999. The fix is at the right layer: the
redundancy is a property of the *declaration pair*, so it is recognized where redeclarations are
already checked, and the cascade is cut at its cause (the forward decl is removed from lookup)
rather than by suppressing the downstream ambiguity diagnostic. Three guards keep it precise:
(a) genuine conflicts are untouched — two definitions (`hasBody == true` both) and two forward
declarations (`false` both) fail the `!=` test and keep the generic error; (b) a link-time alias
(`struct Foo = Bar;`, body-less but `hasBody == true`) is excluded via `aliasedType.exp`; (c)
generic structs are excluded (`eitherIsGeneric`) because `checkRedeclaration` unwraps `GenericDecl`
to its inner `StructDecl`, but lookup finds the enclosing `GenericDecl` — hiding the inner decl
would not suppress the cascade, so generics keep their existing diagnostics. The diagnostic is
order-neutral: the note locates the complete declaration regardless of whether it precedes or
follows the forward declaration. Returning `SLANG_OK` lets `checkForRedeclaration`'s loop continue,
so with multiple forward decls each is hidden.

**① lookup skip (slang-lookup.cpp).** `hiddenFromLookup` was read only inside `_isUncheckedLocalVar`
(local-var-gated), so setting it on a struct member had no effect until this skip was added. The
skip is placed in the semantic member-iteration branch beside the existing `declToExclude` skip and
is gated `&& !isLocalVar(m)`: block-scope locals continue to use `hiddenFromLookup` for their
separate, transient in-order-visibility mechanism (`slang-check-stmt.cpp`), and the
`ConsiderAllLocalNamesInScope` behavior for locals is unchanged. The completion-request branch is
intentionally left alone — it is only reached for broken source that is already a hard error.

**③ `LBracket` arm (slang-parser.cpp).** `operator[]` is a recognizable, intentional HLSL/C++ shape.
`operator()` is already handled one case above; `[` was simply missing and fell into the `default`
"invalid operator" arm, and the unconsumed `]` then produced a second parse error. Consuming the
`]` and setting `isValidOperator = false` reuses the existing non-function-operator handling
(`isOperatorName` stays false, so no second `OperatorNameOnNonFunction` fires) and emits one focused
diagnostic. The note names `__subscript` only; it deliberately does not prescribe `get`/`set`/`ref`
forms (the issue notes those are not uniformly portable — that belongs in the migration doc).

**⑧ overload note (slang-check-overload.cpp).** The input shape is an argument of type
`ArrayExpressionType` whose `getElementCount()` is not a `ConstantIntVal` (for `int(N)` it is a
`TypeCastIntVal` over a `DeclRefIntVal`). That shape is correct and expected — it is exactly what a
generic value parameter used as an array extent produces during pre-specialization generic-body
checking, which the issue explicitly does *not* want to change. So the fix adds only an explanatory
note, at the existing no-applicable-overload emit site. The guard reads a *single* candidate's
`argMismatchActualType` and `argMismatchExpectedType` and fires only when that one slot pairs a
non-literal-extent array actual with a literal-extent array parameter — so a mismatch whose actual
is a generic-extent array but whose failing parameter is a scalar (or any unrelated array mismatch)
does not get the note. `as<>` is null-safe, so a null `argMismatch*Type` or null element count is
handled; the loop breaks after the first match so the note is emitted once.

**Testing.** The four new `.slang` regression tests were confirmed to fail against the pre-fix
binary (improved diagnostic absent / cascade present) and to pass after. `tests/diagnostics/`
(751/751, 8 ignored) and `tests/language-feature/` (2221/2221) pass with no regressions.

Closes #12764 is intentionally **not** used: #12764 is a maintainer's umbrella epic covering seven
patterns plus a migration doc and an agent skill. This PR resolves the feasible-now diagnostic slice
(patterns ①, ③, ⑧); the doc page, the agent skill, and the needs-infra patterns (② out-of-line
member definitions, ⑦ member-constraint→constrained-extension) remain for the epic.
