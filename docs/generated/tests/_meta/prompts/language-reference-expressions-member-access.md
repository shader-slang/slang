# Prompt: docs/generated/tests/language-reference/expressions-member-access/

See [`_common.md`](_common.md).

## Target

[`docs/language-reference/expressions-member-access.md`](../../../../language-reference/expressions-member-access.md).

## High-value claims

- **Struct field access**: `base.m` reads / writes a named field.
  Member ref is l-value when both the base is an l-value AND the
  member is mutable.
- **Vector swizzle via member: rgba mapping**: `.r`/`.g`/`.b`/`.a`
  maps to indices 0/1/2/3 (same as `.x`/`.y`/`.z`/`.w` per the
  sibling vector-and-matrix doc).
- **Matrix `_mij` (zero-based) swizzle**: e.g. `m._m01` is row 0 col 1.
- **Matrix shorthand (one-based) swizzle**: e.g. `m._41` is row 4 col 1 (1-based).
- **Static member with `.` and `::`**: `Color.Red` and `Color::Red`
  are equivalent.

## What NOT to test

- xyzw swizzle (already covered in
  `language-reference/types-vector-and-matrix`).
- Generic / overloaded member-name disambiguation — large surface;
  defer.
- Implicit dereference through `ConstantBuffer<T>` — needs a binding
  setup that INTERPRET doesn't expose; defer.
