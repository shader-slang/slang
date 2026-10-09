# Prompt: docs/generated/tests/conformance/generics/

See [`_common.md`](../../../_meta/prompts/_common.md).

## Target

Bundle at `docs/generated/tests/conformance/generics/`, anchored to
[`docs/language-reference/generics.md`](../../../../language-reference/generics.md).

## Claim-extraction strategy

The doc is split into the following normative sections; every claim in each is extracted:

- **Syntax** — grammar productions for generic struct/interface/typealias/function/
  constructor/subscript/parameter-decl/where-clause. Claims: each grammar production
  accepts a list of zero or more generic parameters; `let` vs traditional syntax are both
  valid; optional default values for type and value parameters.
- **Description** — what each parameter kind means: type param, value param (Boolean,
  integer), type param pack, value param pack; constraints apply to every element of a pack;
  value parameters cannot be constrained (only their type is declared); the `optional` modifier
  enables the `T is U` expression.
- **Type Checking** — before-specialization checking; equality constraint `T == U` makes T
  an alias for U; conformance constraint `T : U` makes T implement U; no assumptions beyond
  declared type for value parameters.
- **Parameter Binding** — explicit vs implicit vs mixed; promotion rule for ambiguous
  fundamental types (lowest-to-highest rank table); ambiguous value argument is an error;
  ambiguous non-fundamental type argument is an error; explicit binding resolves ambiguity.
- **Examples** — each example is a normative illustration of a specific combination:
  generic struct with type+value params; generic typealias for partial binding; generic
  function with IFunc param; generic constructor; generic subscript; type constraint;
  optional constraint; type equality constraint; type coercion constraint; type parameter
  pack; multiple packs; generic extension; explicit+implicit binding; implicit type+value binding;
  ambiguous value arg; ambiguous type arg.
- **Pack queries** — `__first`, `__last`, `__trimFirst`, `__trimLast`; partiality/totality;
  `where nonempty(P)` as the non-emptiness guard.

## What NOT to test here

- Generic enumeration declarations — the doc notes these are not currently useful.
- Type coercion constraints (`int(A)` in extensions) — doc restricts to generic extensions
  only; covered in `conformance/types-extension`.
- Remark 2 (`__generic` modifier) — the doc says "using generic-params-decl is recommended";
  the `__generic` form is an undocumented implementation detail.
- Optional constraints are marked experimental — tested but not treated as stable.
- `__first`/`__last` crash the compiler (SIGSEGV, exit 139) — see finding
  `pack-query-first-last-compiler-crash.yaml`; partial tests cover `nonempty` constraint
  instead.
