# Prompt: name-resolution/visibility.md

See [_common.md](_common.md) for the universal rules and the
**Name-resolution family contract** that applies to this page.

## Target

Produce `docs/llm-generated/name-resolution/visibility.md` — the page
that documents declaration visibility rules: which `public` /
`internal` / `private` keyword each decl carries, what the per-language-
version defaults are, and where in the resolution pipeline visibility
is enforced.

Audience: a developer adding or modifying a visibility-related
diagnostic, a language designer reasoning about cross-module access, or
a contributor wondering why a declaration in another module is or is
not visible.

## Family-specific guidance

This page uses the `## Rules` heading (not `## Algorithm`). The body is
rule-based rather than pipeline-shaped.

The `## Concepts` section must cover, at minimum:

- The `VisibilityModifier` family declared in
  [slang-ast-modifier.h](../../../source/slang/slang-ast-modifier.h):
  `PublicModifier`, `PrivateModifier`, `InternalModifier`, and any
  other concrete subclass present at `source_commit`. Cite the
  declaration site for each.
- The `DeclVisibility` enum (or equivalent) used internally if present
  in `slang-ast-modifier.h` or `slang-ast-support-types.h`.
- `ModuleDecl::defaultVisibility` (cite the field in
  [slang-ast-decl.h](../../../source/slang/slang-ast-decl.h)).
- The `IgnoreForLookupModifier` (if present at `source_commit`) and
  its role in hiding a decl from lookup entirely.

The `## Rules` section must cover, in this order:

1. **Per-keyword semantics.** What each of `public`, `private`,
   `internal` means in plain prose:
   - `public`: visible outside the module.
   - `internal`: visible only inside the declaring module.
   - `private`: visible only inside the declaring container.
   Cite the visibility-classification helper in
   [slang-check-modifier.cpp](../../../source/slang/slang-check-modifier.cpp)
   or [slang-check-decl.cpp](../../../source/slang/slang-check-decl.cpp)
   that maps a `VisibilityModifier` to the internal enum.
2. **Defaults by language version.** Cite
   `SlangLanguageVersion` (or its actual identifier at `source_commit`)
   and the `ModuleDecl::defaultVisibility` field. The legacy version
   defaults to `public`; the modern version defaults to `internal`.
   Cite the assignment site in
   `slang-check-decl.cpp` (search for `defaultVisibility =`).
3. **Where visibility is filtered.** Visibility is filtered at two
   sites:
   - The lookup boundary, when a result from another module is being
     considered.
   - Overload resolution, via the `TryCheckOverloadCandidateVisibility`
     step in
     [slang-check-overload.cpp](../../../source/slang/slang-check-overload.cpp).
   Cite `isDeclVisibleFromScope` (or the actual helper at
   `source_commit`) declared / defined in
   `slang-check-decl.cpp` or `slang-check-modifier.cpp`.
4. **Container-level rules.** Visibility of a member is bounded by the
   visibility of its container; e.g. a `public` member of an
   `internal` struct is effectively `internal`. Cite the helper that
   computes the effective visibility.
5. **Interaction with `extern` and `export`.** Cover any
   `ExternModifier` / `ExportModifier` (or equivalent) handling that
   alters cross-module visibility, if present at `source_commit`.
6. **Generic parameters and synthesized members.** Visibility of
   generic parameters; visibility of synthesized accessors, default
   conformance impls, and other compiler-generated decls. Cite the
   synthesis site in
   [slang-check-decl.cpp](../../../source/slang/slang-check-decl.cpp).

Include a small mermaid flowchart showing how visibility filtering
interacts with lookup:

```
LookupResult -> for each item: check effective visibility against requesting scope -> drop or keep -> filtered LookupResult
```

The `## Edge cases and failure modes` section must cover:

- A `public` member inside an `internal` struct: effective visibility
  is `internal`.
- A name accessible by direct lookup but filtered out by visibility:
  what diagnostic is produced and where in
  [slang-diagnostics.h](../../../source/slang/slang-diagnostics.h) it
  lives.
- A `using`-decl that re-exports an `internal` decl from another
  module: how it is rejected.
- Synthesized members and how their visibility is decided.
- Language-version interaction: a module compiled as legacy that
  imports a module compiled as modern (or vice versa); which side's
  default wins.
- `IgnoreForLookupModifier` interaction: visibility is irrelevant when
  the decl is invisible to lookup in the first place.

## Forbidden content (in addition to the universal forbidden list and the family contract)

- Per-`Modifier`-subclass field documentation — link
  [../ast-reference/modifiers.md](../ast-reference/modifiers.md)
  instead.
- The lookup algorithm itself — that belongs in
  [lookup.md](lookup.md).
- Overload-resolution ranking — that belongs in
  [overload-resolution.md](overload-resolution.md). This page covers
  only the visibility-filter step, not what happens after.

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every named visibility modifier exists in
      [slang-ast-modifier.h](../../../source/slang/slang-ast-modifier.h)
      at `source_commit`.
- [ ] The default-visibility-by-language-version rule cites the
      actual assignment site.
- [ ] Every claim about the visibility filter cites a function in
      `slang-check-decl.cpp`, `slang-check-modifier.cpp`, or
      `slang-check-overload.cpp`.
