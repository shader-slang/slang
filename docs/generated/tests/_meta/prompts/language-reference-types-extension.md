# Prompt: docs/generated/tests/language-reference/types-extension/

See [`_common.md`](_common.md).

## Target

[`docs/language-reference/types-extension.md`](../../../../language-reference/types-extension.md).

## High-value claims

- **Struct extension adds member functions** (Example 1).
- **Struct extension provides interface requirements** for a struct
  that declares conformance via `: IReq` but doesn't implement
  inline (Example 2).
- **Struct extension adds interface conformance** to a struct that
  did NOT originally declare conformance (Example 3) — different
  from Example 2.
- **Enum extension** adds static data, static functions, and
  non-static member functions where `this` is the enumeration
  value.

## What NOT to test

- Generic extensions and where-clauses — defer; rich enough for
  their own bundle.
- Interface-cannot-be-extended (Remark) — negative test; useful
  but defer.
- The known undefined-which-member-takes-effect case from issue
  #9660 — record as a `## Doc gaps observed` warning rather than
  test (the doc explicitly says the behaviour is undefined).
