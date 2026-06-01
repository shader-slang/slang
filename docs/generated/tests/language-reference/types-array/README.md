---
generated: true
model: claude-opus-4-7
generated_at: 2026-06-01T16:00:00+00:00
source_commit: d25453d7f0b4867db4cb5eabf34fb6cd088cf596
watched_paths_digest: 54362a1302edcff4977438bcbbe74abb68f67cc4d8f847d9beedd40413682d05
source_doc: docs/language-reference/types-array.md
source_doc_digest: ad7e70e01944de764a4e5bc82497a352737e4133daa640a2bfae8511b826b4e5
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for language-reference/types-array

## Intent

Tests verify array-type claims in the **language reference** at
[`docs/language-reference/types-array.md`](../../../../language-reference/types-array.md):
declaration-syntax equivalence, length inference from initializers,
pass-by-value semantics (the explicit GLSL/C-divergence remark),
subscript access, and the right-to-left index-order rule for
C-style declarations.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| The three declaration forms `var x : int[3]`, `int[3] x`, and `int x[3]` declare the same type. | functional | [#declaration-syntax](../../../../language-reference/types-array.md#declaration-syntax) | [`declaration-syntax-equivalence.slang`](declaration-syntax-equivalence.slang) |
| Element-count inference: `int a[] = {1, 2, 3, 4}` is equivalent to `int a[4] = {...}`. | boundary | [#element-count-inference-for-unknown-length-array](../../../../language-reference/types-array.md#element-count-inference-for-unknown-length-array) | [`element-count-inferred.slang`](element-count-inferred.slang) |
| Arrays are passed by **value**, not by pointer-decay. Mutating an array parameter inside a callee does NOT mutate the caller's array. (Doc Remark 4) | boundary | [#declaration-syntax](../../../../language-reference/types-array.md#declaration-syntax) | [`pass-by-value-not-decay.slang`](pass-by-value-not-decay.slang) |
| Basic subscript access: `arr[i]` reads element i; `arr[i] = v` writes. | functional | [#declaration-syntax](../../../../language-reference/types-array.md#declaration-syntax) | [`subscript-read-write.slang`](subscript-read-write.slang) |
| Multi-dimensional array element access: `arr[i][j]` for `int[N][M]`. | functional | [#declaration-syntax](../../../../language-reference/types-array.md#declaration-syntax) | [`multidim-subscript.slang`](multidim-subscript.slang) |
| `inout` array parameter: caller's array IS visible to the callee, and mutations propagate back. | boundary | [#declaration-syntax](../../../../language-reference/types-array.md#declaration-syntax) | [`inout-array-propagates.slang`](inout-array-propagates.slang) |


## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| Natural / C-style / D3D constant-buffer layout rules (stride, alignment, total size). | internal-source-fact | [#memory-layout](../../../../language-reference/types-array.md#memory-layout) | Not observable from the slangc CLI without reflection. |
| 0-length array subscript-rejection at runtime. | (unclassified) | [#declaration-syntax](../../../../language-reference/types-array.md#declaration-syntax) | The doc says "0-length arrays may not be accessed during runtime using the subscript operator" but the diagnostic surface and the timing (compile-time vs runtime) are unclear; deferred. |
| Unknown-length arrays in struct (last-member only), out/inout restriction. | (unclassified) | [#declaration-syntax](../../../../language-reference/types-array.md#declaration-syntax) | The restrictions are negative tests for invalid placements; useful to write but not the highest-leverage. |
| C-style nested-declarator index-ordering (Remark 2). | (unclassified) | [#declaration-syntax](../../../../language-reference/types-array.md#declaration-syntax) | The right-to-left-vs-left-to-right rule is subtle. A correctness test requires reading element values at known multi-dim positions; deferred until a sibling type-traits or reflection-API surface is exercised. |


## Doc gaps observed

NA
