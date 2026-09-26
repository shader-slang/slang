# NVVM evidence navigation

Historical files retain their original names and evidence identities. Git history preserves prior
STATUS/WORKFLOW wording; accepted260 is commit `e673f646d0b493d7b87d1bb7c078c483595d170f`.
The current handoff belongs in [STATUS](STATUS.md), not in historical reports. The latest
[results package263](results/2026-09-26/README.md) includes refresh commands and presentation figures.

| Topic                                                       | Stable entry points                                                                                                                                                 |
| ----------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Current compiler capability, outcomes and failure histories | [validation262](runtime-validation.slice-262.json), [integration262](report.slice-262-master-integration.md), [FP8 support260](report.slice-260-fp8-widening.md)    |
| Runtime inventories                                         | [frozen v1](census.slice-195.tsv), [discovery](discovery-corpus.manifest.tsv)                                                                                       |
| Material workload contract                                  | [manifest](complex-corpus.manifest.json), [fixed quality subset](quality-corpus.manifest.json)                                                                      |
| Recent BF16 local storage                                   | [report256](report.slice-256-bf16-local-vectors.md), [report259](report.slice-259-bf16-local-records.md)                                                            |
| BF16 physical layout research and correction                | [research253](report.slice-253-bf16-storage-layout.md), [report254](report.slice-254-bf16-cuda-layout.md), [research255](report.slice-255-bf16-physical-storage.md) |
| Material timing protocol and profile                        | [research257](report.slice-257-material-profile.md), [timing257](timing-evidence.slice-257.json)                                                                    |
| Accepted class-metadata optimization                        | [report252](report.slice-252-ast-class-lookup.md), [timing252](timing-evidence.slice-252.json)                                                                      |
| Discarded getter-visibility experiment                      | [research258](report.slice-258-ast-context-getter.md), [timing258](timing-evidence.slice-258.json)                                                                  |
| Known semantic boundaries                                   | [column-major247](report.slice-247-column-major.md), [texture248](report.slice-248-texture-contract.md)                                                             |

Earlier slices remain discoverable by number: `plan.slice-N-*.md`, `report.slice-N-*.md`,
`runtime-validation.slice-N.json`, `semantic-evidence.slice-N.json`, and `timing-evidence.slice-N.json`.
Use `rg --files issue-nvvm-backend` and filter the topic or slice number. The latest accepted compact
ledger preserves first-known and resolved histories with their original evidence references; raw
`build/` directories are local evidence, not prerequisites for comparing durable old outcomes.
