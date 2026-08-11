---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:27:12Z
target_doc: target-pipelines/wgsl.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 12
actions:
  fixed: 11
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for target-pipelines/wgsl.md

## Summary

Eleven of the twelve gaps were fixed by editing the document; one was
deferred. No gap was escalated: in every `drift-from-source` case the
watched source agreed with the observation, so the document was
incomplete rather than the compiler wrong. The bulk of the work is two
new Phase D subsections (`### Emitted spellings` and
`### Texture, sampler, and texture-intrinsic spellings`) that pin the
literal, vector, matrix, array, texture and sampler spellings the
`WGSLSourceEmitter` produces and the `case wgsl:` intrinsic templates
in `hlsl.meta.slang`; the rest are targeted additions to four
`## Notable passes` sections and four pass-table Notes cells. Nothing
was rejected as bogus or out of scope.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 3889ad2105cd | fixed | `source/slang/slang-emit-wgsl.cpp:849` emits the `var` keyword, `:1082` the `i32(N)` literal; `source/slang/slang-emit-c-like.cpp:3702,3714` emit `if(` / `else`. Printed form confirmed by `docs/generated/tests/design/target-pipelines/wgsl/nested-branches-five-deep-phi.slang` (`CHECK: var v_{{[0-9]+}} : i32`) and `eliminate-phis-default-options.slang`. | added a minimal `if`/`else` example under `eliminatePhis with default options` showing the `var v_0 : i32;` declaration and the per-arm assignments |
| 777a56bb41c0 | fixed | `source/slang/hlsl.meta.slang:13946-13949` declares `NonUniformResourceIndex` with `[require(cpp_cuda_glsl_hlsl_spirv, nonuniformqualifier)]`; that alias is `cpp \| cuda \| glsl \| hlsl \| spirv` (`source/slang/slang-capabilities.capdef:330`), excluding `wgsl`. `E36107` text confirmed by `non-uniform-resource-index-rejected.slang`. The existing emit-time claim is still correct (`source/slang/slang-emit-c-like.cpp:2753-2756`). | added a paragraph stating the capability requirement, the `E36107` type-check rejection, and that the emit-time wrapper drop applies only to insts reaching WGSL by another route (`nonuniform(DescriptorHandle<T>)`, `hlsl.meta.slang:27817-27818`) |
| d27a9f53454e | fixed | `source/slang/slang-emit-wgsl.cpp:74-101` (`emitSwitchCaseSelectorsImpl`) writes `case ` + selectors, or the bare keyword `default `, then `:`. Printed form confirmed by `switch-without-default-gets-synthesized-default.slang` (`CHECK: case i32(0)` / `CHECK: default`). | added a before/after example to the `legalizeSwitch` bullet showing a `default:`-less Slang switch emitting `case i32(0):`, `case i32(1):` and a `default :` arm |
| da19e305367a | fixed | `source/slang/slang-emit-wgsl.cpp:1494-1525` emits `And` as `select(vecN<bool>(false), rhs, lhs)` and `Or` as `select(rhs, vecN<bool>(true), lhs)`. Printed forms confirmed by `vector-logical-and-becomes-select.slang` and `int-vector-logical-and-casts-operands-to-bool.slang`. | added the emitted `select(...)` forms for `And` and `Or` under `legalizeLogicalAndOr`, with the already-boolean and cast-from-integer operand cases |
| 0a4d2ff77760 | deferred | Could not confirm a Slang-level construct that reaches the arm: `source/slang/slang-ir-legalize-binary-operator.cpp:200-210` casts only `IRVectorType` operands and `:246-263` then asserts both operands are already `array<vector<bool,N>>`, so only a boolean matrix qualifies — but `source/slang/core.meta.slang:3724-3730` constrains `operator&&` to `T : ILogical` and the file declares no `matrix` conformance to `ILogical`, so a plain `bool2x2 && bool2x2` may not type-check at all. The emitted per-element shape has no bundle test and cannot be settled without running `slangc`, which is unavailable on this host (Linux x86-64 build, arm64 host). Follow-up: run the compiler on a boolean-matrix `&&`, or confirm the arm is unreachable and file it as dead code. A source-confirmed narrowing of the entry condition was added to the section in the meantime, but the two things the gap asks for remain unwritten. | — |
| ab792853eacb | fixed | `source/slang/slang-ir-lower-append-consume-structured-buffer.cpp:29-50` name-hints the two generated fields `elements` and `counter`; `:122-138` rewrites `Append` as `AtomicInc` on `counter[0]` (relaxed) then a store into `elements[oldCounter]`. Emitted spellings confirmed by `append-structured-buffer-lowers-to-counter-and-elements.slang` (`array<atomic<i32>>`, `array<u32>`, `atomicAdd`). | extended row 54's Notes with the two generated storage buffers and the `atomicAdd`-then-store rewrite, and stated that `AppendStructuredBuffer<T>` is not front-end-rejected on WGSL |
| 9b5730e82911 | fixed | `source/slang/slang-emit.cpp:654-690` shows `checkStaticAssert` raising `StaticAssertionFailure` / `StaticAssertionFailureWithoutMessage` / `StaticAssertionConditionNotConstant`; ids 41400 / 41401 / 41402 at `source/slang/slang-diagnostics.lua:5322-5341`, and `array-index-out-of-bounds` = 30029 at `:1295-1300`. `E30029` also confirmed by `array-index-out-of-bounds-rejected.slang`. | added `E41400` / `E41401` / `E41402` to row 72's Notes and `E30029` to row 24's Notes |
| 700f09296336 | fixed | `source/slang/slang-emit-wgsl.cpp:304-341` (`emitStructFieldAttributes`) writes `@align(N)` per field as GCD(field offset, struct alignment); wrapper naming and the `data : array<vector<T,R>, C>` field at `source/slang/slang-ir-lower-buffer-element-type.cpp:2778-2820` and `:437-450` (`getLayoutName`). Emitted names confirmed by `structured-buffer-of-matrix-wraps-storage.slang`, `constant-buffer-matrix-std140-wrapper.slang`, `matrix-storage-square-2x2.slang`, `vec3-padding-trailing-field.slang`, `constant-buffer-uniform-std140.slang`. | extended Phase C row 21's Notes with the `<Name>_std430` / `<Name>_std140` and `_MatrixStorage_<elem><R>x<C>_ColMajor<rule>` wrapper names, the `data` field shape, and the `@align(N)` field attributes |
| 0d94aa42f381 | fixed | `source/slang/slang-emit-wgsl.cpp:1053-1145` (integer literals), `:1163-1196` (float/half/NaN/infinity), `:1793` (`emitVectorTypeNameImpl`), `:253-266` and `:580-588` (matrix transpose spelling), `:617-632` (arrays), `:602-615` (pointers), `:716` (`atomic<T>`). Printed forms confirmed by `integer-literal-spelling.slang`, `float-positive-and-negative-zero.slang`, `large-array-1024-elements.slang`, `uint3-becomes-vec3-u32.slang`, `float-nan-via-helper.slang`, `float-vector-with-infinity.slang`. | added a `### Emitted spellings` subsection under Phase D with a Slang-to-WGSL spelling table and a two-line worked example |
| 794da7170ab0 | fixed | `source/slang/slang-emit-wgsl.cpp:633-714` builds the texture type name piecewise (`texture` / `texture_storage`, `_depth`, `_multisampled`, shape, `_array`); `:703-707` emits the scalar element type; `:393-430` (`getWgslImageFormat`) infers `r32float` / `rg32float` / `rgba32float`; `:590-600` emit `sampler` / `sampler_comparison`. Confirmed by `texture1d-emit.slang`, `texture2d-binding.slang`, `texture3d-emit.slang`, `texturecube-emit.slang`, `texture2darray-emit.slang`, `rwtexture2d-storage-emit.slang`. | added a Slang-type-to-WGSL-type table (texture family, sampler, storage-texture format inference and access mode) in the new `### Texture, sampler, and texture-intrinsic spellings` subsection |
| 3b04183a771b | fixed | `source/slang/hlsl.meta.slang` `case wgsl:` arms: `:2615` `textureSample`, `:3748` `textureSampleLevel`, `:2830` `textureSampleBias`, `:3503` `textureSampleGrad`, `:2978` / `:3009` compare variants, `:3952` / `:4181` gather, `:4677` `textureLoad`, `:5322-5334` `textureStore`; array-layer split at `:2606-2610`, `:4669` (`i32(...)`) and `:3945` (`u32(...)`); cube `static_assert`s at `:4662` and `:5303`. Confirmed by `texture2d-sample-emit.slang`, `texture2d-load-emit.slang`, `rwtexture2d-storage-emit.slang`, `texture2darray-emit.slang`. | added the intrinsic-to-WGSL-builtin table and the array-layer coordinate-split explanation to the same new Phase D subsection |
| 3fda9351de83 | fixed | `source/slang/slang-emit-wgsl.cpp:63-73` calls plain `getSink()->diagnose(...)`, not `diagnoseOnce`, so the report is per qualified value; the diagnostic is declared `warning("precise-qualifier-unsupported-on-target", 56005, ...)` at `source/slang/slang-diagnostics.lua:5700-5705`. `E56005` and the message prefix confirmed by `precise-qualifier-dropped-with-diagnostic.slang`. | added `E56005`, its message text, and the "once per `precise`-qualified value (plain `diagnose`, not `diagnoseOnce`)" note to the `precise` paragraph |
