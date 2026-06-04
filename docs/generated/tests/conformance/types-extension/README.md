---
generated: true
model: claude-opus-4-7
generated_at: 2026-06-01T17:30:00+00:00
source_commit: d25453d7f0b4867db4cb5eabf34fb6cd088cf596
watched_paths_digest: 987317307ec9d99d423ee75f8ab6c4e8e20fd551312cc8bcf8fa0479036efa42
source_doc: docs/language-reference/types-extension.md
source_doc_digest: 6035f3897673a5155e982266b6248994b5a9dc440e9c8a53624caf637fcb81ab
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for language-reference/types-extension

## Intent

Tests verify type-extension semantics claimed in the **language
reference** at
[`docs/language-reference/types-extension.md`](../../../../language-reference/types-extension.md):
adding member functions to structs, supplying interface
requirements through an extension, adding interface conformances
to existing types, and extending enumerations with static data and
member functions where `this` is the enum value.

## Functional coverage

| Claim                                                                                                                                                         | Intent     | Anchor                                                              | Tests                                                                                                    |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| A struct extension adds a member function callable on instances of the original struct (Example 1).                                                           | functional | [#struct](../../../../language-reference/types-extension.md#struct) | [`struct-extension-adds-member-fn.slang`](struct-extension-adds-member-fn.slang)                         |
| A struct that declares conformance to an interface but does not implement the requirements can have those requirements satisfied by an extension (Example 2). | functional | [#struct](../../../../language-reference/types-extension.md#struct) | [`struct-extension-supplies-interface-impl.slang`](struct-extension-supplies-interface-impl.slang)       |
| An extension can ADD a new interface conformance to a struct that did not originally declare it (Example 3).                                                  | boundary   | [#struct](../../../../language-reference/types-extension.md#struct) | [`struct-extension-adds-interface-conformance.slang`](struct-extension-adds-interface-conformance.slang) |
| An enum extension adds static data and static functions, callable via `EnumType.member` (Example 1, enum).                                                    | functional | [#enum](../../../../language-reference/types-extension.md#enum)     | [`enum-extension-static-members.slang`](enum-extension-static-members.slang)                             |
| An enum extension adds non-static member functions where `this` is the enumeration value (Example 1, enum).                                                   | boundary   | [#enum](../../../../language-reference/types-extension.md#enum)     | [`enum-extension-this-is-value.slang`](enum-extension-this-is-value.slang)                               |

## Untested claims

| Claim                                                                             | Reason         | Anchor                                                                    | Why untested                                                                                                  |
| --------------------------------------------------------------------------------- | -------------- | ------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| Generic extension declaration (with type parameters and where-clauses).           | (unclassified) | [#generic](../../../../language-reference/types-extension.md#generic)     | Worth a separate bundle once generics infrastructure is exercised.                                            |
| Interface types cannot be extended (Remark).                                      | (unclassified) | [#extension](../../../../language-reference/types-extension.md#extension) | Negative test; deferred.                                                                                      |
| Subscript-op-decl, property-decl, function-call-op-decl added through extensions. | out-of-bundle  | [#extension](../../../../language-reference/types-extension.md#extension) | Their existence as members of structs is in the struct bundle; testing the extension path duplicates surface. |

## Doc gaps observed

| Anchor                                                              | Kind                  | Gap                                                                                                                                                                                                                                                                                                                                                                      | Suggested addition                                                                            |
| ------------------------------------------------------------------- | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------- |
| [#struct](../../../../language-reference/types-extension.md#struct) | undocumented-behavior | The Warning notes "When an extension and the base type contain a member with the same signature, it is currently undefined which member takes effect." The undefined-behavior label is honest, but a future doc rev should specify the resolution rule (base wins / extension wins / diagnostic). Tracked at [#9660](https://github.com/shader-slang/slang/issues/9660). | Define the resolution rule and document it; or implement a diagnostic for the ambiguous case. |
