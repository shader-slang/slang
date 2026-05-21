---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T12:30:00+00:00
source_commit: ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
watched_paths_digest: a79bc9266564f8d663b4a60ade42dd146b58decdeb8ab1715ae4cd41e01a9a09
source_doc: docs/llm-generated/cross-cutting/diagnostics.md
source_doc_digest: 35cfb9612e0af198f089e0c87a82055a4f43737c2865d74d83133722d9f18bda
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for cross-cutting/diagnostics

## Intent

Tests verify the diagnostic-system surface described in
[`docs/llm-generated/cross-cutting/diagnostics.md`](../../../docs/llm-generated/cross-cutting/diagnostics.md):
the severity names rendered to users, the integer ids attached to
every diagnostic, the parameter-interpolation rules for legacy and
rich diagnostics, the source-location decoding that produces
caret-aligned messages, and the user-facing suppression surface
(`#pragma warning(disable: <id>)`). Each test exercises one
documented claim of the diagnostic system, not the per-feature
condition that any particular diagnostic flags.

Most tests use `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` because the
diagnostic itself is the observable being verified. Suppression
tests use `//TEST:SIMPLE(filecheck=CHECK):-target hlsl` with
`CHECK-NOT:` patterns to assert the diagnostic is **absent**.

Diagnostics fire pre-codegen and are target-independent for every
claim in this bundle, so a single directive per file is sufficient;
no multi-backend matrix is needed.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                       | Claim (one line)                                                                                                                  | Tests                                                                       |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| C-01     | [#severity-levels](../../../docs/llm-generated/cross-cutting/diagnostics.md#severity-levels)                                                                 | `Severity::Error` renders as the user-facing word `error`.                                                                        | [`severity-name-error.slang`](severity-name-error.slang)                                                 |
| C-02     | [#severity-levels](../../../docs/llm-generated/cross-cutting/diagnostics.md#severity-levels)                                                                 | `Severity::Warning` renders as the user-facing word `warning`.                                                                    | [`severity-name-warning.slang`](severity-name-warning.slang)                                               |
| C-03     | [#severity-levels](../../../docs/llm-generated/cross-cutting/diagnostics.md#severity-levels)                                                                 | `Severity::Note` renders as the user-facing word `note`.                                                                          | [`severity-name-note.slang`](severity-name-note.slang)                                                  |
| C-04     | [#error-codes-and-the-name-field](../../../docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field)                                   | Each diagnostic has a unique integer id; undefined-identifier is E30015.                                                          | [`error-code-undefined-identifier.slang`](error-code-undefined-identifier.slang)                                     |
| C-05     | [#error-codes-and-the-name-field](../../../docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field)                                   | Distinct diagnostics have distinct codes; function-redefinition is E30201.                                                        | [`error-code-function-redefinition.slang`](error-code-function-redefinition.slang)                                    |
| C-06     | [#error-codes-and-the-name-field](../../../docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field)                                   | Warnings carry integer ids identically to errors; comma-operator warning is 41024.                                                | [`error-code-warning-comma-operator.slang`](error-code-warning-comma-operator.slang)                                   |
| C-07     | [#error-codes-and-the-name-field](../../../docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field)                                   | Preprocessor diagnostics carry integer ids in their own range; undefined-identifier-in-preprocessor-expression is 15205.          | [`preprocessor-warning-code.slang`](preprocessor-warning-code.slang)                                           |
| C-08     | [#error-codes-and-the-name-field](../../../docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field)                                   | Parser diagnostics carry integer ids in their own range (2xxxx); unexpected-token is E20001.                                      | [`parser-error-has-code.slang`](parser-error-has-code.slang)                                               |
| C-09     | [#error-codes-and-the-name-field](../../../docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field)                                   | Callers can suppress a diagnostic by integer id; `#pragma warning(disable: <id>)` is the user-facing surface for `overrideDiagnostic`. | [`pragma-warning-disable-suppresses.slang`](pragma-warning-disable-suppresses.slang)                                   |
| C-10     | [#diagnosticsink](../../../docs/llm-generated/cross-cutting/diagnostics.md#diagnosticsink)                                                                   | Front-end stages emit diagnostics through the sink by default (no override required).                                             | [`diagnostic-fires-without-suppression.slang`](diagnostic-fires-without-suppression.slang)                                |
| C-11     | [#source-locations-and-message-rendering](../../../docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering)                   | `SourceLoc` is decoded into a caret aligned to the offending token's column.                                                      | [`caret-points-at-offending-token.slang`](caret-points-at-offending-token.slang)                                     |
| C-12     | [#source-locations-and-message-rendering](../../../docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering)                   | The caret span covers the full token, not just the start column.                                                                  | [`source-span-multi-character.slang`](source-span-multi-character.slang)                                         |
| C-13     | [#source-locations-and-message-rendering](../../../docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering)                   | The decoded location includes the file path so consumers can navigate to the offending site.                                      | [`source-location-file-line-format.slang`](source-location-file-line-format.slang)                                    |
| C-14     | [#source-locations-and-message-rendering](../../../docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering)                   | Errors triggered inside a macro expansion still surface the underlying message at the use site.                                   | [`macro-expansion-stack-in-diagnostic.slang`](macro-expansion-stack-in-diagnostic.slang)                                 |
| C-15     | [#anatomy-of-a-legacy-diagnostic](../../../docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-legacy-diagnostic)                                   | `~param:Name` interpolation places the actual identifier into the rendered message.                                               | [`name-interpolation-in-message.slang`](name-interpolation-in-message.slang)                                       |
| C-16     | [#anatomy-of-a-legacy-diagnostic](../../../docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-legacy-diagnostic)                                   | A legacy diagnostic with a `note(...)` helper emits a primary span and a companion note.                                          | [`note-multi-span-rendering.slang`](note-multi-span-rendering.slang)                                           |
| C-17     | [#anatomy-of-a-legacy-diagnostic](../../../docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-legacy-diagnostic)                                   | A legacy warning with a note (macro-redefinition) renders the warning and the previous-definition note.                           | [`macro-redefinition-warning.slang`](macro-redefinition-warning.slang)                                          |
| C-18     | [#anatomy-of-a-rich-diagnostic](../../../docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-rich-diagnostic)                                       | `{type}` interpolation in a rich diagnostic places the actual type name (quoted) into the rendered message.                       | [`type-name-interpolation-in-message.slang`](type-name-interpolation-in-message.slang)                                  |

## Tests in this bundle

| File                                            | Intent     | Doc anchor                                  |
| ----------------------------------------------- | ---------- | ------------------------------------------- |
| [`caret-points-at-offending-token.slang`](caret-points-at-offending-token.slang)         | negative   | `#source-locations-and-message-rendering`   |
| [`diagnostic-fires-without-suppression.slang`](diagnostic-fires-without-suppression.slang)    | negative   | `#diagnosticsink`                           |
| [`error-code-function-redefinition.slang`](error-code-function-redefinition.slang)        | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-undefined-identifier.slang`](error-code-undefined-identifier.slang)         | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-warning-comma-operator.slang`](error-code-warning-comma-operator.slang)       | negative   | `#error-codes-and-the-name-field`           |
| [`macro-expansion-stack-in-diagnostic.slang`](macro-expansion-stack-in-diagnostic.slang)     | negative   | `#source-locations-and-message-rendering`   |
| [`macro-redefinition-warning.slang`](macro-redefinition-warning.slang)              | negative   | `#anatomy-of-a-legacy-diagnostic`           |
| [`name-interpolation-in-message.slang`](name-interpolation-in-message.slang)           | negative   | `#anatomy-of-a-legacy-diagnostic`           |
| [`note-multi-span-rendering.slang`](note-multi-span-rendering.slang)               | negative   | `#anatomy-of-a-legacy-diagnostic`           |
| [`parser-error-has-code.slang`](parser-error-has-code.slang)                   | negative   | `#error-codes-and-the-name-field`           |
| [`pragma-warning-disable-suppresses.slang`](pragma-warning-disable-suppresses.slang)       | functional | `#error-codes-and-the-name-field`           |
| [`preprocessor-warning-code.slang`](preprocessor-warning-code.slang)               | negative   | `#error-codes-and-the-name-field`           |
| [`severity-name-error.slang`](severity-name-error.slang)                     | negative   | `#severity-levels`                          |
| [`severity-name-note.slang`](severity-name-note.slang)                      | negative   | `#severity-levels`                          |
| [`severity-name-warning.slang`](severity-name-warning.slang)                   | negative   | `#severity-levels`                          |
| [`source-location-file-line-format.slang`](source-location-file-line-format.slang)        | negative   | `#source-locations-and-message-rendering`   |
| [`source-span-multi-character.slang`](source-span-multi-character.slang)             | negative   | `#source-locations-and-message-rendering`   |
| [`type-name-interpolation-in-message.slang`](type-name-interpolation-in-message.slang)      | negative   | `#anatomy-of-a-rich-diagnostic`             |
| [`warnings-as-errors-promotes-warning.slang`](warnings-as-errors-promotes-warning.slang)     | boundary   | `#diagnosticsink`                           |
| [`pragma-warning-pop-empty-warns.slang`](pragma-warning-pop-empty-warns.slang)          | boundary   | `#error-codes-and-the-name-field`           |
| [`pragma-warning-push-not-popped-warns.slang`](pragma-warning-push-not-popped-warns.slang)    | boundary   | `#error-codes-and-the-name-field`           |
| [`pragma-warning-unknown-specifier-warns.slang`](pragma-warning-unknown-specifier-warns.slang)  | boundary   | `#error-codes-and-the-name-field`           |
| [`pragma-warning-disable-multiple-codes.slang`](pragma-warning-disable-multiple-codes.slang)   | boundary   | `#error-codes-and-the-name-field`           |
| [`diagnostic-on-first-line.slang`](diagnostic-on-first-line.slang)                | boundary   | `#source-locations-and-message-rendering`   |
| [`diagnostic-on-final-source-line.slang`](diagnostic-on-final-source-line.slang)         | boundary   | `#source-locations-and-message-rendering`   |
| [`deeply-nested-error-still-rendered.slang`](deeply-nested-error-still-rendered.slang)      | stress     | `#source-locations-and-message-rendering`   |
| [`many-undefined-identifiers-stress.slang`](many-undefined-identifiers-stress.slang)       | stress     | `#diagnosticsink`                           |
| [`error-chain-with-companion-note.slang`](error-chain-with-companion-note.slang)         | boundary   | `#anatomy-of-a-legacy-diagnostic`           |
| [`error-code-break-outside-loop.slang`](error-code-break-outside-loop.slang)           | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-continue-outside-loop.slang`](error-code-continue-outside-loop.slang)        | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-divide-by-zero.slang`](error-code-divide-by-zero.slang)               | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-invalid-array-size.slang`](error-code-invalid-array-size.slang)           | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-assign-non-lvalue.slang`](error-code-assign-non-lvalue.slang)            | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-subscript-non-array.slang`](error-code-subscript-non-array.slang)          | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-call-operator-not-found.slang`](error-code-call-operator-not-found.slang)      | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-no-member-of-type.slang`](error-code-no-member-of-type.slang)            | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-redeclaration-conflicts.slang`](error-code-redeclaration-conflicts.slang)      | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-type-mismatch.slang`](error-code-type-mismatch.slang)                | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-expected-a-type.slang`](error-code-expected-a-type.slang)              | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-user-defined-error.slang`](error-code-user-defined-error.slang)           | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-user-defined-warning.slang`](error-code-user-defined-warning.slang)         | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-unknown-preprocessor-directive.slang`](error-code-unknown-preprocessor-directive.slang) | negative | `#error-codes-and-the-name-field`           |
| [`error-code-directive-without-if.slang`](error-code-directive-without-if.slang)         | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-end-of-file-in-conditional.slang`](error-code-end-of-file-in-conditional.slang)   | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-divide-by-zero-in-preprocessor.slang`](error-code-divide-by-zero-in-preprocessor.slang) | negative | `#error-codes-and-the-name-field`           |
| [`error-code-unexpected-eof.slang`](error-code-unexpected-eof.slang)               | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-cyclic-include.slang`](error-code-cyclic-include.slang)               | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-include-failed.slang`](error-code-include-failed.slang)               | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-return-needs-expression.slang`](error-code-return-needs-expression.slang)      | negative   | `#error-codes-and-the-name-field`           |
| [`error-code-invalid-type-void.slang`](error-code-invalid-type-void.slang)            | negative   | `#error-codes-and-the-name-field`           |

## Doc gaps observed

- The doc enumerates `Severity::Fatal` and `Severity::Internal` and
  states their rendered names (`fatal error`, `internal error`) but
  does not describe any user-reachable program that reliably
  triggers either severity. Both are typically driven by environment
  conditions (missing include file for fatal, ICE for internal) that
  are non-trivial to script as a regression. Suggest the doc either
  link to a representative example or add a short "how to trigger"
  note next to each severity.
- The doc mentions `overrideDiagnostic` and the `flag` group as
  suppression surfaces but does not say what the user-facing
  spelling of either is (e.g. `#pragma warning(disable: <name>)` vs
  `<flag>`). The integer-id form is implicitly common, but the
  string-name and flag-group forms are under-documented. Suggest the
  doc add one explicit example of each suppression form.
- The JSON-emitting writer described under
  `#source-locations-and-message-rendering` is mentioned but has no
  user-facing claim that is observable via `slangc` defaults. To
  test it we would need a doc claim about the command-line knob that
  selects the JSON writer; that is not currently in the doc.
- The `name` field of a diagnostic is referenced as a unique
  identifier but the doc does not state whether the name appears in
  any rendered output. Empirically, slangc renders `error[E30201]`
  but not the name; we cannot test the name's externally observable
  consequence until the doc states one.
- The doc describes per-source warning-state tracking
  (`SourceWarningStateTrackerBase`) for pragma overrides but does
  not state a claim about scope (e.g. "the pragma applies to the
  rest of the translation unit" vs "the pragma applies to the
  current `__include` only"). This is a real user-visible behavior
  worth a documented claim, but absent one we do not test it.
- The doc mentions that the Severity filter "can suppress,
  downgrade, or upgrade diagnostics by id" but does not name the
  user-facing surfaces for `upgrade` and `downgrade` (the
  `-warnings-as-errors <id>` and `-warnings-disable <id>` command
  line flags). The boundary test
  `warnings-as-errors-promotes-warning.slang` exercises the
  `-warnings-as-errors` surface; the doc could call out both
  command-line flags explicitly.
- The doc describes the `#pragma warning` overrides but does not
  enumerate the legal specifier set (`push`, `pop`, `disable`,
  `error`, `default`, `suppress`). The `pragma-warning-unknown-
  specifier-warns.slang` boundary test pins one rejected
  specifier; the doc could list the accepted set explicitly to
  pair with the rejection diagnostic.
- The doc claims macro-expanded locations are "rendered with their
  expansion stack" but does not describe what an
  end-of-file-during-conditional diagnostic chain looks like
  (primary + companion `see 'if' directive` note). The boundary
  test `error-code-end-of-file-in-conditional.slang` exercises
  this chain; the doc could add a small example of the multi-
  record rendering.

## Out of scope (no-GPU runner)

None for this bundle. Diagnostic-system behaviors are fully
observable on the CPU runner via `DIAGNOSTIC_TEST` and
`//TEST:SIMPLE` directives over text targets.
