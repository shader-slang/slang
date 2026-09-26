# Integrate master before the results baseline

## Motivation

The results package needs the new backend measured against current Slang behavior. Integrate
upstream `6eb89786ca882d71049c8568638e247f60864b6f` before freezing measurements, preserving the
accepted260 runtime obligations rather than comparing old binaries to new source.

## Proposed solution

Merge master at `5294b5ae697fd3ca0164b929818ee69367daa4f5`, preserving upstream public numbering and
qualified NVVM semantics. Rebuild matching generated modules, compiler, provider and test tools;
review full correctness outcomes against accepted260 before accepting a results baseline.

## Change summary

Twelve text conflicts and one delete/modify conflict were resolved. CUDA option IDs move to160–162;
NVVM instruction/decoration IDs move to905/906, retaining upstream158/159 and902–904. The unused
historical NVVM E52014 placeholder is removed; active E52015–18 remain. Packaging retains both NVVM
and user skills. Test directives retain branch coverage and upstream GLSL imports/texture changes.

The removed version-query API is replaced by upstream's path-query contract in the NVVM adapter and
unit/CLI coverage. LLVM coexistence tests continue checking LLVM21.1 alongside provider LLVM14.
All22 submodules match the merged pins. Three new dependencies were recovered from official GitHub
archives/API after Git endpoints returned503; exact commit/tree hashes and strict fsck passed.
Their checkouts have shallow history. No alternative dependency revisions were substituted.

## Concepts and vocabulary

Stable IR IDs identify serialized instructions; changed branch IDs require regenerated module caches.
The optional path-provider interface reports the actual loaded compiler library using a held symbol.
A frozen runtime cell is one registered test identity/backend mode with an independent output oracle.

## Process report

For a pointer into a uniform CUDA parameter group, `__ldg` would select the wrong address space.
Upstream `isAddressIntoCudaConstantParameterGroup` preserves that exclusion through casts/offsets;
we retain it and remove the redundant branch `canUseReadOnlyGlobalLoad`. A resource pointer loaded
from the group still addresses its original resource. Existing constant/buffer tests remain.

Upstream texture fetches reshape float3/int3/uint3 results. Those branches remain verbatim; original
NVVM semantic tags stay on their generic fallbacks, with no speculative new supported shape.
`visitIntrinsicAsmStmt` now materializes explicit checked values through `getSimpleVal`; NVVM still
copies operand order and requires its existing canonical helper shape. The generated AST table,
inline predicate/constructor, bounds checks and cardinality assertion remain one source of truth.

`Session::getDownstreamCompilerPath` discovers the compiler and queries the optional borrowed path
interface exposed by `DownstreamCompilerBase::castAs/getObject`. NVVM's override delegates to
`getPathFromSymbol(m_nvvmVersion)`; that required symbol is initialized before compiler admission,
and its owning library remains held. No new public interface, fallback or reconstructed path exists.
Positive coverage reloads the returned NVVM library and calls its version symbol. Injected-library
coverage checks the real fake-symbol owner and preserves single-load/lifetime assertions.

Old LLVM binaries need not implement the new optional path interface. `_queryLLVM21` instead uses
the existing V4 locator/descriptor and keeps its compiler set alive across the entire coexistence
exercise, including the parent preflight (caught by the first build and fixed before rerun).
Session discovery still occurs in the requested fresh-process order. Both LLVM versions
and both load orders remain required; no hidden Session symbol is called across the test DSO.

Source review found879 unique stable IR IDs and111 unique explicit option IDs. The14 changed runtime
source paths preserve TEST_INPUT directives and expected output contracts; updated hashes must be
reviewed separately from execution. Upstream version-query and CUDA callable test identity changes
have explicit replacement mappings in the accepted ledger. The final compiler build identifies
49593da724e172838bd65b9eb48b2a3f11334522; runner fixes identify c3455e606. Compiler/provider bytes
are unchanged between these revisions. Cached CMake version metadata was refreshed before validation.

Upstream now rejects compute-stage mip-count queries (53e0e2b58656aa492ef6feaff2083ba3230a962f)
and multisample texture types on CUDA (49d32d96847de09b096daacec7b48bcdc4dbd8f1). All six affected
cells were already unresolved. Their old failure records remain history; no support is claimed.
The census classifier formerly mistook render-test's EXPECTED/ACTUAL wrapper for GPU execution.
It now recognizes explicit compiler errors before output comparisons, retaining NVVM preflight and
provider precedence. The old abort-only exception is removed. Thirty contract cases pass, and
replaying 3,426 old/new raw logs changes only the three mislabeled multisample classifications.
A fresh full checkpoint follows this runner change; no compiler or oracle was modified.

The final full checkpoint ran1713 cells;1673 passed,39 retained known gaps, and one NVRTC compile
failed while deleting `default_program.pch`. The failed layout-optional-field cell was correct in
the earlier full run. Three predeclared serial rounds then passed all three modes (9/9), with the
same five-field outcomes as260. The accepted record is explicitly composite: only that NVRTC outcome
comes from supplemental evidence, and the original failure/comparison remain a validation incident.
Parallel NVRTC PCH reliability remains unresolved. Automatic caching and shared default program names
suggest a file-lifecycle collision; serial success does not prove the collision or fix it. No cache
option, shader or oracle was changed. The effective1713-cell inventory retains1674 correct,
39 unresolved and18 resolved histories; six upstream rejection transitions are not new resolutions.

Units pass1086 with13 existing skips; semantics pass1170 with78 skips. Exact identity audits preserve
common statuses and record37 new unit registrations,120 semantic additions, the two path-query API
replacements and callable directive remapping. The new semantic skip is the unavailable Linux dx11
variant; CPU/LLVM/CUDA variants pass. Focused coverage passes29, including all10 callable replacements;
runtime smoke4, toolkit18 and material compile/assembly6 pass. The optimized AST proof covers705 tags,
497025 pairs,639 concrete and66 abstract tags with zero failures. No matching Debug proof is claimed.
Runner contracts pass13 results cases,6 discovery cases,14 complex cases (one existing skip), and
30 census-classification subcases. Raw gates and all failed/interrupted attempts remain under
`build/nvvm-maintenance` and `build/nvvm-results/2026-09-26-integration`.

The helper inventory retains upstream immutable-address classification and removes the redundant
branch helper; retains the held-symbol path override and existing V4 coexistence discovery; and
replaces the abort-only classifier exception with general explicit compiler-error precedence.
Each change has layer-specific coverage described above. The optional path interface's public API
change is upstream behavior; provider ABI42 is unchanged. Accepted262 authorizes measurements next,
not another implementation slice.
