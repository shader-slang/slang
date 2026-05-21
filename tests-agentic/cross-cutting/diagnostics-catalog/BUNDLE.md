---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T08:00:00+00:00
source_commit: 2964da04de136705348240f2bd9affa5f3818226
watched_paths_digest: d978c673bd877c4c2dbdf69ee7ae59f91418131f0d86559e7a80b68c78f20ed3
source_doc: docs/llm-generated/cross-cutting/diagnostics.md
source_doc_digest: 35cfb9612e0af198f089e0c87a82055a4f43737c2865d74d83133722d9f18bda
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for cross-cutting/diagnostics-catalog

## Intent

This bundle is the systematic negative-test sweep called for by the
agentic-tests plan §15.3: one `DIAGNOSTIC_TEST` file per diagnostic
code that the Slang compiler can emit, anchored to the structured
catalogs in `source/slang/slang-diagnostics.lua` and the
`*-diagnostic-defs.h` headers in `source/compiler-core/`. Each test
drives the compiler from a minimum-reproduction input and verifies
that the target code (e.g. `E15405`) appears in slangc's diagnostic
stream. The bundle is split across five parallel agents, one per
uncovered-bucket file; this section corresponds to bucket 0
(codes 1..20102, 123 codes covering lexer, preprocessor, parser,
and driver/linkage diagnostics).

Tests use `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):`
because almost every reproduction also fires a follow-up parser or
checker diagnostic; `non-exhaustive` lets us pin just the target
catalog code.

## Catalog coverage

| Bucket | Codes in bucket | Tests added | Codes dropped |
| ------ | --------------- | ----------- | ------------- |
| 0      | 123             | 39          | 84            |
| 3      | 123             | 105         | 18            |

## Tests in this bundle

| File | Code | Severity | Pipeline stage |
| ---- | ---- | -------- | -------------- |
| `1-cannot-open-file.slang` | E1 | err | preprocess |
| `10000-illegal-character-hex.slang` | E10000 | error | lex |
| `10001-illegal-character-literal.slang` | E10001 | error | lex |
| `10002-octal-literal.slang` | E10002 | warning | lex |
| `10003-invalid-digit-for-base.slang` | E10003 | error | lex |
| `10004-end-of-file-in-literal.slang` | E10004 | error | lex |
| `10005-newline-in-literal.slang` | E10005 | error | lex |
| `10010-quote-cannot-be-delimiter.slang` | E10010 | error | lex |
| `10012-integer-literal-too-large-for-any-type.slang` | E10012 | error | lex |
| `15002-directive-after-else.slang` | E15002 | err | preprocess |
| `15100-expected-preprocessor-directive-name.slang` | E15100 | err | preprocess |
| `15102-expected-token-in-preprocessor-directive.slang` | E15102 | err | preprocess |
| `15103-unexpected-tokens-after-directive.slang` | E15103 | err | preprocess |
| `15104-expected-2-tokens-in-preprocessor-directive.slang` | E15104 | err | preprocess |
| `15200-expected-token-in-preprocessor-expression.slang` | E15200 | err | preprocess |
| `15201-syntax-error-in-preprocessor-expression.slang` | E15201 | err | preprocess |
| `15203-expected-token-in-defined-expression.slang` | E15203 | err | preprocess |
| `15204-directive-expects-expression.slang` | E15204 | warning | preprocess |
| `15206-expected-integral-version-number.slang` | E15206 | err | preprocess |
| `15207-unknown-language-version.slang` | E15207 | err | preprocess |
| `15208-unknown-language.slang` | E15208 | err | preprocess |
| `15403-expected-token-in-macro-parameters.slang` | E15403 | err | preprocess |
| `15404-builtin-macro-redefinition.slang` | E15404 | warning | preprocess |
| `15405-token-paste-at-start.slang` | E15405 | err | preprocess |
| `15406-token-paste-at-end.slang` | E15406 | err | preprocess |
| `15407-expected-macro-parameter-after-stringize.slang` | E15407 | err | preprocess |
| `15408-duplicate-macro-parameter-name.slang` | E15408 | err | preprocess |
| `15409-variadic-macro-parameter-must-be-last.slang` | E15409 | err | preprocess |
| `15500-expected-token-in-macro-arguments.slang` | E15500 | warning | preprocess |
| `15501-wrong-number-of-arguments-to-macro.slang` | E15501 | err | preprocess |
| `15503-invalid-token-paste-result.slang` | E15503 | warning | preprocess |
| `15600-expected-pragma-directive-name.slang` | E15600 | err | preprocess |
| `15601-unknown-pragma-directive-ignored.slang` | E15601 | warning | preprocess |
| `15614-pragma-warning-suppress-cannot-identify-next-line.slang` | E15614 | warning | preprocess |
| `20017-const-not-allowed-on-c-style-ptr-decl.slang` | E20017 | err | parse |
| `20018-const-not-allowed-on-type.slang` | E20018 | err | parse |
| `20019-volatile-not-allowed-on-type.slang` | E20019 | err | parse |
| `20101-unintended-empty-statement.slang` | E20101 | warning | parse |
| `20102-unexpected-body-after-semicolon.slang` | E20102 | err | parse |
| `31225-static-const-variable-requires-initializer.slang` | E31225 | err | check |
| `31226-static-const-global-non-constant-init.slang` | E31226 | err | check |
| `31230-removed-modifier-usage.slang` | E31230 | err | parse |
| `31231-deprecated-modifier-usage.slang` | W31231 | warning | parse |
| `31400-decl-not-allowed-in-context.slang` | E31400 | err | check |
| `32000-invalid-enum-tag-type.slang` | E32000 | err | check |
| `32003-unexpected-enum-tag-expr.slang` | E32003 | err | check |
| `32006-enum-case-implicit-tag-value-overflow.slang` | W32006 | warning | check |
| `33070-expected-function.slang` | E33070 | err | check |
| `33072-cannot-have-generic-dyn-interface.slang` | E33072 | err | check |
| `33073-cannot-have-associated-type-in-dyn-interface.slang` | E33073 | err | check |
| `33074-cannot-have-generic-method-in-dyn-interface.slang` | E33074 | err | check |
| `33075-cannot-have-mutating-method-in-dyn-interface.slang` | E33075 | err | check |
| `33076-cannot-have-differentiable-method-in-dyn-interface.slang` | E33076 | err | check |
| `33077-dyn-interface-cannot-inherit-non-dyn-interface.slang` | E33077 | err | check |
| `33078-cannot-use-extension-to-make-type-conform-to-dyn-interface.slang` | E33078 | err | check |
| `33079-cannot-have-unsized-member-when-inheriting-dyn-interface.slang` | E33079 | err | check |
| `33080-cannot-have-opaque-member-when-inheriting-dyn-interface.slang` | E33080 | err | check |
| `33081-cannot-have-non-copyable-member-when-inheriting-dyn-interface.slang` | E33081 | err | check |
| `33082-cannot-conform-generic-to-dyn-interface.slang` | E33082 | err | check |
| `33180-cannot-specialize-generic-with-existential.slang` | E33180 | err | ir-pass |
| `36005-invalid-visibility-modifier-on-type-of-decl.slang` | E36005 | err | check |
| `36100-conflicting-capability-due-to-use-of-decl.slang` | E36100 | err | check |
| `36101-conflicting-capability-due-to-statement.slang` | E36101 | err | check |
| `36104-use-of-undeclared-capability.slang` | E36104 | err | check |
| `36105-unknown-capability.slang` | E36105 | err | check |
| `36106-expect-capability.slang` | E36106 | err | check |
| `36108-decl-has-dependencies-not-compatible-on-target.slang` | E36108 | err | check |
| `36109-invalid-target-switch-case.slang` | E36109 | err | check |
| `36110-use-of-undeclared-capability-of-interface-requirement.slang` | E36110 | err | check |
| `36112-entry-point-and-profile-are-incompatible.slang` | W36112 | warning | check |
| `36113-using-internal-capability-name.slang` | W36113 | warning | check |
| `36114-use-of-undeclared-capability-of-inheritance-decl.slang` | E36114 | err | check |
| `36116-capability-has-multiple-stages.slang` | E36116 | err | check |
| `36117-decl-has-dependencies-not-compatible-on-stage.slang` | E36117 | err | check |
| `36118-sub-type-has-subset-of-abstract-atoms-to-super-type.slang` | E36118 | err | check |
| `36119-requirment-has-subset-of-abstract-atoms-to-implementation.slang` | E36119 | err | check |
| `36120-target-switch-cap-cases-conflict.slang` | E36120 | err | check |
| `38000-entry-point-function-not-found.slang` | E38000 | err | check |
| `38006-specified-stage-doesnt-match-attribute.slang` | W38006 | warning | check |
| `38010-unhandled-mod-on-entry-point-parameter.slang` | W38010 | warning | check |
| `38011-entry-point-cannot-return-resource-type.slang` | E38011 | err | check |
| `38012-entry-point-cannot-return-array-type.slang` | E38012 | err | check |
| `38024-invalid-dispatch-thread-id-type.slang` | E38024 | err | check |
| `38029-type-argument-does-not-conform-to-interface.slang` | E38029 | err | check |
| `38031-invalid-use-of-no-diff.slang` | E38031 | err | check |
| `38033-cannot-use-no-diff-in-non-differentiable-func.slang` | E38033 | err | check |
| `38034-cannot-use-borrow-in-on-differentiable-parameter.slang` | E38034 | err | check |
| `38036-cannot-use-constref-on-differentiable-member-method.slang` | E38036 | err | check |
| `38040-non-uniform-entry-point-parameter-treated-as-uniform.slang` | W38040 | warning | check |
| `38041-int-val-from-non-int-spec-const-encountered.slang` | E38041 | err | check |
| `38042-implicit-type-coerce-constraint-with-non-implicit-conversion.slang` | E38042 | err | check |
| `38043-type-coerce-constraint-missing-conversion.slang` | E38043 | err | check |
| `38045-geometry-shader-missing-output-stream.slang` | E38045 | err | check |
| `38046-geometry-shader-missing-max-vertex-count.slang` | E38046 | err | check |
| `38047-mesh-shader-missing-output-topology.slang` | E38047 | err | check |
| `38048-mesh-shader-missing-outputs.slang` | E38048 | err | check |
| `38050-invalid-entry-point-varying-type.slang` | E38050 | err | check |
| `38051-invalid-entry-point-varying-type-for-target.slang` | E38051 | err | check |
| `38052-vertex-shader-missing-sv-position.slang` | W38052 | warning | check |
| `38100-type-doesnt-implement-interface-requirement.slang` | E38100 | err | check |
| `38101-this-expression-outside-of-type-decl.slang` | E38101 | err | check |
| `38103-this-type-outside-of-type-decl.slang` | E38103 | err | check |
| `38104-return-val-not-available.slang` | E38104 | err | check |
| `38105-member-does-not-match-requirement-signature.slang` | E38105 | err | check |
| `38106-member-return-type-mismatch.slang` | E38106 | err | check |
| `38107-generic-signature-does-not-match-requirement.slang` | E38107 | err | check |
| `38108-parameter-direction-does-not-match-requirement.slang` | E38108 | err | check |
| `38109-non-copyable-type-cannot-conform-to-interface.slang` | W38109 | warning | check |
| `38201-glsl-module-not-available.slang` | E38201 | err | link |
| `38203-vector-with-disallowed-element-type-encountered.slang` | E38203 | err | ir-pass |
| `38204-cannot-use-resource-type-in-structured-buffer.slang` | E38204 | err | ir-pass |
| `38205-recursive-types-found-in-structured-buffer.slang` | E38205 | err | check |
| `38206-vector-with-invalid-element-count-encountered.slang` | E38206 | err | ir-pass |
| `39001-parameter-bindings-overlap.slang` | W39001 | warning | check |
| `39007-unknown-register-class.slang` | E39007 | err | check |
| `39008-expected-a-register-index.slang` | E39008 | err | check |
| `39009-expected-space.slang` | E39009 | err | check |
| `39010-expected-space-index.slang` | E39010 | err | check |
| `39011-invalid-component-mask.slang` | E39011 | err | check |
| `39013-register-modifier-but-no-vulkan-layout.slang` | W39013 | warning | check |
| `39014-unexpected-specifier-after-space.slang` | E39014 | err | check |
| `39015-whole-space-parameter-requires-zero-binding.slang` | E39015 | err | check |
| `39017-dont-expect-out-parameters-for-stage.slang` | E39017 | err | check |
| `39018-dont-expect-in-parameters-for-stage.slang` | E39018 | err | check |
| `39019-global-uniform-not-expected.slang` | W39019 | warning | check |
| `39020-too-many-shader-record-constant-buffers.slang` | E39020 | err | check |
| `39022-vk-index-without-vk-location.slang` | W39022 | warning | check |
| `39023-mixing-implicit-and-explicit-binding-for-varying-params.slang` | E39023 | err | check |
| `39026-matrix-layout-modifier-on-non-matrix-type.slang` | E39026 | err | check |
| `39027-get-attribute-at-vertex-must-refer-to-per-vertex-input.slang` | E39027 | err | check |
| `39028-not-valid-varying-parameter.slang` | E39028 | err | check |
| `39029-register-modifier-but-no-vk-binding-nor-shift.slang` | W39029 | warning | check |
| `39030-target-does-not-support-descriptor-handle.slang` | E39030 | err | ir-pass |
| `39071-binding-attribute-ignored-on-uniform.slang` | W39071 | warning | check |
| `40000-ray-payload-field-missing-access-qualifiers.slang` | E40000 | err | check |
| `40001-ray-payload-invalid-stage-in-access-qualifier.slang` | E40001 | err | check |
| `40004-integer-literal-too-large.slang` | W40004 | warning | lex |
| `40005-integer-literal-truncated.slang` | W40005 | warning | check |
| `40006-unimplemented-system-value-semantic.slang` | E40006 | err | check |
| `40008-invalid-l-value-for-ref-parameter.slang` | E40008 | err | ir-pass |
| `40009-float-literal-unrepresentable.slang` | W40009 | warning | check |
| `40010-float-literal-too-small.slang` | W40010 | warning | check |
| `40011-overload-candidate.slang` | N40011 | note | check |
| `40012-need-compile-time-constant.slang` | E40012 | err | check |

## Codes dropped (could not reach from minimum input)

| Code | Name | Reason |
| ---- | ---- | ------ |
| E2 | cannot-find-file | Driver-only diagnostic (file-system search across include paths); not emitted from .slang content. |
| E3 | token-type-expected-but-eof | Lexer EOF-token diagnostic; superseded by parser E20001 in every observed trigger. |
| E4 | cannot-write-output-file | Driver-only diagnostic (output-file write failure); requires controlled file-system state. |
| E5 | failed-to-load-dynamic-library | Driver-only diagnostic (dynamic library load); not emitted from .slang content. |
| E6 | too-many-output-paths-specified | CLI option diagnostic; not emitted from .slang content. |
| E12 | cannot-deduce-source-language | CLI option diagnostic; not emitted from .slang content. |
| E15 | unknown-stage | CLI option diagnostic (`-stage`); not emitted from .slang content. |
| E16 | unknown-pass-through-target | CLI option diagnostic (`-pass-through`); not emitted from .slang content. |
| E17 | unknown-command-line-option | CLI option diagnostic; not emitted from .slang content. |
| E18 | separate-debug-info-unsupported-for-target | CLI option diagnostic (`-separate-debug-info`); not emitted from .slang content. |
| E19 | unknown-source-language | CLI option diagnostic (`-lang`); not emitted from .slang content. |
| E20 | entry-points-need-to-be-associated-with-translation-units | CLI option diagnostic (`-entry`); not emitted from .slang content. |
| E22 | unknown-downstream-compiler | CLI option diagnostic (`-downstream`); not emitted from .slang content. |
| E28 | unable-to-generate-code-for-target | CLI option diagnostic (`-target`); not emitted from .slang content. |
| E30 | same-stage-specified-more-than-once | CLI option diagnostic (`-stage`); not emitted from .slang content. |
| E32 | explicit-stage-doesnt-match-implied-stage | CLI option diagnostic (`-stage`); not emitted from .slang content. |
| E33 | stage-specification-ignored-because-no-entry-points | CLI option diagnostic (`-stage`); not emitted from .slang content. |
| E34 | stage-specification-ignored-because-before-all-entry-points | CLI option diagnostic (`-stage`); not emitted from .slang content. |
| E35 | no-stage-specified-in-pass-through-mode | CLI option diagnostic (`-pass-through`); not emitted from .slang content. |
| E36 | expecting-an-integer | CLI option diagnostic; not emitted from .slang content. |
| E37 | expecting-a-unsigned-integer | CLI option diagnostic; not emitted from .slang content. |
| E40 | same-profile-specified-more-than-once | CLI option diagnostic (`-profile`); not emitted from .slang content. |
| E41 | conflicting-profiles-specified-for-target | CLI option diagnostic (`-profile`); not emitted from .slang content. |
| E42 | profile-specification-ignored-because-no-targets | CLI option diagnostic (`-profile`); not emitted from .slang content. |
| E43 | profile-specification-ignored-because-before-all-targets | CLI option diagnostic (`-profile`); not emitted from .slang content. |
| E44 | target-flags-ignored-because-no-targets | CLI option diagnostic (target flags); not emitted from .slang content. |
| E45 | target-flags-ignored-because-before-all-targets | CLI option diagnostic (target flags); not emitted from .slang content. |
| E50 | duplicate-targets | CLI option diagnostic (`-target`); not emitted from .slang content. |
| E51 | unhandled-language-for-source-embedding | Internal driver state (source-embedding); not emitted from .slang content. |
| E60 | cannot-deduce-output-format-from-path | CLI option diagnostic (`-o`); not emitted from .slang content. |
| E61 | cannot-match-output-file-to-target | CLI option diagnostic (`-o`); not emitted from .slang content. |
| E62 | unknown-command-line-value | CLI option diagnostic; not emitted from .slang content. |
| E63 | unknown-help-category | CLI option diagnostic (`-h`); not emitted from .slang content. |
| E70 | cannot-match-output-file-to-entry-point | CLI option diagnostic (`-o`); not emitted from .slang content. |
| E71 | invalid-type-conformance-option-string | CLI option diagnostic (`-type-conformance`); not emitted from .slang content. |
| E72 | invalid-type-conformance-option-no-type | CLI option diagnostic (`-type-conformance`); not emitted from .slang content. |
| E73 | cannot-create-type-conformance | CLI option diagnostic (`-type-conformance`); not emitted from .slang content. |
| E80 | duplicate-output-paths-for-entry-point-and-target | CLI option diagnostic (`-o`); not emitted from .slang content. |
| E81 | duplicate-output-paths-for-target | CLI option diagnostic (`-o`); not emitted from .slang content. |
| E82 | duplicate-dependency-output-paths | CLI option diagnostic (`-dep`); not emitted from .slang content. |
| E83 | unable-to-write-repro-file | CLI option diagnostic (`-repro`); not emitted from .slang content. |
| E86 | unable-to-create-module-container | Internal API diagnostic (module container); not emitted from .slang content. |
| E87 | unable-to-set-default-downstream-compiler | CLI/API diagnostic (`-default-downstream-compiler`); not emitted from .slang content. |
| E89 | expecting-slang-riff-container | Driver/API diagnostic (RIFF container); not emitted from .slang content. |
| E90 | incompatible-riff-semantic-version | Driver/API diagnostic (RIFF version); not emitted from .slang content. |
| E91 | riff-hash-mismatch | Driver/API diagnostic (RIFF hash); not emitted from .slang content. |
| E92 | unable-to-create-directory | Driver/API diagnostic (directory creation); not emitted from .slang content. |
| E93 | unable-to-extract-repro-to-directory | Driver/API diagnostic (repro extraction); not emitted from .slang content. |
| E94 | unable-to-read-riff | Driver/API diagnostic (RIFF read); not emitted from .slang content. |
| E95 | unknown-library-kind | Driver/API diagnostic (`-r` library kind); not emitted from .slang content. |
| E96 | kind-not-linkable | Driver/API diagnostic (library kind); not emitted from .slang content. |
| E97 | library-does-not-exist | CLI option diagnostic (library path); not emitted from .slang content. |
| E98 | cannot-access-as-blob | Internal API diagnostic (blob access); not emitted from .slang content. |
| E100 | failed-to-load-downstream-compiler | Driver-only diagnostic (downstream compiler load); not emitted from .slang content. |
| E101 | downstream-compiler-doesnt-support-whole-program-compilation | Driver-only diagnostic (whole-program); not emitted from .slang content. |
| E102 | downstream-compile-time | Driver-only timing note; not emitted from .slang content. |
| E103 | performance-benchmark-result | Driver-only benchmark note; not emitted from .slang content. |
| E104 | need-to-enable-experiment-feature | CLI option diagnostic (`-experimental-feature`); not emitted from .slang content. |
| E105 | null-component-type | API diagnostic (component types); not emitted from .slang content. |
| E10011 | unexpectedEndOfInput | Lexer EOF diagnostic; in practice the lexer falls back to E10000 or parser sees the EOF first. |
| E15209 | language-version-differs-from-including-module | Multi-file include diagnostic (cross-module language version); requires a second source file. |
| E15301 | import-failed | AST deserialization diagnostic (precompiled module); not reachable from a single .slang. |
| E15303 | cannot-resolve-imported-decl | Precompiled-module decl-resolution diagnostic; requires a second compiled module artifact. |
| E15304 | no-unique-identity | Internal preprocessor/file-system diagnostic (no unique identity); not reachable from a single .slang. |
| E15502 | error-parsing-to-macro-invocation-argument | Macro-argument inner parse error; observed compiler output instead surfaces E15500 (warning). |
| E15602 | pragma-once-ignored | Catalog entry has no current emission site in the compiler source (apparent dead diagnostic). |
| E15615 | pragma-warning-cannot-insert-here | Catalog entry has no current emission site in the compiler source (apparent dead diagnostic). |
| E15616 | pragma-warning-point-suppress | Catalog entry has no current emission site in the compiler source (apparent dead diagnostic). |
| E20000 | unexpectedCharacter | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20002 | newlineInLiteral | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20003 | endOfFileInComment | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20004 | expectingAHexDigit | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20005 | expectingADigit | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20006 | expectingValueName | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20007 | unexpectedTokenExpectedTokenType | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20008 | unexpectedToken | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20009 | unableToConvertField | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20010 | fieldNotFound | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20011 | fieldRequiredOnType | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20012 | tooManyElementsForArray | JSON-parser diagnostic; emitted by the JSON lexer, not the Slang source lexer. |
| E20013 | invalid-cuda-sm-version | CLI option diagnostic (`-profile sm_*`); not emitted from .slang content. |
| E20014 | type-name-expected-but-eof | Parser EOF diagnostic for type-name; in observed reproductions the parser emits E20001 first. |
| E20015 | unexpected-eof | Parser EOF diagnostic; in observed reproductions the parser emits E20001 first. |
| E20016 | missing-layout-binding-modifier | Catalog entry has no current emission site in the compiler source (apparent dead diagnostic). |
| E36102 | conflicting-capability-due-to-statement-enclosing-func | Internal capability-conflict path with no context decl; in observed reproductions the compiler emits E36101 (decl context) first. |
| E38005 | expected-type-for-specialization-arg | Requires API-driven specialization argument (`SpecializationArg`); not reachable from a single .slang via slangc CLI. |
| E38008 | specialization-parameter-of-name-not-specialized | A `type_param` without specialization crashes the compiler on the CLI path; do not synthesize. |
| E38009 | expected-value-of-type-for-specialization-arg | Requires API-driven specialization of a `GlobalGenericValueParamDecl`; not reachable from slangc CLI. |
| E38013 | specialization-parameter-not-specialized | Requires API-driven specialization; not reachable from a single .slang via slangc CLI. |
| E38021 | type-argument-for-generic-parameter-does-not-conform-to-interface | Fires only when `-specialize` provides a non-conforming type for a global `type_param`; not reachable from a self-contained .slang. |
| E38022 | cannot-specialize-global-generic-to-itself | Requires API-driven specialization of a global type_param to itself; not reachable from slangc CLI. |
| E38023 | cannot-specialize-global-generic-to-another-generic-param | Requires API-driven specialization across global type_params; not reachable from slangc CLI. |
| E38025 | mismatch-specialization-arguments | Requires API-driven specialization argument count mismatch; not reachable from slangc CLI. |
| E38028 | invalid-form-of-specialization-arg | Requires API-driven specialization with malformed argument value; not reachable from slangc CLI. |
| E38032 | use-of-no-diff-on-differentiable-func | Catalog entry has no current emission site in the compiler source (apparent dead diagnostic). |
| E38035 | encountered-non-differentiable-function-during-higher-order-diff | Requires `fwd_diff` of a function that internally calls another non-differentiable function during higher-order pass; minimum repro not found in 2 attempts. |
| E38200 | recursive-module-import | Requires two `.slang` source files that import each other; not reachable from a single-file repro. |
| E39000 | conflicting-explicit-bindings-for-parameter | Fires when the same parameter has two conflicting `register`/`vk::binding` specs; requires multi-file or multi-decl conflict beyond minimum repro. |
| E39012 | requested-bindless-space-index-unavailable | Requires `-bindless-space-index` CLI option combined with a space already taken; CLI option side-effect, not user .slang content. |
| E39025 | conflicting-vulkan-inferred-binding-for-parameter | Requires two parameters whose Vulkan binding inference happens to overlap; minimum repro not found in 2 attempts. |
| E39031 | class-type-not-supported | Triggering this with `class X { int a; } X x;` segfaults the compiler in this build; do not lock in buggy behavior. |
| E39998 | generic-evaluation-recursion-limit-exceeded | Requires generic evaluation depth above `kMaxTypeNestingDepth`; needs synthesized deep generic chain beyond minimum repro. |

