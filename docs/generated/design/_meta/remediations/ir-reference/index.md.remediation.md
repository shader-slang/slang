---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T17:45:00+00:00
target_doc: ir-reference/index.md
review_report: ../../reviews/ir-reference/index.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/index.md

## Summary

One critical finding addressed by softening the "exactly one family
page" claim into a "most opcodes ... a small number play two roles"
formulation that matches the post-fix tree, calling out the
remaining intentional dual-listings, naming every abstract /
grouping-only parent that does not appear as a table row, and
bumping the per-page opcode counts and Lua-entry-root descriptions
to reflect the family-page edits made in this remediation cycle.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The reviewer's recommendation said to "fix family pages first, then update the index wording and counts to match exact coverage". The family-page remediations in this cycle removed the duplicate `key` / `indexedFieldKey` / `witness_table` / `witness_table_entry` / `interface_req_entry` / `thisTypeWitness` / `TypeEqualityWitness` rows from `generics-and-existentials.md`, moved `DiffTypeInfo` from `misc.md` to `differentiation.md`, removed nine grouping-only parent rows across the tree, and added the type-flow specialization, constexpr arithmetic, backward-autodiff temporaries, cooperative matrix/vector, fragment-shader interlock, system-opcode, tensor-runtime-helper, string/native-pointer, object/CUDA, and `Require*` clusters as new rows. The remaining intentional cross-listings (`struct`/`class`/`interface`, `param`, `global_var`) are explicitly called out. | Rewrote the intro paragraph; added an explicit "Most opcodes ... A small number ... Abstract / grouping-only Lua entries ..." paragraph naming every abstract parent; updated the Pages table to bump approximate counts (values 110→150, control-flow 20→25, generics 30→55, resources 80→85, differentiation 30→35, metadata 60→55, misc 50→55) and extended each Family-column description and Lua-entry-root cell to mention the new clusters; extended the "Within a family page" paragraph to enumerate the smaller grouping parents that no longer appear as rows. |
