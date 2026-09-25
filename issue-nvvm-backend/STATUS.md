# NVVM backend status

Read [WORKFLOW.md](WORKFLOW.md) before resuming. The autonomous loop is authorized; the parent owns
independent acceptance and local commits. No push is authorized.

## Slice252 is accepted

The bounded constructor-visibility change passes all performance and correctness gates. Every
semantic median falls 16.86–18.89%, summed wall medians 7.35527%, and all 12 per-round wall comparisons
improve. All 264 PTX outputs and 24 support cubins exactly match 250/251. Read
[plan252](plan.slice-252-ast-class-lookup.md), [report252](report.slice-252-ast-class-lookup.md),
[timing252](timing-evidence.slice-252.json) and [validation252](runtime-validation.slice-252.json).

One generated table remains authoritative. Its incomplete internal declaration and independently
deduced count preserve pointer identity, bounds, factories/destructors, casts and metadata. Matching
Debug/optimized proofs pass 702 tags,492804 pairs,636 objects and66 abstract classes, plus specific
Debug -1/702 assertions. The table remains hidden/local, with unchanged exports and NatVis.

- Runtime smoke4, full units1047pass/13skip, semantic1052pass/77skip, toolkit18 and contracts6 pass.
  Exact1060 unit/1129 semantic outcomes match before, including all513 relevant250 passes.
- Full frozen1356 cells/1347 correct and discovery345 cells/315 correct preserve every five-field
  outcome:1701 cells,1662 correct,39 unresolved and18 resolved histories. No losses or additions.
- Final6 material compiles/assemblies are exact250/251. All compiler sources,12 artifacts and563
  inputs remain measured identity. Only a proof-driver help example changes after timing, with an
  exact doc-only hash record.120 final snapshots also retain unchanged NatVis.

Compiler SHA256 `10ffeb3246d56c9a835b1cd606e35a1a2cd6c8f9fcb3f6bfef36fed26413ea7b`;
provider unchanged `5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`, ABI41.
An initial audit failure is exactly the expected+7 generated FIDDLE line-macro shift; a later local
loader-oracle suffix typo is retained. Neither changed production code or retried timing.

Latest accepted implementation/full checkpoint252, targeted233 and implementation cadence0.
Rolling implementation history is249 FP8 scalar transport,250 BF16 literal correctness and252
material compile-time improvement. Independent parent acceptance verifies all timings, exact corpus
outcomes and histories, four proofs,120 snapshots,4224 indexed artifacts and226 compact references.
Material runtime contracts remain unavailable. No push or system/driver change occurred.

## Research251 is accepted

Fresh accepted250 material measurements preserve all132 PTX and66 cubin outputs, with108 measured
compiles/24 warmups and54 measured assemblies/12 warmups. SemanticChecking medians484.74–488.73ms
and fresh wall1407.13–1601.66ms are current observations, not a paired speedup over246. Three separate
completed profiles preserve PTX and collect273 qualitative snapshots. Read
[plan251](plan.slice-251-material-profile.md), [report251](report.slice-251-material-profile.md) and
[timing251](timing-evidence.slice-251.json).

The repeated class-metadata path supports one future prototype: make the existing
SyntaxClassBase(ASTNodeType) constructor visible for inlining, preserving one generated table and
all cast semantics. The table's current translation-unit linkage requires a deliberate internal
shared declaration and count/pointer/assertion proof. Preserve the existing `slang.natvis` table
references when selecting linkage. No optimization is implemented in251; the old
predicate is not consistently the dominant leaf, and allocation samples have heterogeneous consumers.
119 source/generated/test,12 artifacts and563 runtime inputs remain exact250. No fresh GPU runtime
cells; material bindings/textures/LUT/input/output contracts remain unavailable.

Latest accepted implementation/full checkpoint250, targeted233 and implementation cadence0 remain
unchanged. Rolling implementation history remains246 material compile-time,249 FP8 scalar transport,
250 BF16 correctness. Independent parent acceptance verifies all sample inventories, statistics,
profiles, identities,579 raw artifacts and134 primary snapshots; an additional parent snapshot
records the debugger consumer. Raw research lives under `build/nvvm-loop/slice-251-material-profile`.

## Slice250 is accepted

The bounded BF16 negative canonical literal correction passed independent acceptance on provider
ABI 41. Read [plan250](plan.slice-250-bf16-signed-literals.md),
[report250](report.slice-250-bf16-signed-literals.md), and
[validation250](runtime-validation.slice-250.json).

The existing emitter now sends the checked uint16 BF16 encoding as its signed16 bit representation
to the integer-constant builder. `pack(BFloat16(-1.25f))` returns49056 (0xbfa0). Twelve exact values
cover both signs of1.25, zero, minimum/maximum subnormal, minimum normal and maximum finite.
Canonical producer, rounding/overflow, provider API, ABI 41 and BF16 storage/vector roles are unchanged.

Final source is accepted249 commit `c952717c4baa37cbe73d4eacdd5d011cce6a24c5` plus the250 diff:
119 source/generated/test snapshots,12 artifacts,563 runtime inputs. Compiler library SHA256
`ae6fe92965ed066a02ff9eab550093294b916c6b29f122f02e8a3f2ab8b74f60`; unchanged provider SHA256
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`.

- Fresh runtime 4, focused 18, units 513 with one unchanged skip, toolkit 18 and runner contracts6 pass.
  All513 prior unit pass identities are retained, including doubleSourceLiteralsRoundTrip.
- Full frozen 452/1356 preserves 1347 correct and 9 unresolved; discovery 115/345 preserves all 342
  old outcomes and adds 3 correct. Combined1701cells/1662correct/39unresolved. All1698 old outcomes
  match exactly in classification, return code, execution counts, diagnostic and canonical shape.
- All39 unresolved histories and 18 resolved histories survive. Texture/column-major wrong-output
  baselines are unchanged. No missing/extra/duplicate requested cells.
- Whole-buffer audit checks 3444 final words and 12 before words with zero mismatches. Final IR and
  PTX retain all 12 intended canonical literal signs and exact helper argument bits.
- Every material/backend/optimization cell is reassessed: complex 6 compile/assembly checks pass.
  No material runtime or performance claim; bindings/textures/LUT/input/output contract unavailable.

Latest accepted implementation and full checkpoint is250; latest targeted acceptance is233;
implementation cadence0. Rolling implementation history is246 material compile-time,249 FP8
scalar transport,250 BF16 correctness. Independent parent acceptance verifies every old outcome,
failure history, exact output, final IR/PTX, unit ID and identity, plus119 source snapshots and2132
indexed artifacts. Correctness priority explains this slice; reconsider material-driven work next.

## Next bounded action

Qualify the CUDA BF16 vector storage-layout boundary recorded by research240 on the accepted252
compiler. BF4's reported alignment8 differs from the actual CUDA prelude alignment2; establish an
observable source-level reflection/storage consequence with independent byte-layout and output
oracles before choosing a producer-side repair. Cover BF2/BF3/BF4 controls, wrapped fields, array
stride and neighboring scalar types. Keep this bounded research separate from new NVVM storage
admission and from the original dynamic-dispatch fixture's other unsupported shapes.

This known producer mismatch has a clearer correctness contract than general FP8 runtime conversion
(shared overflow differs from CUDA SATFINITE) or texture queries (recorded248 contract/toolkit gaps).
Further material optimization needs fresh profiling on252; all6 material cells already compile and
assemble, and runtime bindings/textures/LUT/input/output contracts remain unavailable. Preserve the
original texture/column-major failures and all existing input/oracle identities.

## Accepted historical evidence

[Accepted249](runtime-validation.slice-249.json) retains qualified FP8 scalar byte transport and
all-byte/cross-format/phi supplemental replays. Those supplements remain inherited with their249
identities; the full250 corpus and listed focused/gate results are fresh. [Research248](report.slice-248-texture-contract.md)
and [research247](report.slice-247-column-major.md) retain original failure histories and isolated
semantic controls. [Implementation246](runtime-validation.slice-246.json) retains the AST predicate
inlining and material timing evidence. [Implementation244](runtime-validation.slice-244.json)
retains finite/subnormal FP8 producer repairs; shared overflow policy remains separate.

## Environment and evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86,
LLVM14, providerABI41, matching RelWithDebInfo. Inspect/source `build/nvvm-loop/slice-203-env.sh` and
follow the local slang-build skill. At most4 CPU workers, sequential GPU suites and 30-minute bounds.

Raw roots `build/nvvm-loop/slice-252-before`, `slice-252-prototype` and `slice-252-after` retain
matched baseline/candidate layouts, all timing attempts, proof and runtime evidence, final snapshots,
commands, parent audits and the closed worker artifact index.
No GPU loss, driver/system change, reboot, push or worker commit occurred.
