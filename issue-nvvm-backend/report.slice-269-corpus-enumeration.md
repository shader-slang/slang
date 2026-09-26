# Repair compute-corpus enumeration and broaden language coverage

## Motivation

The native test harness accepts `// TEST` and extra comment slashes, but the corpus runners required
`//TEST`. That hid the multidimensional CUDA lane-index test and 38 historically excluded neural
files from enumeration. The earlier audit also found five eligible CUDA sources outside the main
corpora. Meanwhile, the selected tests were concentrated in compute, intrinsics and dynamic dispatch,
with little direct selection from several core-language directories.

A second indexing discrepancy matters when preserving tests. Consider these authored directives:

```slang
//DISABLE_TEST:SIMPLE:
//DIAGNOSTIC_TEST:SIMPLE:
// TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
```

The last test has native index2 because disabled and diagnostic entries occupy harness indices.
The old runners counted only strict active TEST lines, potentially choosing the wrong indexed oracle.

## Proposed solution

Use one directive enumerator in the census runner and reuse it for discovery selection and source
mirrors. Follow the native harness's case-sensitive command and comment-prefix rules; count native
indices while excluding disabled, diagnostic and whole-file-ignored tests from compute selection.
Preserve every existing selected contract and frozen ID, then qualify nine additional cases against
their existing independent output checks at NVRTC O3 and NVVM O0/O3.

## Change summary

The two Python runners share directive interpretation; their contract tests cover the missed prefixes,
indexing and expected-file selection. Three discovery ordinals migrate to preserve their original
selected directives. Frozen v1 remains byte-for-byte unchanged. Discovery expands through five CUDA
backfills and four core-language cases, with rationale retained in its manifest. This report, plan,
accepted record and navigation retain results; raw evidence stays under `build/nvvm-corpus269`.
The compiler and provider remain exactly the accepted268 binaries; this slice makes no compiler change.

## Concepts and vocabulary

A source-test ordinal is the authored harness-list index, including disabled and diagnostic entries.
A frozen CUDA ID numbers selected CUDA contracts within a source; changing the native index must not
change that ID or its selected contract. A mode cell is one selected case run at one backend and
optimization setting. Multiple cases can share a source file; neither count measures semantic coverage.

## Process report

The helper inventory comprises one enumerator, an enabled compute-comparison predicate, and mirror
source filtering using that same enumeration. No second parser or compatibility ordinal is retained.
`_gatherTestsForFile` remains the reference for prefix, disable and whole-file-ignore behavior;
`_gatherTestOptions` owns the native directive structure. Slang-test still owns category/API gating
and execution. The census regressions fail before the repair and pass afterward. An initial discovery-test
pre-fix run failed because the helper signature changed; that TypeError is retained but excluded
as behavioral evidence. A separate reproduction uses the old signature and demonstrates rejection
of a valid native ordinal, while the repaired selector accepts it.

The preservation audit compares all 452 frozen and 119 prior discovery selections by ID, source,
selected line, categories, command, arguments, source hash and resolved expected-file path/hash.
All contracts are unchanged. Native indices change for 83 frozen cases; the frozen inventory itself
is untouched. Three discovery indices change: texture-subscript-multisample2→3,
non-square-column-major0→1 and op-assignment-unify-mat0→1. Each still selects exactly its original
source line and oracle. This includes the known column-major mismatch, which must remain visible.

All nine prescribed additions pass27/27 focused GPU cells with unchanged sources and independent
oracles. Five backfills cover aggregate snapshots/resources, multidimensional wave indexing,
wave-prefix bit counts and forward/reverse differentiation of shaped numerics. Four breadth additions
cover these concrete language cases:

| Existing source                           | Demonstrated surface                                                              |
| ----------------------------------------- | --------------------------------------------------------------------------------- |
| `interfaces/conjunction-assoc-type.slang` | Conjunctive associated-type constraints through generic reads and mutation        |
| `generics/assoc-type-default-init.slang`  | Dependent associated-type initialization under dynamic dispatch                   |
| `enums/enum-array-indexing.slang`         | Enum values indexing local arrays without casts                                   |
| `bitfield/default-init-mixed.slang`       | Empty/partial initialization of mixed 32/64-bit backing words and ordinary fields |

These four paths are under `tests/language-feature`. Dynamic-dispatch scaffolding and default
bitfield layout are intact; the separate MSVC-packing variant was not substituted or claimed.

The first nine-case attempt passed eight cases in all modes. The numerics backfill failed all three
with E20001 because this build lacked its existing standard numerics modules. This was a packaging
prerequisite, not a backend failure. After preserving the accepted268 layout, a dry run of the
existing build graph showed only source/header staging and four module-generation commands using
the accepted bootstrap/core. An ignored wrapper retained that configured graph without triggering
CMake regeneration and compiler-version drift. The successful build added exactly four serialized
modules, changing/removing no preexisting installed artifact. Their sources, bootstrap/core and graph
hashes are retained; module identity is fresh269 while compiler/provider identity remains268.
The unchanged nine cases then passed every mode. The original failed attempt remains evidence,
and RESULTS now documents the prerequisite. This qualifies the fixed backfill, not a broader
autodiff pilot. A no-op formatter PATH failure is also retained; final syntax/contracts and supported
formatting invocation pass.

The full checkpoint preserves all 1,713 old outcomes and adds 27 correct cells: 1,740 total,
1,701 correct, 39 unchanged gaps and 18 retained resolved histories. All 567 old source hashes
remain exact; nine new hashes are appended explicitly. The original comparison remains
review-required solely for additions; the separate maintained comparison with `--allow-additions`
passes. Acceptance reviews those additions explicitly rather than rewriting the original result.
Units retain all 1,099 identities (1,086 pass/13 skip); semantic checks retain all 1,248
(1,170 pass/78 skip). Runtime4, toolkit18 and material compile/assembly6 pass. Runner contracts
pass: census3, discovery7, complex14 plus one skip, results16. Independent review confirms exact
old outcomes, inputs, histories, artifact identities and new results. The inherited automatic-PCH
reliability incident remains open, and material GPU execution is not claimed.

The refreshed static inventory contains 4,639 active files and 10,834 explicit TEST/DIAGNOSTIC_TEST
directives, including 1,852 compute-comparison files and 795 files with explicit CUDA comparisons.
The expanded main corpus has 580 cases/576 files/1,740 mode cells: 31.1% of compute-comparison files
and 517/795 (65.0%) explicit CUDA files. All 516 historically policy-eligible CUDA sources are now
represented: 514 in the main corpus and two in focused268 fixtures. Three historically excluded
FP8 sources already selected in earlier slices explain 517 rather than 514. The parser audit finds
no remaining explicit-CUDA enumeration misses. These are selection counts, not passing-coverage claims.
These counts describe authored tests before host/category filtering and synthesized variants.
The two new268 fixtures account for the increase from the previous audit; they remain separate
focused tests outside this manifest expansion. Directory membership is a guide to underrepresented
regions, not a language-feature coverage percentage. No new performance measurement or material GPU
execution is claimed. The finite sequence stops after its reviewed local commit; both Slack notifications are skipped at the user's request.
