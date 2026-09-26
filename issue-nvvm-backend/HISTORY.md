# NVVM evidence navigation

Historical files retain their original names and evidence identities. Git history preserves prior
STATUS/WORKFLOW wording; accepted260 is commit `e673f646d0b493d7b87d1bb7c078c483595d170f`.
The current handoff belongs in [STATUS](STATUS.md), not in historical reports. The latest
[results package263](results/2026-09-26/README.md) includes refresh commands and presentation figures.

| Topic                                                       | Stable entry points                                                                                                                                                                                                                                                                                    |
| ----------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Current compiler capability, outcomes and failure histories | [validation269](runtime-validation.slice-269.json), [corpus269](report.slice-269-corpus-enumeration.md), [borrowed vectors268](report.slice-268-borrowed-vector-storage.md), [helper optimization267](report.slice-267-receiver-snapshot.md), [integration262](report.slice-262-master-integration.md) |
| Runtime inventories                                         | [frozen v1](census.slice-195.tsv), [discovery](discovery-corpus.manifest.tsv)                                                                                                                                                                                                                          |
| Material workload contract                                  | [manifest](complex-corpus.manifest.json), [fixed quality subset](quality-corpus.manifest.json)                                                                                                                                                                                                         |
| Recent BF16 local storage                                   | [report256](report.slice-256-bf16-local-vectors.md), [report259](report.slice-259-bf16-local-records.md)                                                                                                                                                                                               |
| BF16 physical layout research and correction                | [research253](report.slice-253-bf16-storage-layout.md), [report254](report.slice-254-bf16-cuda-layout.md), [research255](report.slice-255-bf16-physical-storage.md)                                                                                                                                    |
| Material timing protocol and profile                        | [research257](report.slice-257-material-profile.md), [timing257](timing-evidence.slice-257.json)                                                                                                                                                                                                       |
| Accepted class-metadata optimization                        | [report252](report.slice-252-ast-class-lookup.md), [timing252](timing-evidence.slice-252.json)                                                                                                                                                                                                         |
| Discarded getter-visibility experiment                      | [research258](report.slice-258-ast-context-getter.md), [timing258](timing-evidence.slice-258.json)                                                                                                                                                                                                     |
| Known semantic boundaries                                   | [column-major247](report.slice-247-column-major.md), [texture248](report.slice-248-texture-contract.md)                                                                                                                                                                                                |

Earlier slices remain discoverable by number: `plan.slice-N-*.md`, `report.slice-N-*.md`,
`runtime-validation.slice-N.json`, `semantic-evidence.slice-N.json`, and `timing-evidence.slice-N.json`.
Use `rg --files issue-nvvm-backend` and filter the topic or slice number. The latest accepted compact
ledger preserves first-known and resolved histories with their original evidence references; raw
`build/` directories are local evidence, not prerequisites for comparing durable old outcomes.

- Research264: [material timing/resources](report.slice-264-material-attribution.md),
  [evidence](timing-evidence.slice-264.json), [reproducible probes](experiments/material-attribution/README.md).
  No compiler change; aggregate optimization remains a future investigation.

- Results265: [explanatory Monday package](results/2026-09-26-attribution/README.md),
  [report](report.slice-265-attribution-package.md), [plan](plan.slice-265-attribution-package.md).
  Reusable optional stage attribution; accepted compiler unchanged; finite follow-up stopped.

- Research266: [differential material reduction](report.slice-266-material-reproducer.md),
  [evidence](research-evidence.slice-266.json), [runnable fixtures](experiments/material-reproducer/README.md).
  Constant NVRTC/NVVM exponential counts 0/3; branchless 0/0; 18 GPU cells pass. No compiler change;
  exact downstream pass cause remains open. Bounded experiment complete, loop stopped.

- Implementation267: [helper value parameters](report.slice-267-receiver-snapshot.md),
  [validation](runtime-validation.slice-267.json), [source counterfactuals](experiments/receiver-snapshot/README.md).
  Full correctness preserved; material O3 exponentials 6→0, stack 784→0, fewer registers, paired
  compile medians 1.45–2.35% lower. No spills added; O0 stacks grow 320 bytes with larger modules.
  Earlier failed prototypes and the final reviewed resource flag remain recorded. Loop stopped.

- Correctness268: [borrowed native vectors](report.slice-268-borrowed-vector-storage.md),
  [validation](runtime-validation.slice-268.json), [plan](plan.slice-268-borrowed-vector-storage.md).
  Readonly helper storage no longer misclassified as compact;6newGPU+8assembly cells pass,
  full1,713 outcomes preserved. Its finite269 follow-up is also complete; the general loop stays stopped.

- Corpus269: [enumeration and breadth](report.slice-269-corpus-enumeration.md),
  [validation](runtime-validation.slice-269.json), [plan](plan.slice-269-corpus-enumeration.md).
  Shared native directive enumeration preserves all prior contracts; nine additions bring the main
  corpus to 580 cases/576 sources/1,740 cells. All 27 new cells pass and all 1,713 old outcomes are
  unchanged. Discovery reaches128; four required numerics modules were supplied without changing
  compiler/provider bytes. Finite sequence complete, notifications skipped at user request, loop stopped.
