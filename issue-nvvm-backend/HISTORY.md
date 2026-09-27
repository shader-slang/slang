# NVVM evidence navigation

Historical files retain their original names and evidence identities. Git history preserves prior
STATUS/WORKFLOW wording; accepted260 is commit `e673f646d0b493d7b87d1bb7c078c483595d170f`.
The current handoff belongs in [STATUS](STATUS.md), not in historical reports. The latest
[results package263](results/2026-09-26/README.md) includes refresh commands and presentation figures.

| Topic                                                       | Stable entry points                                                                                                                                                                                                                                                                                                                                                                                  |
| ----------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Current compiler capability, outcomes and failure histories | [validation277](runtime-validation.slice-277.json), [PCH277](report.slice-277-pch-ownership.md), [records270](report.slice-270-fp8-aggregate.md), [corpus269](report.slice-269-corpus-enumeration.md), [borrowed vectors268](report.slice-268-borrowed-vector-storage.md), [helper optimization267](report.slice-267-receiver-snapshot.md), [integration262](report.slice-262-master-integration.md) |
| Runtime inventories                                         | [frozen v1](census.slice-195.tsv), [discovery](discovery-corpus.manifest.tsv)                                                                                                                                                                                                                                                                                                                        |
| Material workload contract                                  | [manifest](complex-corpus.manifest.json), [fixed quality subset](quality-corpus.manifest.json)                                                                                                                                                                                                                                                                                                       |
| Recent BF16 local storage                                   | [report256](report.slice-256-bf16-local-vectors.md), [report259](report.slice-259-bf16-local-records.md)                                                                                                                                                                                                                                                                                             |
| BF16 physical layout research and correction                | [research253](report.slice-253-bf16-storage-layout.md), [report254](report.slice-254-bf16-cuda-layout.md), [research255](report.slice-255-bf16-physical-storage.md)                                                                                                                                                                                                                                  |
| Material timing protocol and profile                        | [profile272](report.slice-272-material-profile.md), [timing272](timing-evidence.slice-272.json), [research257](report.slice-257-material-profile.md), [timing257](timing-evidence.slice-257.json)                                                                                                                                                                                                    |
| Accepted class-metadata optimization                        | [report252](report.slice-252-ast-class-lookup.md), [timing252](timing-evidence.slice-252.json)                                                                                                                                                                                                                                                                                                       |
| Discarded getter-visibility experiment                      | [research258](report.slice-258-ast-context-getter.md), [timing258](timing-evidence.slice-258.json)                                                                                                                                                                                                                                                                                                   |
| Known semantic boundaries                                   | [column-major247](report.slice-247-column-major.md), [texture248](report.slice-248-texture-contract.md)                                                                                                                                                                                                                                                                                              |

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

- Implementation270: [internal FP8/BF16 records](report.slice-270-fp8-aggregate.md),
  [validation](runtime-validation.slice-270.json), [plan](plan.slice-270-fp8-aggregate.md).
  Original dynamic dispatch now passes NVVM O0/O3;1,738 other outcomes and all576input hashes are
  preserved. Units add one pass; all full gates and actual aggregate-phi/raw-bit controls pass.
  Flat internal values and qualified local storage remain separate from external/resource roles.
  Development loop resumed and continuing; notifications skipped at the user's request.

- Research271: [four language interactions](report.slice-271-language-breadth.md),
  [evidence](research-evidence.slice-271.json), [plan](plan.slice-271-language-breadth.md).
  Twelve focused cells pass across switch fallthrough, lambda capture, tuple mutation and defer/typed
  errors. Independent actual-output inspection supplements the weak tuple CHECK. No compiler or
  inventory change; accepted270 and its failure histories/cadence remain authoritative. Loop continues.

- Research272: [current material attribution](report.slice-272-material-profile.md),
  [timing evidence](timing-evidence.slice-272.json), [plan](plan.slice-272-material-profile.md).
  42 exact controls,139 units,132 compiles and66 assemblies pass; full accepted layout restored.
  NVVM O3 evaluation/sample medians1362.80/1454.46ms; semantic checking385–397ms motivates the next
  call-path/profile investigation. No retained compiler change or new correctness baseline; loop active.

- Research273: [semantic call paths](report.slice-273-semantic-profile.md),
  [evidence](research-evidence.slice-273.json), [plan](plan.slice-273-semantic-profile.md).
  16 exact-output profiles yield982 stacks; generic overload/inheritance recur in912-stack sensitivity
  too. Existing caches remain canonical; concrete key/outcome evidence is needed before an optimization.
  Zero reduction variants or compiler changes; accepted270 remains unchanged and the loop continues.

- Research274: [inheritance-cache outcomes](report.slice-274-inheritance-observation.md),
  [evidence](research-evidence.slice-274.json), [plan](plan.slice-274-inheritance-observation.md).
  All8 material runs compute6230canonical keys once; no repeated-key optimization/reducer justified.
  42controls/403resource rows and exact1100unit/1248semantic identities pass. Temporary observer removed,
  accepted270 layout restored exactly; separate local audits used after reviewer-thread limit. Loop active.

- Research275: [PCH ownership reproduction](report.slice-275-pch-reproduction.md),
  [evidence](research-evidence.slice-275.json), [plan](plan.slice-275-pch-reproduction.md).
  Shared16 yield6passes/8reuse failures/2crashes; serial8/private16 pass. Exact262 deletion signature
  remains open. No production change; accepted270 unchanged. Next namespace/lifetime qualification.

- Research276: [PCH directory lifetime](report.slice-276-pch-lifetime.md),
  [evidence](research-evidence.slice-276.json), [plan](plan.slice-276-pch-lifetime.md).
  All20 processes/100compiles pass, including held-library retirement and surviving-owner reuse.
  No production change; stable percompiler namespace selected for actual adapter qualification.

- Correctness277: [automatic-PCH owner isolation](report.slice-277-pch-ownership.md),
  [validation](runtime-validation.slice-277.json), [completed plan](plan.slice-277-pch-ownership.md).
  Fixed40-process comparison passes with40 private retired namespaces; all1740 main outcomes and
  native identities preserved, three passing unit additions. Material PTX/cubin/resources exact.
  Full277/targeted233/cadence0. Root local audits plus author self-review due thread limit.
  Loop continues with accessor/generic language probes; Slack skipped.

- Research278: [accessor/generic interactions](report.slice-278-accessor-generics.md),
  [evidence](research-evidence.slice-278.json), [completed plan](plan.slice-278-accessor-generics.md).
  Six unchanged sources pass18 cells/72 independently checked words. Main corpus and277 identity
  unchanged; full277/targeted233/cadence0 inherited. No compiler change. Next: nested FP8/BF16 probe.

- Correctness279: [nested record admission and padding-preserving stores](report.slice-279-nested-records.md),
  [validation](runtime-validation.slice-279.json), [completed plan](plan.slice-279-nested-records.md).
  New nested FP8/BF16 composition exposed valid LLVM miscompiled by libNVVM O3; accepted277 integer
  control also fails. Provider splits direct nested struct stores using canonical offsets/alignment.
  Focused14native/18GPU and full1740 preservation pass; six new units, old native identities exact.
  Material O3/NVRTC artifacts exact; O0 store expansion independently reviewed, resources unchanged.
  Full279/targeted233/cadence0. Reused independent reviewer; next bounded gate is array-nested stores.
