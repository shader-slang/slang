---
generated: true
model: claude-opus-4-7
generated_at: 2026-06-01T17:00:00+00:00
source_commit: d25453d7f0b4867db4cb5eabf34fb6cd088cf596
watched_paths_digest: 7e48c2c3f5b6937ce21a8aadf2307d70e90df026caf8595744d7aa7344db94a7
source_doc: docs/language-reference/expressions-member-access.md
source_doc_digest: ab827430dedad798221ccb6a8d05027ec37ed381ec7b70905cec740e9ed7e864
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for language-reference/expressions-member-access

## Intent

Tests verify member-expression semantics claimed in the **language
reference** at
[`docs/language-reference/expressions-member-access.md`](../../../../language-reference/expressions-member-access.md):
struct field access (read + write), vector swizzle via `.rgba`
mapping (companion to the xyzw mapping covered in the
vector-and-matrix bundle), matrix swizzle via `_mij` zero-based
and one-based shorthand, and the `.` vs `::` equivalence for
static member access.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Struct field read: `base.fieldName` returns the named field's value. | functional | [#member-expression](../../../../language-reference/expressions-member-access.md#member-expression) | [`struct-field-read.slang`](struct-field-read.slang) |
| Struct field write: a member ref is an l-value when its base is an l-value and the member is mutable. | boundary | [#member-expression](../../../../language-reference/expressions-member-access.md#member-expression) | [`struct-field-write.slang`](struct-field-write.slang) |
| Vector swizzle via member: `.r`/`.g`/`.b`/`.a` map to indices 0/1/2/3 — the same mapping as `.x`/`.y`/`.z`/`.w`. | functional | [#vector-swizzles](../../../../language-reference/expressions-member-access.md#vector-swizzles) | [`vector-rgba-mapping.slang`](vector-rgba-mapping.slang) |
| Matrix zero-based swizzle: `m._mij` reads row i, column j (zero-indexed). | boundary | [#matrix-swizzles](../../../../language-reference/expressions-member-access.md#matrix-swizzles) | [`matrix-mij-zero-based.slang`](matrix-mij-zero-based.slang) |
| Static member access via `.` and `::` are equivalent: `Color.Red` and `Color::Red` produce the same value. | functional | [#static-member-expressions](../../../../language-reference/expressions-member-access.md#static-member-expressions) | [`static-dot-vs-colon-colon.slang`](static-dot-vs-colon-colon.slang) |


## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| Matrix one-based shorthand swizzle (`_41`, etc.). | (unclassified) | [#matrix-swizzles](../../../../language-reference/expressions-member-access.md#matrix-swizzles) | The one-based form is parallel to the zero-based `_mij`; coverage of one suffices for the underlying swizzle mechanism. A dedicated one-based test can be added without changing the bundle's design. |
| Implicit dereference through `ConstantBuffer<T>` and other pointer-like types. | needs-cli-test | [#implicit-dereference](../../../../language-reference/expressions-member-access.md#implicit-dereference) | The `ConstantBuffer<T>` binding-setup is not available in INTERPRET; a CLI/compute-dispatch test would need separate binding wiring. |
| Subscript and `.`-operator forms marked `TODO` in the doc. | (unclassified) | [#subscript-operator](../../../../language-reference/expressions-member-access.md#subscript-operator) | Doc section is empty (`> TODO`); revisit when filled in. |


## Doc gaps observed

NA
