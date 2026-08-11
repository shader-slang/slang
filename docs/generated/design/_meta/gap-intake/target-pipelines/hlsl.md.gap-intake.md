---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:14:06Z
target_doc: target-pipelines/hlsl.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 18
actions:
  fixed: 15
  rejected_bogus: 1
  rejected_out_of_scope: 1
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for target-pipelines/hlsl.md

## Summary

Eighteen gaps, all from `docs/generated/tests/design/target-pipelines/hlsl`.
Nothing was escalated: no gap turned out to be a compiler defect, and every
observation that was confirmable agreed with the source. Fifteen are fixed,
one is rejected as bogus (the loops section already states the consequence the
gap asks for), one is out of scope (the `## Source` line numbers the gap wants
dropped are mandated by the target-pipeline contract), and one is deferred
because settling it needs a compiler run this host cannot do. The page now
stands at 82,772 bytes against its 98,304-byte cap, so the edits had room to
be explicit.

Two gaps anchored on `#phase-d-hlsl-emit-and-downstream-tools` were applied as
one consolidated edit, a new `### Shape of the emitted file` subsection, since
both ask what the emitted artifact actually contains. Several fixes say more
than the gap asked because the source says more: the `and(a, b)` / `or(a, b)`
answer for `legalizeLogicalAndOr` turns out to be selected by operand shape
*and* two profile predicates, not by the pass at all; the coverage row now
distinguishes the CLI bit-width path (`E45113`) from the API byte-width path
(`E45114`), which the gap conflated; and the `descriptor_handle` row records
that `_sm_6_6` is the only disjunct of that alias an HLSL target can satisfy.
One fix says less: gap `8db3a13d2d01` asked for two generated-name shapes, and
only `_S<N>` belongs to `wrapStructuredBuffersOfMatrices`.

Three fixes rest on files outside this page's `watched_paths` —
`slang-options.cpp` (CLI spellings), `slang-diagnostics.lua` (diagnostic
codes), and `slang-capabilities.capdef` (capability aliases). Each is a
declarative table read directly and cited by line, but the manifest should
gain those paths so the claims stay tracked. See the Actions rows for
`0a34a3865c53`, `456356a76d90`, `151da0a0a8be`, `dd82a7558698`, `ea3331936f79`
and `1b0d1d14d61c`.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| dd6630d4d2c0 | fixed | Both storable repairs insert at `entryBlock->getFirstOrdinaryInst()` in watched `source/slang/slang-ir-variable-scope-correction.cpp:116` (hoist at `:183`, spill at `:200-215`), and `kIROp_Var` is on the never-fold list in watched `source/slang/slang-emit-c-like.cpp:1462-1470`, so the var is emitted as a declaration statement at that position. No bundle test anchors this section, so the emitted shape is stated in prose rather than as fabricated verbatim text. | added a paragraph stating the repairs surface as a declaration lifted to the top of the function body, with no dedicated marker, and that the clone repair leaves no declaration at all |
| 1b0d1d14d61c | fixed | `alias descriptor_handle = glsl_spirv \| _sm_6_6 \| cpp \| cuda \| metal \| wgsl` at `source/slang/slang-capabilities.capdef:1474` — `_sm_6_6` is the only HLSL-reachable disjunct. The user surface `DescriptorHandle<T>` is declared in watched `source/slang/hlsl.meta.slang:27478` with `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]` at `:27480`. Gate itself at watched `source/slang/slang-emit.cpp:2729-2738`. **`slang-capabilities.capdef` is outside `watched_paths`.** | named `DescriptorHandle<T>` and the `_sm_6_6` disjunct in the `descriptor_handle` row of the capability-gates table |
| db73d48479a1 | fixed | `emitNamedMemoryTypeFlagSet` / `emitNamedSemanticFlagSet` in watched `source/slang/slang-emit-hlsl-prelude.cpp:553-580` and `:582-609` each emit `"("` before and `")"` after the flag spelling, independently of the shorthand-vs-joined branch; the call arms are at watched `source/slang/slang-emit-hlsl.cpp:1177-1195`. Both forms are the verified CHECK lines of `barrier-flags-named-constants-emit.slang:41,44`. | added the two verbatim `Barrier(...)` call forms as a fenced block in the barrier bullet, so the per-argument parenthesisation is documented |
| 056b53499498 | fixed | The form is chosen at emit, not by the pass: watched `source/slang/slang-emit-hlsl.cpp:1262-1288` returns `false` for `as<IRBasicType>` results (falling through to the infix `&&` / `\|\|` at watched `source/slang/slang-emit-c-like.cpp:2605-2616`) and otherwise writes `and(` / `or(`, but only when `m_effectiveProfile.getVersion() >= DX_6_0` and `isTargetHLSL2018`. Matrix case reaches emit as the `MakeArray` built at watched `source/slang/slang-ir-legalize-binary-operator.cpp:246-291`. | added a three-bullet list giving the emitted form per operand shape (scalar infix, vector `and()` / `or()` behind two profile predicates, lowered matrix per-row inside an array construction) |
| 55797498c6d9 | fixed | `addDefaultPayloadAccessQualifiersToField` in watched `source/slang/slang-ir-hlsl-legalize.cpp:91-122`: returns early when both sides are present, and fills only the missing side with the literal stage list `caller, anyhit, closesthit, miss` built at `:99-104`. Matches the CHECK67-DAG lines of `raypayload-access-qualifiers-sm67.slang:40,42` and its CHECK66-NOT contrast. | added a before/after payload example showing `read(caller)` preserved and the `write(...)` side filled with the full stage list, plus the SM 6.6 no-qualifier contrast |
| 108e85e927a2 | deferred | The pass body (`source/slang/slang-ir-legalize-uniform-buffer-load.cpp:29-49`) confirms the IR rewrite but says nothing about emit, and the file is outside this page's `watched_paths`. Whether the `makeStruct` survives to text depends on the later `simplifyForEmit` / DCE, so answering "is it observable" requires running `slangc` on a whole-object `ConstantBuffer<S>` read — impossible here (the tree's build is Linux x86-64, the host is arm64). No bundle test anchors `#legalizeuniformbufferload`. Needs a test written against that shape first. | — |
| 7f177d41fca4 | rejected-bogus | The document already states the consequence the gap's suggested addition asks for. `## Loops in the pipeline` reads: "There is also no HLSL legalization driver function to host such a loop; HLSL relies on the downstream DXC / fxc compiler for further optimization. DXC and fxc have their own optimization loops, but those are out of scope." The remaining half of the suggestion ("the emitted text is deliberately un-simplified relative to SPIR-V") is a comparative claim with no support in the watched paths, and the `## Loops in the pipeline` section is mandated as pipeline-structure material by `docs/generated/design/_meta/prompts/_common.md:345-348`. | — |
| 0a34a3865c53 | fixed | CLI spelling `-fspv-reflect` at `source/slang/slang-options.cpp:898-901`. The "no HLSL artifact" half is confirmed in watched paths by absence: `IRUserTypeNameDecoration` (added at `source/slang/slang-ir-user-type-hint.cpp:30`) has no handler anywhere in watched `source/slang/slang-emit-hlsl.cpp` or `slang-emit-c-like.cpp`; its only emit consumer is `source/slang/slang-emit-spirv.cpp:6843-6853`. **`slang-options.cpp` is outside `watched_paths`.** | added `-fspv-reflect` and the statement that the decoration leaves nothing in emitted HLSL to the `VulkanEmitReflection` row |
| 456356a76d90 | fixed | CLI spelling `-embed-downstream-ir` at `source/slang/slang-options.cpp:1218-1221`. The artifact question is answered in watched `source/slang/slang-emit.cpp:707-752`: the pass only removes `IRPublicDecoration` / `IRDownstreamModuleExportDecoration` from functions whose signature mentions `kIROp_HLSLStructuredBufferType` or `kIROp_MatrixType`; neither decoration has an HLSL spelling. **`slang-options.cpp` is outside `watched_paths`.** | added `-embed-downstream-ir` and the statement that the pass narrows the embedded-IR export set without changing emitted HLSL text |
| 151da0a0a8be | fixed | `-trace-coverage-boolean` at `source/slang/slang-options.cpp:642-650` and `-trace-coverage-counter-width <bits>` at `:676-691`. `CoverageCounterWidthBytesInvalid` is `coverage-counter-width-bytes-invalid`, id `45114`, at `source/slang/slang-diagnostics.lua:5258-5262`; the CLI bit-width counterpart is `45113` at `:5251-5256`. Watched `source/slang/slang-emit.cpp:1163-1186` confirms the byte-width check is reachable only when the API option is set directly. **`slang-options.cpp` and `slang-diagnostics.lua` are outside `watched_paths`.** | added both CLI spellings and the E45113 (bits, CLI) / E45114 (bytes, API) split to the `instrumentCoverage` row |
| 21a2e979a3f1 | fixed | Watched `source/slang/slang-emit.cpp:754-830`: the helper returns at `:764-767` unless one of the three denormal modes differs from `FloatingPointDenormalMode::Any`, then adds `FpDenormalPreserve` / `FpDenormalFlushToZero` to entry points. Those decorations are read only at `source/slang/slang-emit-spirv.cpp:6425` and `:6443`; grep for `Denorm` in watched `slang-emit-hlsl.cpp` and `slang-emit-c-like.cpp` returns nothing. | rewrote the `addDenormalModeDecorations` Notes cell to record the early return and that the decorations leave no marker in emitted HLSL |
| fd38d79db386 | fixed | `NonUniformResourceIndex` is declared in watched `source/slang/hlsl.meta.slang:13946-13949` as `__intrinsic_op($(kIROp_NonUniformResourceIndex))` with `[require(cpp_cuda_glsl_hlsl_spirv, nonuniformqualifier)]` — a capability requirement, not a stage one — so the gap's premise that a stage applies is wrong. Textual survival is the verified CHECK of `float-non-uniform-resource-index-fragment.slang:33`; the pass gate `!isSPIRV(target)` is at watched `source/slang/slang-emit.cpp:2270`. | added the `textures[NonUniformResourceIndex(idx)].Sample(samp, uv)` spelling to the Phase C row and stated that no stage gate applies |
| a58af9003740 | fixed | Assembly order in watched `source/slang/slang-emit.cpp:2938-2969` (front matter, prelude, `emitPreModule`, module). `HLSLSourceEmitter::emitFrontMatterImpl` at watched `source/slang/slang-emit-hlsl.cpp:2534` writes the NVAPI defines at `:2542,2547` only under `m_extensionTracker->m_requiresNVAPI`, then `#pragma pack_matrix(...)` from `getMatrixLayoutMode()` at `:2588-2597`. Both markers are verified CHECKs of `prelude-pack-matrix-pragma.slang:26` and `prelude-nvapi-include-conditional.slang:28-30`. Applied together with `d47ad3aa8c64`. | added a `### Shape of the emitted file` subsection under Phase D giving the front-matter/prelude order, the conditional NVAPI defines, and the `#ifdef`-guarded include |
| d47ad3aa8c64 | fixed | `HLSLSourceEmitter::_emitHLSLTextureType` at watched `source/slang/slang-emit-hlsl.cpp:320-393` composes access prefix + base shape + `MS` + `Array` + `<Element[, N]>`; samplers at `:1931-1943`; register letters `b`/`t`/`u`/`s` from `LayoutResourceKind` in `_emitHLSLRegisterSemantic` at `:170-183`. Method spellings are `__intrinsic_asm` strings in watched `source/slang/hlsl.meta.slang` (`.Sample` at `:1408`, the spliced `.Gather$(compareFunc)$(componentFunc)` at `:4370`, `.SampleLevel` / `.SampleGrad` / `.SampleCmp` / `.SampleCmpLevelZero` / `.Load*` throughout). Applied together with `a58af9003740`. | documented the native-spelling composition rule, the register-class mapping, and intrinsic-method pass-through in the same new Phase D subsection |
| ea3331936f79 | fixed | `alias node = _node + _sm_6_8;` at `source/slang/slang-capabilities.capdef:1532`, whose own doc comment reads "requires SM 6.8 or later". The predicate is at watched `source/slang/slang-emit-hlsl.cpp:437`. So in a capability-checked compile the version test is already satisfied and the `node` disjunct cannot decide. **`slang-capabilities.capdef` is outside `watched_paths`.** Existing line number 438 left untouched to avoid an inconsistency with the other reference to the same site. | added a note to the third profile-predicate row that the `node` disjunct is defensive, since the `node` atom conjoins `_sm_6_8` |
| 20f31c85247b | rejected-out-of-scope | The line numbers the gap wants dropped are required by the contract this page was generated from: the Target-pipeline page contract in `docs/generated/design/_meta/prompts/_common.md:299-305` says `## Source` must "Cite the line numbers of the relevant entry points (`linkAndOptimizeIR`, the per-target `emit*ForEntryPoints*`, the per-target `legalize*`)". Staleness of those numbers is handled by the `watched_paths_digest` / `mark-fresh` regeneration cycle, not by softening the page. | — |
| dd82a7558698 | fixed | `invalid-barrier-semantic-flags-value` = id `31116` and `invalid-barrier-memory-type-flags-value` = id `31117` at `source/slang/slang-diagnostics.lua:2657-2670`, with the hex-value message text on `:2662` and `:2669`. The memory-type message is the verified CHECK of `barrier-flags-invalid-memory-type-diag.slang:33`, and the reporting agent observed the same two codes. Emitter side confirmed in watched `source/slang/slang-ir-hlsl-legalize.cpp:69-89`. **`slang-diagnostics.lua` is outside `watched_paths`.** | named E31117 / E31116 in the `validateBarrierFlagsForHLSL` section and quoted the message shape listing the spellable bits with hex values |
| 8db3a13d2d01 | fixed | `_S<instID>` is the emitter's fallback name for an instruction with no name hint, at watched `source/slang/slang-emit-c-like.cpp:1290-1292` (with the `startsWith("_S")` collision guard at `:1003`); the emitted shape is the verified CHECK of `wrap-structured-buffer-of-matrix.slang:31-33`. The `_MatrixStorage_*` half was deliberately **not** written here: that name is built by `lowerBufferElementTypeToStorageType` (`source/slang/slang-ir-lower-buffer-element-type.cpp:2778-2790`, Phase C row 21, outside `watched_paths`), it is not produced by this pass, and its real shape is `_MatrixStorage_<elem><R>x<C>[_ColMajor][_logical]<layoutRule>`, not the `<spelling>natural_<N>` the gap states. | added the `_S<N>` fallback-naming rule and a before/after `RWStructuredBuffer<float4x4>` shape to `wrapStructuredBuffersOfMatrices`, noting the rule is general to anonymous synthesized structs |
