---
generated: true
model: claude-opus-4-7
generated_at: 2026-06-01T15:30:00+00:00
source_commit: d25453d7f0b4867db4cb5eabf34fb6cd088cf596
watched_paths_digest: 609ff4e746cefdbe56991a8a4c083d5d6012c7547fdc7d66f60ae026d16af186
source_doc: docs/language-reference/types-vector-and-matrix.md
source_doc_digest: b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for language-reference/types-vector-and-matrix

## Intent

Tests verify the vector and matrix semantics claimed in the
**language reference** at
[`docs/language-reference/types-vector-and-matrix.md`](../../../../language-reference/types-vector-and-matrix.md):
element access (subscript and named-component), swizzle extract
and assign, element-wise unary and binary operators, scalar
broadcasting, matrix element/row access, and the `mul()`
vs `*` distinction for matrix multiplication.

## Functional coverage

| Claim                                                                                                                                                     | Intent     | Anchor                                                                                                        | Tests                                                                          |
| --------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| Vector subscript access: `v[0]` is the first element, indices 0 through N-1 are valid.                                                                    | functional | [#element-access](../../../../language-reference/types-vector-and-matrix.md#element-access)                   | [`vector-subscript-access.slang`](vector-subscript-access.slang)               |
| Vector named-component access: `.x`/`.y`/`.z`/`.w` map to indices 0/1/2/3.                                                                                | functional | [#element-access](../../../../language-reference/types-vector-and-matrix.md#element-access)                   | [`vector-xyzw-access.slang`](vector-xyzw-access.slang)                         |
| Vector swizzle extract: `.xy` returns a `vector<T, 2>`; an element may be repeated (e.g. `.xww`).                                                         | boundary   | [#element-access](../../../../language-reference/types-vector-and-matrix.md#element-access)                   | [`vector-swizzle-extract.slang`](vector-swizzle-extract.slang)                 |
| Vector swizzle assign: assigning to `v.xz` writes the named columns (target columns must be unique).                                                      | boundary   | [#element-access](../../../../language-reference/types-vector-and-matrix.md#element-access)                   | [`vector-swizzle-assign.slang`](vector-swizzle-assign.slang)                   |
| Unary arithmetic operator on a vector applies element-wise (e.g. `-v`).                                                                                   | functional | [#operators](../../../../language-reference/types-vector-and-matrix.md#operators)                             | [`vector-unary-elementwise.slang`](vector-unary-elementwise.slang)             |
| Binary arithmetic with a scalar broadcasts the scalar across all vector elements (e.g. `v - 1`, `4 - v`).                                                 | boundary   | [#operators](../../../../language-reference/types-vector-and-matrix.md#operators)                             | [`vector-binary-scalar-broadcast.slang`](vector-binary-scalar-broadcast.slang) |
| Binary arithmetic between two vectors of the same length is applied element-wise.                                                                         | functional | [#operators](../../../../language-reference/types-vector-and-matrix.md#operators)                             | [`vector-binary-elementwise.slang`](vector-binary-elementwise.slang)           |
| Matrix element access: `m[r][c]` indexes row r, column c.                                                                                                 | functional | [#row-and-element-access-](../../../../language-reference/types-vector-and-matrix.md#row-and-element-access-) | [`matrix-elem-access.slang`](matrix-elem-access.slang)                         |
| `*` performs element-wise multiplication on same-shaped matrices; **`mul()` performs matrix multiplication** — the doc's explicit GLSL-divergence remark. | boundary   | [#operators](../../../../language-reference/types-vector-and-matrix.md#operators)                             | [`matrix-mul-vs-star.slang`](matrix-mul-vs-star.slang)                         |

## Untested claims

| Claim                                                                     | Reason                | Anchor                                                                                                        | Why untested                                                                                                                                                                                                                          |
| ------------------------------------------------------------------------- | --------------------- | ------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Memory layout: `N` contiguous values of `T` with no padding.              | internal-source-fact  | [#memory-layout](../../../../language-reference/types-vector-and-matrix.md#memory-layout)                     | Layout is not observable from the slangc CLI without reflection.                                                                                                                                                                      |
| Alignment range claim: at least `align(T)`, at most `N * align(T)`.       | internal-source-fact  | [#memory-layout](../../../../language-reference/types-vector-and-matrix.md#memory-layout)                     | Target-defined and not observable through `printf`.                                                                                                                                                                                   |
| Standard type aliases (`floatN`, `int32_tN`, `boolN`, etc.).              | out-of-bundle         | [#standard-type-aliases](../../../../language-reference/types-vector-and-matrix.md#standard-type-aliases)     | Each alias is exercised implicitly by every test in this bundle that uses `int4` / `vector<int, 4>` etc. A dedicated "alias works" test would be a no-op.                                                                             |
| Matrix type dimension range `[1, 4]` for rows/columns.                    | out-of-bundle         | [#matrix-types](../../../../language-reference/types-vector-and-matrix.md#matrix-types)                       | Boundary tests for the range would need `matrix<T, 5, 5>` to reject and `matrix<T, 1, 1>` to accept; deferred — focus on the more-bug-prone `mul`-vs-`*` distinction.                                                                 |
| Matrix subscript `m[r][c]` visible in SPIRV emission.                     | implementation-detail | [#row-and-element-access-](../../../../language-reference/types-vector-and-matrix.md#row-and-element-access-) | SPIRV constant-folds matrix literals into OpConstant even with function-wrapper approach; no matrix-subscript opcode survives. Covered on HLSL/GLSL/Metal/WGSL/CUDA/CPP.                                                              |
| WGSL integer-matrix `mul()` correctness (functional value check on WGSL). | implementation-detail | [#operators](../../../../language-reference/types-vector-and-matrix.md#operators)                             | WGSL lowers integer matrices to `array<vec2<i32>, N>` and the `mul()` lowering discards the result in the emitted body; only the emission skeleton is checked, not runtime correctness. Float-matrix `mul()` on WGSL emits correctly. |

## Doc gaps observed

NA
