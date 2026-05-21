---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T19:00:00Z
source_commit: 2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
watched_paths_digest: 668c0693482182a9e432e769a62c17c6a73f88b00454fecfd038698b2462cd72
source_doc: docs/llm-generated/name-resolution/index.md
source_doc_digest: b0430d4ce5c1b754210dd984ba0aa37efa52cb522661212bd1c593e713d61d77
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for name-resolution/index

## Intent

Tests verify the **cross-cutting orientation claims** made by
[`docs/llm-generated/name-resolution/index.md`](../../../docs/llm-generated/name-resolution/index.md):
the subtree's product (a resolved `DeclRef`), the documented
four-phase flow (scope walk -> raw `LookupResult` -> visibility
filter -> overload resolution -> `DeclRef` + breadcrumbs), the
ordering of those phases, the note that phases interleave (with
shadowing-during-walk as the named example), and the downstream
coupling into AST-to-IR lowering where breadcrumbs become concrete
field accesses in emitted code.

The bundle is intentionally small (6 tests). The index doc is
mostly a set of pointers to peer pages; the per-phase details
belong to the four peer bundles under `tests-agentic/name-resolution/`,
and we route them there via `## Out of scope` rather than
duplicating.

Strategy: one positive test per cross-cutting composition that the
index doc itself asserts, using the lightest runner that makes the
composition observable.

## Claims enumerated

| Claim ID | Anchor                            | Claim (one line)                                                                                                            | Tests                                            |
| -------- | --------------------------------- | --------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------ |
| C-01     | #name-resolution                  | The subtree turns an identifier in source text into a resolved DeclRef whose value is observable.                           | [`end-to-end-resolution-positive.slang`](end-to-end-resolution-positive.slang)           |
| C-02     | #flow-diagram                     | The four documented phases compose to pick a single visible candidate from a same-name set.                                 | [`flow-phases-compose-positive.slang`](flow-phases-compose-positive.slang)             |
| C-03     | #flow-diagram                     | The visibility filter precedes overload resolution, so an inaccessible-but-more-specific overload does not win.             | [`visibility-filters-before-overload.slang`](visibility-filters-before-overload.slang)       |
| C-04     | #where-this-fits-in-the-pipeline  | The resolved DeclRef flows into AST-to-IR lowering; breadcrumb chains turn into concrete field accesses in emitted code.    | [`breadcrumb-flows-into-emitted-hlsl.slang`](breadcrumb-flows-into-emitted-hlsl.slang)       |
| C-05     | #flow-diagram                     | Phases interleave; shadowing is enforced during the scope walk, so an inner-scope same-name decl wins without an ambiguity. | [`phases-interleave-shadowing-during-walk.slang`](phases-interleave-shadowing-during-walk.slang)  |
| C-06     | #name-resolution                  | The product of resolution is a DeclRef (not merely a Decl); a generic call's substitution is carried at the use site.       | [`declref-product-of-resolution.slang`](declref-product-of-resolution.slang)            |

## Tests in this bundle

| File                                            | Intent     | Doc anchor                          |
| ----------------------------------------------- | ---------- | ----------------------------------- |
| [`end-to-end-resolution-positive.slang`](end-to-end-resolution-positive.slang)          | functional | `#name-resolution`                  |
| [`flow-phases-compose-positive.slang`](flow-phases-compose-positive.slang)            | functional | `#flow-diagram`                     |
| [`visibility-filters-before-overload.slang`](visibility-filters-before-overload.slang)      | functional | `#flow-diagram`                     |
| [`breadcrumb-flows-into-emitted-hlsl.slang`](breadcrumb-flows-into-emitted-hlsl.slang)      | functional | `#where-this-fits-in-the-pipeline`  |
| [`phases-interleave-shadowing-during-walk.slang`](phases-interleave-shadowing-during-walk.slang) | functional | `#flow-diagram`                     |
| [`declref-product-of-resolution.slang`](declref-product-of-resolution.slang)           | functional | `#name-resolution`                  |

## Out of scope

The index doc explicitly delegates these to peer pages; tests for
them belong to peer bundles, not here:

- `Scope` shape, sibling-chain construction, file-scope and
  namespace boundaries, generic-parameter scope -- see
  `tests-agentic/name-resolution/scopes/`.
- `LookupMask` filter, transparent-member injection, breadcrumb
  construction details, inheritance walk for member lookup,
  ambiguity / member-not-found diagnostics, container-level
  overload accumulation, override-wins-over-default -- see
  `tests-agentic/name-resolution/lookup/`.
- `public` / `internal` / `private` semantics, accessor
  visibility, default-visibility rules -- see
  `tests-agentic/name-resolution/visibility/`.
- Arity / convertibility filtering, conversion-cost ranking,
  partial generic application, ambiguous-call diagnostics -- see
  `tests-agentic/name-resolution/overload-resolution/`.

## Sibling-bundle overlap

The following peer-bundle behaviors are intentionally not
re-tested here to avoid duplication:

- `unqualified-walks-parent-chain.slang` (lookup bundle) already
  covers the scope-walk-to-parent observation in isolation. The
  index bundle's `end-to-end-resolution-positive.slang` uses the
  same primitive but pins the cross-cutting claim that the result
  is a resolved DeclRef value observable at runtime.
- `block-scope-shadowing.slang` (scopes bundle) covers shadowing
  as a scope-boundary property. The index bundle's
  `phases-interleave-shadowing-during-walk.slang` cites the index
  doc's claim that shadowing is enforced DURING the walk (the
  interleaving observation), not the scope-boundary observation.
- `overload-prefers-better-match.slang` and similar in the
  overload-resolution bundle cover overload ranking on its own.
  The index bundle's `visibility-filters-before-overload.slang`
  exercises the composition of phase ordering, not the ranking
  itself.

## Out of scope (no-GPU runner)

(none) -- all tests run on the interpreter or compile to text
emit without a GPU.

## Doc gaps observed

- The index doc names `TryCheckOverloadCandidateVisibility` as the
  reason phases interleave (visibility is re-checked inside
  overload resolution), but does not give a user-observable
  consequence distinct from "the visibility diagnostic still
  fires". A direct test of this would have nothing to observe
  beyond what `visibility/` and `overload-resolution/` already
  cover; the index doc could either drop the parenthetical or
  add a sentence describing what a developer sees differently.
- The `#pages` and `#related-glossary-terms` sections are pure
  pointer tables; they are explicitly out of scope per the
  per-section prompt. No gap, just noted for future readers.
- The mermaid `#flow-diagram` shows `breadcrumbs` as part of the
  final output but the body text does not state where the
  breadcrumb chain is first attached. The index doc could
  clarify whether breadcrumbs are produced inside lookup, during
  the visibility/overload phases, or assembled at the very end --
  the answer would scope a future "where-do-breadcrumbs-come-from"
  test.
