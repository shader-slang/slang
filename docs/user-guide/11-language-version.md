# Language Version

Like many programming languages, Slang experiences a tension between the desire for rapid innovation/evolution and stability. One of the benefits that users of Slang have so far enjoyed has been the rapid pace of innovation in the language and its standard library. However, as developers start to have larger bodies of Slang code, they may become concerned that changes to the language could break existing code. There is no magical way to keep innovating while also keeping the language static.

Slang supports using the `#language` preprocessor directive, as well as the `-std` compiler option (`CompilerOptionName::LanguageVersion`) to specify the language version that a source file is written against. The source file will then be parsed and checked by the compiler using the rules from the specified language version.

Users are advised to provide a `#language` directive as the first non-whitespace line in their source file, such as:

```
#language slang 2026
```

The following version strings are allowed:
- `latest`: use the latest language version supported by the current compiler.
- `legacy`: use the legacy Slang language.
- `2018`: equivalent to `legacy`.
- `2025`: Slang language version 2025.
- `2026`: Slang language version 2026.

If no `#language` line exists and no version is specified via compiler options, the default setting is `legacy`.

## Deprecation and Removal Policy

This section describes how Slang deprecates and removes language features, standard-library declarations, and core-module APIs. The policy exists so that users can plan around breaking changes and so that contributors have a clear, low-surprise process when proposing one.

### Goal

Source compatibility is maintained per *declared* language version. A source file that opens with `#language slang <version>` compiles today; it should continue to compile on future Slang releases that still advertise support for `<version>`, modulo bug fixes that change behavior the compiler previously accepted incorrectly. Breaking changes are gated on a language-version bump; users opt in by choosing to upgrade their `#language` directive.

### When things get deprecated

A declaration may be deprecated when:

- A redesigned API supersedes it, and keeping both is a maintenance cost or a source of confusion.
- A language construct is found to be unsafe, ambiguous, or incompatible with a planned feature.
- A target backend drops support for the feature and the language cannot paper over the gap.
- A compliance or security requirement rules the feature out.

Cosmetic preference is **not** a valid reason to deprecate. Every deprecation should cite the specific problem it addresses in the PR description or a linked issue.

### Timeline

1. A declaration is marked `[deprecated(message)]` in a release. The compiler emits a diagnostic on every use; compilation still succeeds. The deprecation note lives in the release-notes entry for that version.
2. After **at least one full language version** has passed, the declaration may be marked `[RemovedSince(languageVersion, message)]` where `languageVersion` is the first version in which the declaration is unavailable. It remains available in earlier declared versions.
3. The declaration's definition may be removed from the codebase once the oldest language version still in public support is at or past `languageVersion`. Practically, we keep the stubbed declaration around longer than that.

Exceptions (shorter timelines) require maintainer approval on a per-change basis, are called out in the release notes, and should cite the reason (security, severe correctness bug, infeasible to stage).

### Concrete steps (authors)

When proposing a deprecation:

1. Add `[deprecated("...")]` to the declaration in `source/slang/core.meta.slang` (or the appropriate module file). Prefer a message that names the replacement.
2. If the deprecation schedules a removal, also add `[RemovedSince(<version>, "...")]` — see `@see` cross-links on these attributes in `core.meta.slang`.
3. Add a bullet to the appropriate section of this file (`docs/user-guide/11-language-version.md`) describing the change and linking the tracking issue.
4. For larger API removals, add a dedicated note under `docs/deprecated/` (see `docs/deprecated/a1-02-slangpy.md` for the established format) and link it from this file.
5. Add tests exercising both the deprecation diagnostic (for older versions) and the removal diagnostic (for the target version).
6. Label the PR `pr: breaking` if users selecting the target language version will see new compile errors on code that was previously accepted.

### Coverage: language features vs. core-module APIs

- **Core language features** (syntax, type-system rules, implicit conversions). The attributes above don't always apply directly; typically the deprecation lives in a diagnostic and the removal is gated on a language-version check in the checker. The bullet in this file and the tracking issue are still the source of truth.
- **Core-module declarations** (types, functions, attributes in `core.meta.slang` / `hlsl.meta.slang`). Use `[deprecated]` / `[RemovedSince]` directly.
- **SlangPy / compiler-driver features**. Follow the same phases; communicate in the project's release notes and relevant repository READMEs.

### Tooling

- `[deprecated(message)]` — attaches a diagnostic to every use of the declaration. See `source/slang/core.meta.slang`.
- `[RemovedSince(languageVersion, message)]` — hard error at the specified language version and later.
- `docs/user-guide/11-language-version.md` — changelog, per version.
- `docs/deprecated/` — long-form notes and migration guides.

Planned follow-up tooling is tracked in [#10701](https://github.com/shader-slang/slang/issues/10701) (`[BreakingChangeNote]` / `[AddedSince]`).

## The Legacy Slang Language

When the language version is set to `legacy`, the compiler behavior will be consistent with the Slang language as in 2018-2023. Specifically:

- All declarations have `public` visibility.
- A `module` declaration is not required at the start of each module.

## Slang 2025

Slang language version 2025 brings these changes on top of the legacy language:

- All declarations have `internal` visibility.
- A `module` declaration is required at the start of each module.
- Modifier `volatile` has been deprecated in Slang. See GitHub issue
  [#10614](https://github.com/shader-slang/slang/issues/10614) for details. The modifier is still accepted
  in GLSL source code for compatibility reasons.

## Slang 2026

Slang language version 2026 brings these changes on top of Slang 2025:

- Comma expression is removed when it is used inside a parenthesis. The expression `(a, b)` no longer evaluates to have `b`'s type as in C/C++. Instead, `(a,b)` means `makeTuple(a,b)` and returns a tuple consisting of `a` and `b`. See [SP#027](https://github.com/shader-slang/spec/blob/main/proposals/027-tuple-syntax.md) for details.
- Users must explicitly opt in to enable dynamic dispatch with the `dyn` keyword. More rigorous validations are enabled to make sure dynamic dispatch is not triggered accidentally. See [SP#024](https://github.com/shader-slang/spec/blob/main/proposals/024-any-dyn-types.md) for details.
- Modifier `volatile` has been removed from Slang. See GitHub issue
  [#10614](https://github.com/shader-slang/slang/issues/10614) for details. The modifier is still accepted
  in GLSL source code for compatibility reasons.
- Interface-typed variables can no longer be default-initialized. See GitHub issue
  [#9191](https://github.com/shader-slang/slang/issues/9191) for details.
