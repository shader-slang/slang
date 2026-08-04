---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T12:00:00+00:00
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 6445f2640b2b6124540b80df9bc23261410a0419e7ff9f20571cfe7ad7f42416
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/check-decl

White-box characterization tests targeting under-exercised declaration-checking
branches in `source/slang/slang-check-decl.cpp` (~63% covered). These pin the
compiler's _current observed behaviour_ (not a spec). The design doc
`pipeline/03-semantic-check.md` is the manifest `source_doc`; the real white-box
target is named in each test's `//META: covers=` field.

## Intent

Drive diverse declaration forms and documented error/diagnostic paths through
`slangc`/`slangi` and pin the observed result. Error paths use `//DIAGNOSTIC_TEST`
(they validate locally even without FileCheck); clean codegen forms (property,
subscript, extension method) use `//TEST:SIMPLE -target hlsl` (FileCheck reports
`ignored` locally, CI validates); value-level positives (enum tag values, integer
generic value-parameter specialization) use `//TEST:INTERPRET` so the interpreter
validates them locally. Every diagnostic message and code below was copied
verbatim from a real `slangc` run at `source_commit`.

## Functional coverage

| Test                                                                                       | What it pins                                                                                                         | covers=                           |
| ------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------- | --------------------------------- |
| [`static-const-missing-initializer.slang`](static-const-missing-initializer.slang)         | A `static const` with no initializer is rejected with E31225 ("static const variable 'x' must have an initializer"). | source/slang/slang-check-decl.cpp |
| [`var-no-type-no-initializer.slang`](var-no-type-no-initializer.slang)                     | A `var` with neither explicit type nor initializer is rejected with E30620.                                          | source/slang/slang-check-decl.cpp |
| [`bitfield-non-integral-type.slang`](bitfield-non-integral-type.slang)                     | A `float` bit-field member is rejected with E31301 ("bit-field type (float) must be an integral type").              | source/slang/slang-check-decl.cpp |
| [`enum-tag-type-non-integral.slang`](enum-tag-type-non-integral.slang)                     | An enum with a `float` tag type is rejected with E32000 ("invalid tag type for 'enum': 'float'").                    | source/slang/slang-check-decl.cpp |
| [`unsized-member-not-last.slang`](unsized-member-not-last.slang)                           | An unsized member followed by a sized member is rejected with E30070.                                                | source/slang/slang-check-decl.cpp |
| [`generic-value-param-non-integer-type.slang`](generic-value-param-non-integer-type.slang) | A `let N : float` generic value parameter is rejected with E30624 (only integer/enum types allowed).                 | source/slang/slang-check-decl.cpp |
| [`extension-base-not-interface.slang`](extension-base-not-interface.slang)                 | An `extension A : B` where `B` is a struct is rejected with E30813.                                                  | source/slang/slang-check-decl.cpp |
| [`class-base-not-class-or-interface.slang`](class-base-not-class-or-interface.slang)       | A `class C : NotIface` base that is neither class nor interface is rejected with E30814.                             | source/slang/slang-check-decl.cpp |
| [`interface-base-not-interface.slang`](interface-base-not-interface.slang)                 | An `interface I : NotIface` non-interface base is rejected with E30810.                                              | source/slang/slang-check-decl.cpp |
| [`private-member-not-accessible.slang`](private-member-not-accessible.slang)               | A `private` struct member is not accessible from a free function even in the same file (E30600).                     | source/slang/slang-check-decl.cpp |
| [`property-get-accessor-lowers.slang`](property-get-accessor-lowers.slang)                 | A `property` with a `get` accessor type-checks and its read lowers to HLSL.                                          | source/slang/slang-check-decl.cpp |
| [`subscript-get-accessor-lowers.slang`](subscript-get-accessor-lowers.slang)               | A user `__subscript` get accessor type-checks and `a[i]` lowers to a generated accessor call in HLSL.                | source/slang/slang-check-decl.cpp |
| [`extension-method-call-lowers.slang`](extension-method-call-lowers.slang)                 | A method added via `extension` is found by member lookup and its call lowers to HLSL.                                | source/slang/slang-check-decl.cpp |
| [`generic-value-param-int-specializes.slang`](generic-value-param-int-specializes.slang)   | An integer generic value parameter is accepted and specializes a fixed-size array member (Vec<3>).                   | source/slang/slang-check-decl.cpp |
| [`enum-explicit-tag-values.slang`](enum-explicit-tag-values.slang)                         | An enum with explicit tag values type-checks; cases evaluate to the assigned integers.                               | source/slang/slang-check-decl.cpp |

## Unreachable gaps

| Gap                                                                                                                                   | Why not targeted                                                                                                                                                                                         |
| ------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `Diagnostics::InternalCompilerError` / `Unexpected` sites in slang-check-decl.cpp                                                     | Defensive aborts on states the front-end should never reach with well-formed AST; not driveable to a clean result. One genuinely reachable abort (enum tag-not-first) is filed as a finding, not a test. |
| Derivative-attribute resolution diagnostics (`CannotResolveOriginalFunctionForDerivative`, `CustomDerivativeSignatureMismatch`, etc.) | Reachable but autodiff-specific and already exercised by the dedicated `tests/autodiff/` suite; out of scope for the declaration-form/visibility hint area of this bundle.                               |
| COM-interface constraint diagnostics (`InterfaceInheritingComMustBeCom`, `StructCannotImplementComInterface`)                         | Reachable but a narrow host-interop sub-feature; deferred to keep this bundle focused on the named hint area (modifiers / visibility / generic / extension / enum / property / subscript).               |
| Capability-conflict diagnostics (`ConflictingCapabilityDueToStatement`, `UseOfUndeclaredCapability*`)                                 | Reachable only with target-capability setups; better placed in a capability-focused coverage bundle.                                                                                                     |

## Doc gaps observed

| Anchor                                                                                    | Kind                  | Gap                                                                                                                                                                                                                                                                                                                  | Suggested addition                                                                                                                                                                                                                               |
| ----------------------------------------------------------------------------------------- | --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [#failure-modes](../../../design/pipeline/03-semantic-check.md#failure-modes)             | drift-from-source     | The section states check-level recovery is "continue with a placeholder type so that one error does not cascade", but a malformed enum base list (interface before the integer tag) emits the expected E30821 and then aborts with an internal error E99997 ("Unexpected witness structure") rather than recovering. | Note that recovery is best-effort and that some decl-check error paths still cascade into an InternalError abort, or fix the abort so the documented recovery contract holds; tracked in finding `check-decl-enum-tag-not-first-internal-error`. |
| [#modifier-validation](../../../design/pipeline/03-semantic-check.md#modifier-validation) | undocumented-behavior | The doc describes modifier validation generically but does not state the accessibility scope of `private`: a `private` member is observed to be inaccessible from a free function in the _same source file_ (E30600), i.e. `private` is type-scoped, not file-scoped.                                                | Add a sentence (or a small table) under modifier validation naming the visibility scopes: `private` = declaring-type scope, `internal` = module scope, `public` = cross-module.                                                                  |
