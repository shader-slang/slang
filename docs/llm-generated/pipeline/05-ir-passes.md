---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-15T14:30:00+00:00
source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
watched_paths_digest: 8749b5a60327ef9aea96c0b02a10d643c2d39d04195e7cbd40904b69dabc7f6e
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# IR Passes

This document is a categorized inventory of the IR passes that run
between AST → IR lowering ([04-ast-to-ir.md](04-ast-to-ir.md)) and
target emission ([06-emit.md](06-emit.md)). The intended reader is a
developer adding a pass, debugging a miscompile, or trying to choose
where to insert a new transformation.

The full enumeration of `slang-ir-*.cpp` files lives in the source
tree itself; this document organizes them. Counts in section
headings are approximate (precise at `source_commit`).

## How the passes are ordered

The orchestrator is `linkAndOptimizeIR` in
[slang-emit.cpp](../../../source/slang/slang-emit.cpp) (declared
around line 892 at `source_commit`). It is called by
`emitEntryPointsSourceFromIR` (line 2365) and by the variants used
when emitting LLVM, VM, or other non-textual targets (calls visible
near lines 2508, 3083, 3124, 3178). The function:

1. Links the per-translation-unit IR modules together (using
   [slang-ir-link.cpp](../../../source/slang/slang-ir-link.cpp)).
2. Runs a sequence of validations, specializations, and legalizations.
3. Inserts target-specific passes when the `TargetRequest` indicates
   a target that needs them (HLSL legalization, GLSL legalization,
   SPIR-V legalization, Metal legalization, WGSL legalization, CUDA
   immutable-load handling, etc.).
4. Hands the result to the chosen emit backend
   ([06-emit.md](06-emit.md)).

The pipeline is **not** a fixed list — different targets, different
optimization levels, and the presence of differentiation or coverage
instrumentation all change the sequence. The authoritative ordering
is the body of `linkAndOptimizeIR`; this document reflects categories,
not order.

For an ordered control-flow-graph view of every pass that runs for a
specific target, see the `target-pipelines/` subtree. The first
concrete per-target CFG is
[../target-pipelines/spirv.md](../target-pipelines/spirv.md), which
diagrams every `SLANG_PASS` reachable for `CodeGenTarget::SPIRV`
under the direct-emit path, with conditional gates and the
iterative `simplifyIRForSpirvLegalization` loop drawn explicitly.

A substantial number of passes also run *before* `linkAndOptimizeIR`,
inside `generateIRForTranslationUnit` on the per-translation-unit IR
module before it is cached on the `Module`. Those are documented in
[04b-pre-link-passes.md](04b-pre-link-passes.md) as an ordered,
target-agnostic pipeline (Phase A AST walk → Phase B mandatory
lowering → Phase C mandatory optimization, including the
`performMandatoryEarlyInlining` fixed-point loop → Phase D
non-essential validation and stripping). The per-target IR module
that carries `IRLayoutDecoration`s is built separately by
`TargetProgram::createIRModuleForLayout` and documented in
[04c-layout-ir.md](04c-layout-ir.md); it is **not** fed into
`linkAndOptimizeIR`.

Individual category tables below still list a pass even when its
*primary* call site is in the pre-link region above (for example,
`constructSSA`, `propagateConstExpr`, `eliminateDeadCode`,
`simplifyCFG`, and `peepholeOptimize` are all invoked both in
`generateIRForTranslationUnit` and again from `linkAndOptimizeIR`).
Treat 04b as the authoritative ordering for the pre-link region and
the per-target pages under [../target-pipelines/](../target-pipelines/)
as the authoritative ordering for the post-link region.

## Pass categories

### Linking and validation

Run early and again after major transformations to catch invariant
violations.

| Pass | File | Purpose |
| --- | --- | --- |
| Link | [slang-ir-link.cpp](../../../source/slang/slang-ir-link.cpp) | Pulls in IR for `import`ed modules so all symbols resolve |
| Validate | [slang-ir-validate.cpp](../../../source/slang/slang-ir-validate.cpp) | Structural sanity checks on the IR module |
| Check recursion | [slang-ir-check-recursion.cpp](../../../source/slang/slang-ir-check-recursion.cpp) | Diagnoses unsupported recursion |
| Check unsupported inst | [slang-ir-check-unsupported-inst.cpp](../../../source/slang/slang-ir-check-unsupported-inst.cpp) | Per-target reject-list of opcodes |
| Check shader parameter type | [slang-ir-check-shader-parameter-type.cpp](../../../source/slang/slang-ir-check-shader-parameter-type.cpp) | Validates shader-parameter type rules |
| Check optional `none` usage | [slang-ir-check-optional-none-usage.cpp](../../../source/slang/slang-ir-check-optional-none-usage.cpp) | Diagnoses incorrect uses of `Optional`'s `none` |
| Check generic-with-existential specialize | [slang-ir-check-specialize-generic-with-existential.cpp](../../../source/slang/slang-ir-check-specialize-generic-with-existential.cpp) | Validity of generic / existential specialization |
| Detect uninitialized resources | [slang-ir-detect-uninitialized-resources.cpp](../../../source/slang/slang-ir-detect-uninitialized-resources.cpp) | Diagnoses use of uninitialized resource handles |
| Use of uninitialized values | [slang-ir-use-uninitialized-values.cpp](../../../source/slang/slang-ir-use-uninitialized-values.cpp) | Generic uninitialized-use diagnostic |

### SSA construction and basic cleanup

| Pass | File | Purpose |
| --- | --- | --- |
| SSA | [slang-ir-ssa.cpp](../../../source/slang/slang-ir-ssa.cpp) | Promotes alloca-like locals to SSA values |
| SSA simplification | [slang-ir-ssa-simplification.cpp](../../../source/slang/slang-ir-ssa-simplification.cpp) | Cleans up parameters / arguments after SSA |
| SSA register allocate | [slang-ir-ssa-register-allocate.cpp](../../../source/slang/slang-ir-ssa-register-allocate.cpp) | Names SSA values prior to emit |
| Eliminate phis | [slang-ir-eliminate-phis.cpp](../../../source/slang/slang-ir-eliminate-phis.cpp) | Out-of-SSA conversion for backends that need it |
| DCE | [slang-ir-dce.cpp](../../../source/slang/slang-ir-dce.cpp) | Dead-code elimination |
| SCCP | [slang-ir-sccp.cpp](../../../source/slang/slang-ir-sccp.cpp) | Sparse conditional constant propagation |
| Peephole | [slang-ir-peephole.cpp](../../../source/slang/slang-ir-peephole.cpp) | Local pattern simplifications |
| Simplify CFG | [slang-ir-simplify-cfg.cpp](../../../source/slang/slang-ir-simplify-cfg.cpp) | Empty-block / trivial-branch removal |
| Simplify for emit | [slang-ir-simplify-for-emit.cpp](../../../source/slang/slang-ir-simplify-for-emit.cpp) | Late simplifications targeted at emit |
| Cleanup void | [slang-ir-cleanup-void.cpp](../../../source/slang/slang-ir-cleanup-void.cpp) | Removes spurious `void` instructions |
| Strip default construct | [slang-ir-strip-default-construct.cpp](../../../source/slang/slang-ir-strip-default-construct.cpp) | Drops trivial default-construction insts |
| Strip legalization insts | [slang-ir-strip-legalization-insts.cpp](../../../source/slang/slang-ir-strip-legalization-insts.cpp) | Removes scaffolding from earlier legalization |
| Strip | [slang-ir-strip.cpp](../../../source/slang/slang-ir-strip.cpp) | Drops debug-only / metadata instructions |
| Strip debug info | [slang-ir-strip-debug-info.cpp](../../../source/slang/slang-ir-strip-debug-info.cpp) | Removes debug instructions when not requested |
| Redundancy removal | [slang-ir-redundancy-removal.cpp](../../../source/slang/slang-ir-redundancy-removal.cpp) | CSE-style cleanup |
| Single return | [slang-ir-single-return.cpp](../../../source/slang/slang-ir-single-return.cpp) | Funnels every function to a single `return` |
| Eliminate multilevel break | [slang-ir-eliminate-multilevel-break.cpp](../../../source/slang/slang-ir-eliminate-multilevel-break.cpp) | Lowers labeled / multilevel `break` |
| Missing return | [slang-ir-missing-return.cpp](../../../source/slang/slang-ir-missing-return.cpp) | Diagnoses paths missing a `return` |
| Init local var | [slang-ir-init-local-var.cpp](../../../source/slang/slang-ir-init-local-var.cpp) | Inserts default initialization for locals |
| Variable-scope correction | [slang-ir-variable-scope-correction.cpp](../../../source/slang/slang-ir-variable-scope-correction.cpp) | Fixes scope hoisting |
| Operator shift overflow | [slang-ir-operator-shift-overflow.cpp](../../../source/slang/slang-ir-operator-shift-overflow.cpp) | Diagnoses bad shift counts |

### Specialization and generics

The specialization machinery is what turns generic Slang IR into the
concrete IR consumed by emit.

| Pass | File | Purpose |
| --- | --- | --- |
| Specialize | [slang-ir-specialize.cpp](../../../source/slang/slang-ir-specialize.cpp) | Generic-parameter substitution; the main specialization driver |
| Specialize function call | [slang-ir-specialize-function-call.cpp](../../../source/slang/slang-ir-specialize-function-call.cpp) | Per-call specialization |
| Specialize address space | [slang-ir-specialize-address-space.cpp](../../../source/slang/slang-ir-specialize-address-space.cpp) | Address-space-parametric specialization |
| Specialize arrays | [slang-ir-specialize-arrays.cpp](../../../source/slang/slang-ir-specialize-arrays.cpp) | Specializes array-typed generic parameters |
| Specialize buffer-load arg | [slang-ir-specialize-buffer-load-arg.cpp](../../../source/slang/slang-ir-specialize-buffer-load-arg.cpp) | Buffer-load argument specialization |
| Specialize matrix layout | [slang-ir-specialize-matrix-layout.cpp](../../../source/slang/slang-ir-specialize-matrix-layout.cpp) | Resolves matrix layout choices |
| Specialize resources | [slang-ir-specialize-resources.cpp](../../../source/slang/slang-ir-specialize-resources.cpp) | Specializes resource-typed parameters |
| Specialize stage switch | [slang-ir-specialize-stage-switch.cpp](../../../source/slang/slang-ir-specialize-stage-switch.cpp) | Resolves `[stage]`-conditional code |
| Specialize target switch | [slang-ir-specialize-target-switch.cpp](../../../source/slang/slang-ir-specialize-target-switch.cpp) | Resolves `[target]`-conditional code |
| Defunctionalization | [slang-ir-defunctionalization.cpp](../../../source/slang/slang-ir-defunctionalization.cpp) | First-class function values → tagged unions |
| Bind existentials | [slang-ir-bind-existentials.cpp](../../../source/slang/slang-ir-bind-existentials.cpp) | Resolves dynamic-dispatch interface bindings |
| AnyValue inference | [slang-ir-any-value-inference.cpp](../../../source/slang/slang-ir-any-value-inference.cpp) | Determines `AnyValue` size for existentials |
| AnyValue marshalling | [slang-ir-any-value-marshalling.cpp](../../../source/slang/slang-ir-any-value-marshalling.cpp) | Pack / unpack values into `AnyValue` |
| Lower dynamic-dispatch insts | [slang-ir-lower-dynamic-dispatch-insts.cpp](../../../source/slang/slang-ir-lower-dynamic-dispatch-insts.cpp) | Implements `lookup_witness` and friends |
| Lower expand type | [slang-ir-lower-expand-type.cpp](../../../source/slang/slang-ir-lower-expand-type.cpp) | Variadic-pack expansion |
| Remove unused generic param | [slang-ir-remove-unused-generic-param.cpp](../../../source/slang/slang-ir-remove-unused-generic-param.cpp) | Cleans up specialized generics |
| Deduplicate | [slang-ir-deduplicate.cpp](../../../source/slang/slang-ir-deduplicate.cpp) | Re-deduplicates hoistable insts after edits |
| Deduplicate generic children | [slang-ir-deduplicate-generic-children.cpp](../../../source/slang/slang-ir-deduplicate-generic-children.cpp) | Deduplicates within a generic's body |
| Typeflow set | [slang-ir-typeflow-set.cpp](../../../source/slang/slang-ir-typeflow-set.cpp) | Type-flow set construction |
| Typeflow specialize | [slang-ir-typeflow-specialize.cpp](../../../source/slang/slang-ir-typeflow-specialize.cpp) | Specialization based on type flow |
| Constexpr | [slang-ir-constexpr.cpp](../../../source/slang/slang-ir-constexpr.cpp) | Compile-time evaluation |

### Differentiation (autodiff)

Forward- and reverse-mode automatic differentiation transformations.
The deeper user-level documentation lives in
[../../design/autodiff.md](../../design/autodiff.md).

| Pass | File | Purpose |
| --- | --- | --- |
| Autodiff (driver) | [slang-ir-autodiff.cpp](../../../source/slang/slang-ir-autodiff.cpp) | Top-level differentiation orchestration |
| Forward mode | [slang-ir-autodiff-fwd.cpp](../../../source/slang/slang-ir-autodiff-fwd.cpp) | Forward-mode derivative emission |
| Reverse mode | [slang-ir-autodiff-rev.cpp](../../../source/slang/slang-ir-autodiff-rev.cpp) | Reverse-mode derivative emission |
| Transpose | [slang-ir-autodiff-transpose.cpp](../../../source/slang/slang-ir-autodiff-transpose.cpp) | Reverse-mode transposition |
| Unzip | [slang-ir-autodiff-unzip.cpp](../../../source/slang/slang-ir-autodiff-unzip.cpp) | Splits primal / differential code paths |
| CFG normalization | [slang-ir-autodiff-cfg-norm.cpp](../../../source/slang/slang-ir-autodiff-cfg-norm.cpp) | CFG canonicalization for autodiff |
| Loop analysis | [slang-ir-autodiff-loop-analysis.cpp](../../../source/slang/slang-ir-autodiff-loop-analysis.cpp) | Loop-structure analysis used by autodiff |
| Pairs | [slang-ir-autodiff-pairs.cpp](../../../source/slang/slang-ir-autodiff-pairs.cpp) | Primal / differential pair construction |
| Primal hoist | [slang-ir-autodiff-primal-hoist.cpp](../../../source/slang/slang-ir-autodiff-primal-hoist.cpp) | Hoists primal computation for reuse |
| Region | [slang-ir-autodiff-region.cpp](../../../source/slang/slang-ir-autodiff-region.cpp) | Differentiable-region analysis |
| Check differentiability | [slang-ir-check-differentiability.cpp](../../../source/slang/slang-ir-check-differentiability.cpp) | Validates that flagged code is actually differentiable |

### Type and value legalization

Adapt high-level Slang types to the simpler shapes downstream targets
expect.

| Pass | File | Purpose |
| --- | --- | --- |
| Legalize types | [slang-ir-legalize-types.cpp](../../../source/slang/slang-ir-legalize-types.cpp) | Splits aggregate / parametric types |
| Legalize array return type | [slang-ir-legalize-array-return-type.cpp](../../../source/slang/slang-ir-legalize-array-return-type.cpp) | Returning arrays by reference |
| Legalize binary operator | [slang-ir-legalize-binary-operator.cpp](../../../source/slang/slang-ir-legalize-binary-operator.cpp) | Per-target binary-op legalization |
| Legalize composite select | [slang-ir-legalize-composite-select.cpp](../../../source/slang/slang-ir-legalize-composite-select.cpp) | Lowers `select` on composites |
| Legalize empty array | [slang-ir-legalize-empty-array.cpp](../../../source/slang/slang-ir-legalize-empty-array.cpp) | Avoids zero-length arrays |
| Legalize global values | [slang-ir-legalize-global-values.cpp](../../../source/slang/slang-ir-legalize-global-values.cpp) | Legalizes module-scope values |
| Legalize image subscript | [slang-ir-legalize-image-subscript.cpp](../../../source/slang/slang-ir-legalize-image-subscript.cpp) | Rewrites image indexing |
| Legalize matrix types | [slang-ir-legalize-matrix-types.cpp](../../../source/slang/slang-ir-legalize-matrix-types.cpp) | Per-target matrix shape adjustments |
| Legalize mesh outputs | [slang-ir-legalize-mesh-outputs.cpp](../../../source/slang/slang-ir-legalize-mesh-outputs.cpp) | Mesh-shader output rewriting |
| Legalize uniform-buffer load | [slang-ir-legalize-uniform-buffer-load.cpp](../../../source/slang/slang-ir-legalize-uniform-buffer-load.cpp) | Constant-buffer load legalization |
| Legalize varying params | [slang-ir-legalize-varying-params.cpp](../../../source/slang/slang-ir-legalize-varying-params.cpp) | Stage-input / stage-output rewriting |
| Legalize vector types | [slang-ir-legalize-vector-types.cpp](../../../source/slang/slang-ir-legalize-vector-types.cpp) | Per-target vector shape adjustments |
| Byte-address legalize | [slang-ir-byte-address-legalize.cpp](../../../source/slang/slang-ir-byte-address-legalize.cpp) | `ByteAddressBuffer` lowering |
| Lower buffer element type | [slang-ir-lower-buffer-element-type.cpp](../../../source/slang/slang-ir-lower-buffer-element-type.cpp) | Per-target buffer element transforms |
| Lower combined texture sampler | [slang-ir-lower-combined-texture-sampler.cpp](../../../source/slang/slang-ir-lower-combined-texture-sampler.cpp) | OpenGL-style sampler combination |
| Lower bit cast | [slang-ir-lower-bit-cast.cpp](../../../source/slang/slang-ir-lower-bit-cast.cpp) | `bit_cast` lowering |
| Lower l-value cast | [slang-ir-lower-l-value-cast.cpp](../../../source/slang/slang-ir-lower-l-value-cast.cpp) | L-value cast → ptr operations |
| Lower matrix swizzle store | [slang-ir-lower-matrix-swizzle-store.cpp](../../../source/slang/slang-ir-lower-matrix-swizzle-store.cpp) | Matrix swizzle write lowering |
| Lower optional type | [slang-ir-lower-optional-type.cpp](../../../source/slang/slang-ir-lower-optional-type.cpp) | `Optional<T>` lowering |
| Lower result type | [slang-ir-lower-result-type.cpp](../../../source/slang/slang-ir-lower-result-type.cpp) | `Result<T,E>` lowering |
| Lower tuple types | [slang-ir-lower-tuple-types.cpp](../../../source/slang/slang-ir-lower-tuple-types.cpp) | Tuple decomposition |
| Lower enum type | [slang-ir-lower-enum-type.cpp](../../../source/slang/slang-ir-lower-enum-type.cpp) | Enum → integer lowering |
| Lower conditional type | [slang-ir-lower-conditional-type.cpp](../../../source/slang/slang-ir-lower-conditional-type.cpp) | Conditional types lowering |
| Lower defer | [slang-ir-lower-defer.cpp](../../../source/slang/slang-ir-lower-defer.cpp) | `defer` statements → cleanup blocks |
| Lower error handling | [slang-ir-lower-error-handling.cpp](../../../source/slang/slang-ir-lower-error-handling.cpp) | `throws` lowering |
| Lower reinterpret | [slang-ir-lower-reinterpret.cpp](../../../source/slang/slang-ir-lower-reinterpret.cpp) | `reinterpret` operator |
| Lower out parameters | [slang-ir-lower-out-parameters.cpp](../../../source/slang/slang-ir-lower-out-parameters.cpp) | Out-parameter ABI translation |
| Lower copy logical | [slang-ir-lower-copy-logical.cpp](../../../source/slang/slang-ir-lower-copy-logical.cpp) | Logical-copy semantics |
| Lower coopvec | [slang-ir-lower-coopvec.cpp](../../../source/slang/slang-ir-lower-coopvec.cpp) | Cooperative vectors |
| Lower COM methods | [slang-ir-lower-com-methods.cpp](../../../source/slang/slang-ir-lower-com-methods.cpp) | COM-style virtual call lowering |
| Lower append/consume structured buffer | [slang-ir-lower-append-consume-structured-buffer.cpp](../../../source/slang/slang-ir-lower-append-consume-structured-buffer.cpp) | Append / consume buffer ABI |
| Lower binding query | [slang-ir-lower-binding-query.cpp](../../../source/slang/slang-ir-lower-binding-query.cpp) | Binding-query intrinsic lowering |
| Lower CPU resource types | [slang-ir-lower-cpu-resource-types.cpp](../../../source/slang/slang-ir-lower-cpu-resource-types.cpp) | CPU target resource shape |
| Lower CUDA builtin types | [slang-ir-lower-cuda-builtin-types.cpp](../../../source/slang/slang-ir-lower-cuda-builtin-types.cpp) | CUDA builtin type adjustments |
| Lower GLSL SSBO types | [slang-ir-lower-glsl-ssbo-types.cpp](../../../source/slang/slang-ir-lower-glsl-ssbo-types.cpp) | GLSL SSBO ABI |
| Lower dynamic resource heap | [slang-ir-lower-dynamic-resource-heap.cpp](../../../source/slang/slang-ir-lower-dynamic-resource-heap.cpp) | Bindless resource heap |
| Wrap cbuffer element | [slang-ir-wrap-cbuffer-element.cpp](../../../source/slang/slang-ir-wrap-cbuffer-element.cpp) | Wraps cbuffer scalars |
| Wrap structured buffers | [slang-ir-wrap-structured-buffers.cpp](../../../source/slang/slang-ir-wrap-structured-buffers.cpp) | Structured-buffer element wrapping |
| Bit-field accessors | [slang-ir-bit-field-accessors.cpp](../../../source/slang/slang-ir-bit-field-accessors.cpp) | Bit-field load / store |
| Address inst elimination | [slang-ir-addr-inst-elimination.cpp](../../../source/slang/slang-ir-addr-inst-elimination.cpp) | Removes redundant address operations |
| Extract value from type | [slang-ir-extract-value-from-type.cpp](../../../source/slang/slang-ir-extract-value-from-type.cpp) | Type-level extraction |
| Resolve texture format | [slang-ir-resolve-texture-format.cpp](../../../source/slang/slang-ir-resolve-texture-format.cpp) | Resolves texture formats |
| Resolve varying input ref | [slang-ir-resolve-varying-input-ref.cpp](../../../source/slang/slang-ir-resolve-varying-input-ref.cpp) | Stage-input reference resolution |

### Inlining and call-graph

| Pass | File | Purpose |
| --- | --- | --- |
| Inline | [slang-ir-inline.cpp](../../../source/slang/slang-ir-inline.cpp) | Function inlining |
| Call graph | [slang-ir-call-graph.cpp](../../../source/slang/slang-ir-call-graph.cpp) | Call-graph analysis |
| Reachability | [slang-ir-reachability.cpp](../../../source/slang/slang-ir-reachability.cpp) | Reachability analysis |
| Propagate func properties | [slang-ir-propagate-func-properties.cpp](../../../source/slang/slang-ir-propagate-func-properties.cpp) | Bottom-up function-property inference |
| DLL export | [slang-ir-dll-export.cpp](../../../source/slang/slang-ir-dll-export.cpp) | Marks exported entry points |
| DLL import | [slang-ir-dll-import.cpp](../../../source/slang/slang-ir-dll-import.cpp) | Resolves cross-module imports |
| Marshal native call | [slang-ir-marshal-native-call.cpp](../../../source/slang/slang-ir-marshal-native-call.cpp) | Native ABI marshalling |
| Defer buffer load | [slang-ir-defer-buffer-load.cpp](../../../source/slang/slang-ir-defer-buffer-load.cpp) | Sinks buffer loads to first use |

### Entry-point and parameter handling

| Pass | File | Purpose |
| --- | --- | --- |
| Entry-point pass framework | [slang-ir-entry-point-pass.cpp](../../../source/slang/slang-ir-entry-point-pass.cpp) | Common scaffold for per-entry-point passes |
| Entry-point decorations | [slang-ir-entry-point-decorations.cpp](../../../source/slang/slang-ir-entry-point-decorations.cpp) | Applies stage / numthreads / etc. |
| Entry-point raw-ptr params | [slang-ir-entry-point-raw-ptr-params.cpp](../../../source/slang/slang-ir-entry-point-raw-ptr-params.cpp) | Pointer-typed entry-point parameters |
| Entry-point uniforms | [slang-ir-entry-point-uniforms.cpp](../../../source/slang/slang-ir-entry-point-uniforms.cpp) | Promotes uniforms to entry-point parameters |
| Optix entry-point uniforms | [slang-ir-optix-entry-point-uniforms.cpp](../../../source/slang/slang-ir-optix-entry-point-uniforms.cpp) | OptiX-specific entry-point uniform handling |
| Fix entry-point callsite | [slang-ir-fix-entrypoint-callsite.cpp](../../../source/slang/slang-ir-fix-entrypoint-callsite.cpp) | Cleans up callers of entry-point functions |
| Transform params to constref | [slang-ir-transform-params-to-constref.cpp](../../../source/slang/slang-ir-transform-params-to-constref.cpp) | `in` parameters → `const&` |
| Undo param copy | [slang-ir-undo-param-copy.cpp](../../../source/slang/slang-ir-undo-param-copy.cpp) | Undoes pessimistic param copies |

### Layout and binding

| Pass | File | Purpose |
| --- | --- | --- |
| Layout | [slang-ir-layout.cpp](../../../source/slang/slang-ir-layout.cpp) | Computes IR-level layout |
| Collect global uniforms | [slang-ir-collect-global-uniforms.cpp](../../../source/slang/slang-ir-collect-global-uniforms.cpp) | Gathers globals into uniform buffers |
| Explicit global context | [slang-ir-explicit-global-context.cpp](../../../source/slang/slang-ir-explicit-global-context.cpp) | Threads global context through calls |
| Explicit global init | [slang-ir-explicit-global-init.cpp](../../../source/slang/slang-ir-explicit-global-init.cpp) | Lifts global initializers |
| Translate global varying var | [slang-ir-translate-global-varying-var.cpp](../../../source/slang/slang-ir-translate-global-varying-var.cpp) | Stage-varying globals |
| Late require capability | [slang-ir-late-require-capability.cpp](../../../source/slang/slang-ir-late-require-capability.cpp) | Adds capability requirements after specialization |
| User type hint | [slang-ir-user-type-hint.cpp](../../../source/slang/slang-ir-user-type-hint.cpp) | Carries user type hints to backends |
| Metadata | [slang-ir-metadata.cpp](../../../source/slang/slang-ir-metadata.cpp) | IR-level metadata management |
| String hash | [slang-ir-string-hash.cpp](../../../source/slang/slang-ir-string-hash.cpp) | Stable string hashing for symbols |

### Loop transformations

| Pass | File | Purpose |
| --- | --- | --- |
| Loop unroll | [slang-ir-loop-unroll.cpp](../../../source/slang/slang-ir-loop-unroll.cpp) | `[unroll]` enforcement and unrolling |
| Loop inversion | [slang-ir-loop-inversion.cpp](../../../source/slang/slang-ir-loop-inversion.cpp) | Loop-form normalization |
| Fuse satcoop | [slang-ir-fuse-satcoop.cpp](../../../source/slang/slang-ir-fuse-satcoop.cpp) | Saturated-cooperative fusion |
| Restructure | [slang-ir-restructure.cpp](../../../source/slang/slang-ir-restructure.cpp) | Re-forms structured control flow |
| Restructure scoping | [slang-ir-restructure-scoping.cpp](../../../source/slang/slang-ir-restructure-scoping.cpp) | Structured-region scope inference |
| Synthesize active mask | [slang-ir-synthesize-active-mask.cpp](../../../source/slang/slang-ir-synthesize-active-mask.cpp) | Wave / SIMT active mask |
| Uniformity | [slang-ir-uniformity.cpp](../../../source/slang/slang-ir-uniformity.cpp) | Uniformity (divergence) analysis |

### Target-specific lowering

These passes run only for their named target.

| Pass | File | Target |
| --- | --- | --- |
| GLSL legalize | [slang-ir-glsl-legalize.cpp](../../../source/slang/slang-ir-glsl-legalize.cpp) | GLSL |
| GLSL liveness | [slang-ir-glsl-liveness.cpp](../../../source/slang/slang-ir-glsl-liveness.cpp) | GLSL |
| HLSL legalize | [slang-ir-hlsl-legalize.cpp](../../../source/slang/slang-ir-hlsl-legalize.cpp) | HLSL |
| Metal legalize | [slang-ir-metal-legalize.cpp](../../../source/slang/slang-ir-metal-legalize.cpp) | Metal |
| SPIR-V legalize | [slang-ir-spirv-legalize.cpp](../../../source/slang/slang-ir-spirv-legalize.cpp) | SPIR-V |
| SPIR-V snippet | [slang-ir-spirv-snippet.cpp](../../../source/slang/slang-ir-spirv-snippet.cpp) | SPIR-V |
| WGSL legalize | [slang-ir-wgsl-legalize.cpp](../../../source/slang/slang-ir-wgsl-legalize.cpp) | WGSL |
| CUDA immutable load | [slang-ir-cuda-immutable-load.cpp](../../../source/slang/slang-ir-cuda-immutable-load.cpp) | CUDA |
| Vulkan invert Y | [slang-ir-vk-invert-y.cpp](../../../source/slang/slang-ir-vk-invert-y.cpp) | Vulkan / SPIR-V |
| Float non-uniform resource index | [slang-ir-float-non-uniform-resource-index.cpp](../../../source/slang/slang-ir-float-non-uniform-resource-index.cpp) | DX12 bindless |
| Early raytracing intrinsic simplification | [slang-ir-early-raytracing-intrinsic-simplification.cpp](../../../source/slang/slang-ir-early-raytracing-intrinsic-simplification.cpp) | Raytracing targets |
| Mesh-output reads | [slang-ir-mesh-output-reads.cpp](../../../source/slang/slang-ir-mesh-output-reads.cpp) | Mesh-shader-capable targets |
| PyTorch C++ binding | [slang-ir-pytorch-cpp-binding.cpp](../../../source/slang/slang-ir-pytorch-cpp-binding.cpp) | Torch |
| COM interface | [slang-ir-com-interface.cpp](../../../source/slang/slang-ir-com-interface.cpp) | CPU / COM |
| Translate | [slang-ir-translate.cpp](../../../source/slang/slang-ir-translate.cpp) | Generic translation step used by some targets |

### Instrumentation

| Pass | File | Purpose |
| --- | --- | --- |
| Coverage instrument | [slang-ir-coverage-instrument.cpp](../../../source/slang/slang-ir-coverage-instrument.cpp) | Instruments shaders for coverage tracking; honors `-trace-coverage-binding` and `-trace-coverage-reserved-space` for explicit / reserved binding-slot control |
| Finalize coverage metadata | [slang-ir-coverage-instrument.cpp](../../../source/slang/slang-ir-coverage-instrument.cpp) | `finalizeCoverageInstrumentationMetadata`; runs after global / entry-point uniform packing to fill in CPU/CUDA uniform-marshaling fields determined by the final post-packing layout |
| Insert debug value store | [slang-ir-insert-debug-value-store.cpp](../../../source/slang/slang-ir-insert-debug-value-store.cpp) | Debug-info preservation across optimization |
| Liveness | [slang-ir-liveness.cpp](../../../source/slang/slang-ir-liveness.cpp) | Liveness analysis used by debug info |
| Obfuscate loc | [slang-ir-obfuscate-loc.cpp](../../../source/slang/slang-ir-obfuscate-loc.cpp) | Optional source-loc obfuscation for distributed modules |

### Other passes

A handful of passes do not fit the categories above. They are still
listed for completeness.

| Pass | File | Purpose |
| --- | --- | --- |
| SPIR-V opcode info / snippet | [slang-ir-spirv-snippet.cpp](../../../source/slang/slang-ir-spirv-snippet.cpp) | SPIR-V code-snippet helpers used by SPIR-V passes |

## Pass utilities

These files do not implement transformations but are linked into
many passes. They provide IR walking, instruction-info lookup, and
cloning support that the categorized passes above rely on.

| Module | File | Purpose |
| --- | --- | --- |
| Clone | [slang-ir-clone.cpp](../../../source/slang/slang-ir-clone.cpp) | Generic IR clone helpers (used by inlining, specialization, generics) |
| Dominators | [slang-ir-dominators.cpp](../../../source/slang/slang-ir-dominators.cpp) | Dominator-tree construction; used by SSA construction, loop and SCCP passes |
| Util | [slang-ir-util.cpp](../../../source/slang/slang-ir-util.cpp) | Common IR walking / mutation primitives |
| Insts info | [slang-ir-insts-info.cpp](../../../source/slang/slang-ir-insts-info.cpp) | Opcode tables, name lookup, and per-opcode metadata used by the pretty printer and passes that switch on opcode |
| Insts stable names | [slang-ir-insts-stable-names.cpp](../../../source/slang/slang-ir-insts-stable-names.cpp) | Maps between opcode enum values and serialization-stable string names |

## Adding a new pass

When adding a pass:

1. Create a `slang-ir-<name>.{h,cpp}` pair under
   [source/slang/](../../../source/slang/). The header declares the
   public entry function (e.g. `void doMyPass(IRModule*, ...)`); the
   `.cpp` contains the implementation.
2. Use the helpers in
   [slang-ir.h](../../../source/slang/slang-ir.h) and
   [slang-ir-clone.cpp](../../../source/slang/slang-ir-clone.cpp) /
   [slang-ir-dominators.cpp](../../../source/slang/slang-ir-dominators.cpp) /
   [slang-ir-call-graph.cpp](../../../source/slang/slang-ir-call-graph.cpp)
   instead of writing one-off traversals; respect the hoistable / global-
   value rules described in
   [../../design/ir.md](../../design/ir.md).
3. Insert a call site in `linkAndOptimizeIR`
   ([slang-emit.cpp](../../../source/slang/slang-emit.cpp)) at the
   appropriate point in the sequence.
4. If the pass is target-specific, gate it on the `TargetRequest`.
5. Add tests under [tests/](../../../tests/) — typically a
   `COMPARE_COMPUTE` or `INTERPRET` test, plus a `DIAGNOSTIC_TEST` if
   the pass emits errors.
6. Run `./extras/formatting.sh` before committing.

## What is not in this document

- The exact order in which passes run for any given target. Read
  `linkAndOptimizeIR` in
  [slang-emit.cpp](../../../source/slang/slang-emit.cpp) for the
  authoritative sequence.
- The IR design itself — see
  [../../design/ir.md](../../design/ir.md).
- The opcode catalog — see
  [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).
- The code-emission stage that consumes the optimized IR — see
  [06-emit.md](06-emit.md).
