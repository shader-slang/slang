---
generated: true
model: claude-opus-4-7
generated_at: 2026-06-01T16:30:00+00:00
source_commit: d25453d7f0b4867db4cb5eabf34fb6cd088cf596
watched_paths_digest: 8f786351aede6fdc8aa98781be05cd1d961036b7cc0988efe4a6a4d42609265e
source_doc: docs/language-reference/types-enum.md
source_doc_digest: e19ed1fccf556504153f8dba6968221ac410fe7b67d4058007b45ad9c3710e6f
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for language-reference/types-enum

## Intent

Tests verify enumeration semantics claimed in the **language
reference** at
[`docs/language-reference/types-enum.md`](../../../../language-reference/types-enum.md):
default numbering, explicit-value-then-resume, `[Flags]` shift
semantics, shared enumerator values, and scoped access.

## Functional coverage

| Claim                                                                                                                | Intent     | Anchor                                                                   | Tests                                                        |
| -------------------------------------------------------------------------------------------------------------------- | ---------- | ------------------------------------------------------------------------ | ------------------------------------------------------------ |
| Implicit numbering: first enumerator is 0, each subsequent unspecified is previous+1.                                | functional | [#description](../../../../language-reference/types-enum.md#description) | [`implicit-numbering.slang`](implicit-numbering.slang)       |
| Explicit-then-resume: after an explicit `= V`, subsequent unspecified enumerators continue from V+1.                 | boundary   | [#description](../../../../language-reference/types-enum.md#description) | [`explicit-then-resume.slang`](explicit-then-resume.slang)   |
| `[Flags]` attribute: first enumerator is 1; subsequent unspecified are previous shifted left by 1 (1, 2, 4, 8, ...). | boundary   | [#description](../../../../language-reference/types-enum.md#description) | [`flags-shift-numbering.slang`](flags-shift-numbering.slang) |
| Multiple enumerators may share the same numeric value.                                                               | functional | [#description](../../../../language-reference/types-enum.md#description) | [`shared-values-allowed.slang`](shared-values-allowed.slang) |
| Enumerations are scoped: access via `EnumType.NAME`.                                                                 | functional | [#description](../../../../language-reference/types-enum.md#description) | [`scoped-access.slang`](scoped-access.slang)                 |

## Untested claims

| Claim                                                                       | Reason         | Anchor                                                                   | Why untested                                                                                                                                                                                     |
| --------------------------------------------------------------------------- | -------------- | ------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Underlying type must be `bool` or an integer type.                          | (unclassified) | [#description](../../../../language-reference/types-enum.md#description) | The negative test (e.g. `enum E : float { ... }`) is straightforward but requires confirming the diagnostic code; deferred.                                                                      |
| Default underlying type is `int` (vs explicit `: uint`, `: uint8_t`, etc.). | (unclassified) | [#description](../../../../language-reference/types-enum.md#description) | Observing the type without a sizeof/reflection surface is awkward; the overload-probe trick can test it but is brittle for enums (overload-resolution rules for enums vs ints differ). Deferred. |
| Extension via `extension EnumType { ... }`.                                 | out-of-bundle  | [#description](../../../../language-reference/types-enum.md#description) | Covered (or should be) in `language-reference/types-extension`.                                                                                                                                  |
| Unscoped enumerations, `enum class` distinction.                            | deprecated     | [#description](../../../../language-reference/types-enum.md#description) | Doc remarks mark these as headed for deprecation.                                                                                                                                                |

## Doc gaps observed

NA
