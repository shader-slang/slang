# Preserve CUDA floating-point remainder semantics

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, canonical scalar and selected-vector `kIROp_FRem` values use CUDA/libdevice
`fmod` semantics through the direct NVVM path. Integer `kIROp_IRem` remains the provider's native
integer remainder operation, and the provider's generic floating remainder operation remains
available to direct builder clients.

The bounded primary workload is `hlsl-intrinsic/matrix-float.slang`. Native CUDA and direct NVVM
O0/O3 must produce the same runtime result before the workload receives permanent direct-NVVM
coverage. Frozen corpus v1 and the discovery corpus remain separate regression contracts.

## Progress

- [x] (2026-09-01) Reproduced the matrix runtime mismatch in direct O0 and O3.
- [x] (2026-09-01) Reduced the mismatch to the first canonical `kIROp_FRem` produced by matrix
  arithmetic legalization; all earlier matrix transport and `log10` checkpoints agree.
- [x] (2026-09-01) Audited the provider: floating remainder currently reaches LLVM `CreateFRem`,
  while the existing scalar `FMOD` semantic operation reaches `__nv_fmodf`/`__nv_fmod`.
- [x] (2026-09-01) Implemented one compiler-side scalar/vector remainder recipe using existing extraction,
  construction, and generic value-operation callbacks.
- [x] (2026-09-01) Added focused ownership and runtime coverage, promoted the primary workload, ran both corpora
  and representative measurements, document the result, and commit Slice 176.

## Surprises and Discoveries

- The first three matrix lanes happen to agree. The fourth lane is near an exact multiple of
  `0.11f`, where LLVM `frem` and CUDA's device `fmodf` produce observably different values.
- Dynamic-array extraction, matrix construction, matrix `log10`, and prior arithmetic all match
  the native lane values. The mismatch begins only after `%`, scaling, rounding, and integer-to-
  float conversion.
- The LLVM provider ABI already expresses the required implementation. Scalar FMOD, dynamic lane
  extraction, and vector construction are revision-32 operations.
- The first complete frozen replay exposed two parts of the same requirement-collection cascade.
  Canonical `frem` permits scalar broadcast into a vector result, and requesting scalar FMOD must
  also set the module's libdevice requirement. Without the latter, an otherwise correct PTX module
  retained an unresolved `__nv_fmodf` declaration.

## Decision Log

- Decision: legalize canonical floating `%` in the compiler to scalar CUDA FMOD operations,
  recursively over selected vector lanes.
  Rationale: Slang's CUDA route promises CUDA runtime behavior; LLVM `frem` is not a semantic
  substitute on the measured near-multiple input. The compiler owns classification of Slang IR,
  while the provider already owns the exact scalar libdevice call.
  Date/author: 2026-09-01, Codex.
- Decision: keep `SLANG_NVVM_VALUE_OP_REMAINDER` and its provider implementation.
  Rationale: integer remainder still uses it, and the generic builder API may intentionally request
  LLVM floating remainder. Only canonical Slang `kIROp_FRem` requires CUDA legalization.
  Date/author: 2026-09-01, Codex.
- Decision: keep provider ABI revision 32.
  Rationale: no canonical operation is missing from the current generic interface.
  Date/author: 2026-09-01, Codex.

## Context and Current Pipeline

CUDA matrix legalization flattens the selected matrix arithmetic to ordinary floating scalar or
vector IR before direct emission. `_getNVVMValueOperation` currently classifies both `kIROp_IRem`
and `kIROp_FRem` as `SLANG_NVVM_VALUE_OP_REMAINDER`. The LLVM provider implements floating members
of that family with `IRBuilder::CreateFRem`. Separately, the scalar intrinsic catalog already maps
`SLANG_NVVM_VALUE_OP_FMOD` to libdevice `__nv_fmodf` and `__nv_fmod`.

## Scope and Non-Goals

In scope are exact scalar Float32/Float64 and selected homogeneous vectors of those leaves; shared
capability discovery and emission resolution; lane-wise reconstruction through existing builder
operations; focused fake/provider/runtime tests; permanent promotion of newly correct stable
workloads; and the usual frozen/discovery/measurement evidence.

Out of scope are integer remainder changes, half remainder, arbitrary aggregate recursion, fast-
math policy changes, provider ABI revisions, fixture-name checks, textual IR rewriting, and any
unrelated matrix or arithmetic widening.

## Canonical Representation and Ownership

The accepted shape is exactly a binary `kIROp_FRem` in the established component-wise Float32/64
family. Its result is a supported scalar or selected vector, and each operand is either that result
shape or the corresponding scalar broadcast. The producer is ordinary Slang arithmetic lowering,
including matrix legalization that flattens component-wise `%` into a selected vector. This shape
is canonical and intentionally valid. It must not be repaired upstream because the IR operation
accurately represents source floating remainder; the target-specific distinction is which CUDA
operation implements it.

The resolver will produce one scalar Float32/Float64 FMOD descriptor and an element count. Both
preflight and emission use that resolver. Emission applies the descriptor directly to a scalar or
extracts matching vector lanes, applies scalar FMOD, and reconstructs the exact selected vector.
Malformed type relations continue to receive deterministic preflight diagnostics.

## Validation

Build the host compiler outside the sandbox. Run focused fake/provider unit coverage and
`hlsl-intrinsic/matrix-float.slang` through native CUDA plus direct NVVM O0/O3. Promote only exact
runtime equality. Then run the selected NVVM prefix, permanent NVVM category, exact frozen corpus
v1 identity and semantic comparison, exact discovery identity and semantic comparison, and the
representative native/direct O0/O3 SM70/SM80/SM90 measurements. Run the pinned changed-line format
check and self-review every new helper or special case before committing.

## Outcomes and Retrospective

Slice 176 unlocks frozen `hlsl-intrinsic/matrix-float` in both modes and promotes two permanent
direct lanes. Frozen v1 remains exactly 452 workloads/427 healthy references and advances from
413/413/413 to 414/414/414, with exactly one gain and no old-correct regression. Discovery remains
exactly 82/72 at 72/72/72 with no changed row.

The selected prefix passes 433/433 and the permanent `nvvm` category passes 82/82. The matrix gate
compiles and assembles through CUDA 12.9 for native NVRTC, direct O0 SM70, and direct O3
SM70/SM80/SM90. Provider ABI revision 32 remains unchanged. The failed first frozen replay was
retained as a process discovery: it caught scalar-broadcast classification and the module-level
libdevice dependency before either could become a committed regression.
