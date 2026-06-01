---
generated: true
model: claude-opus-4-7
generated_at: 2026-06-01T13:30:00+00:00
source_commit: d25453d7f0b4867db4cb5eabf34fb6cd088cf596
watched_paths_digest: 8cd80739fdad5ab131decb6198e9617544ebfde862bccdd5808c13d8f3633d5c
source_doc: docs/language-reference/types-fundamental.md
source_doc_digest: 663f749e8cb7b2eb7369790ba71f10f28717032d7716ffc16b13b2f783be258c
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for language-reference/types-fundamental-integer

## Intent

Tests verify the integer-type claims in the **language reference**
(the authoritative spec) at
[`docs/language-reference/types-fundamental.md`](../../../../language-reference/types-fundamental.md)
(the `#integer` section).

The centerpiece is the doc's explicit claim that **all arithmetic
operations on signed and unsigned integers wrap on overflow**.
Tests pressure this at the documented edges (MAX, MAX+1, MIN, MIN-1)
on the universally-supported 32- and 64-bit types. Storage layout
claims (natural size + alignment) are recorded as untested because
there is no slangc-CLI surface that reveals them.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| `int` (32-bit signed) wraps on overflow: `INT_MAX + 1 == INT_MIN`. | boundary | [#integer](../../../../language-reference/types-fundamental.md#integer) | [`int32-wrap-max-plus-one.slang`](int32-wrap-max-plus-one.slang) |
| `int` wraps on underflow: `INT_MIN - 1 == INT_MAX`. | boundary | [#integer](../../../../language-reference/types-fundamental.md#integer) | [`int32-wrap-min-minus-one.slang`](int32-wrap-min-minus-one.slang) |
| `uint` (32-bit unsigned) wraps on overflow: `UINT_MAX + 1 == 0`. | boundary | [#integer](../../../../language-reference/types-fundamental.md#integer) | [`uint32-wrap-max-plus-one.slang`](uint32-wrap-max-plus-one.slang) |
| `uint` wraps on underflow: `0 - 1 == UINT_MAX`. | boundary | [#integer](../../../../language-reference/types-fundamental.md#integer) | [`uint32-wrap-zero-minus-one.slang`](uint32-wrap-zero-minus-one.slang) |
| `int64_t` (64-bit signed) wraps on overflow: `INT64_MAX + 1 == INT64_MIN`. | boundary | [#integer](../../../../language-reference/types-fundamental.md#integer) | [`int64-wrap-max-plus-one.slang`](int64-wrap-max-plus-one.slang) |
| `uint64_t` (64-bit unsigned) wraps on overflow: `UINT64_MAX + 1 == 0`. | boundary | [#integer](../../../../language-reference/types-fundamental.md#integer) | [`uint64-wrap-max-plus-one.slang`](uint64-wrap-max-plus-one.slang) |
| `int` and `int32_t` are spec-level aliases — a value defined as one is assignable to and equal to the same value typed as the other. | functional | [#integer](../../../../language-reference/types-fundamental.md#integer) | [`int-int32-alias.slang`](int-int32-alias.slang) |
| `uint` and `uint32_t` are spec-level aliases. | functional | [#integer](../../../../language-reference/types-fundamental.md#integer) | [`uint-uint32-alias.slang`](uint-uint32-alias.slang) |


## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| Integer types are stored in memory with their natural size and alignment. | internal-source-fact | [#integer](../../../../language-reference/types-fundamental.md#integer) | Slang's `sizeof`/`alignof` surface is limited and target-dependent; verifying layout claims robustly requires reflection APIs that are not exposed at the slangc CLI in a portable way. |
| Target-conditional support for `int8_t`/`int16_t`/`uint8_t`/`uint16_t`. | gpu-other | [#integer](../../../../language-reference/types-fundamental.md#integer) | The doc notes "support for other types depends on the target and target capabilities". Per-target gating + capability tests deferred until the requires-tool / capability-gate infrastructure is settled. |
| Wrap-on-overflow on 8-/16-bit signed and unsigned integers. | gpu-other | [#integer](../../../../language-reference/types-fundamental.md#integer) | Same target-conditional concern: 8-/16-bit integer support is not universal across runners. The 32-/64-bit tests above exercise the same claim on the documented universally-supported types. |


## Doc gaps observed

NA
