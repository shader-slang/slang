# Prompt: docs/generated/tests/spec/types-array/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/spec/types-array/`,
anchored to
[`docs/language-reference/types-array.md`](../../../../language-reference/types-array.md).

## High-value claims

- **Declaration syntax equivalence**: `var x : int[3]`, `int[3] x`,
  and `int x[3]` all declare the same thing.
- **Element-count inference**: `int a[] = {1, 2, 3, 4}` is
  equivalent to `int a[4] = {...}`.
- **Pass-by-value (NOT pointer-decay)**: Unlike C/C++, arrays in
  Slang are passed by value — mutating an array param does NOT
  affect the caller's array (Remark 4).
- **Index order in C-style declaration**: `int[2][3] arr[5][4]` is
  the same type as `int[2][3][4][5] arr` (Remark 2 — right-to-left
  on the variable-bound side, left-to-right on the type-bound side).
- **Basic subscript access**.

## What NOT to test here

- Memory layout claims (natural / C-style / D3D) — not observable
  from the slangc CLI without reflection. Record in untested.
- 0-length array subscript-runtime-rejection — would need a runtime
  bounds-check observable; deferred.
