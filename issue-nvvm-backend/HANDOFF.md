# Start a fresh NVVM session

Current full baseline: [validation279](runtime-validation.slice-279.json),
[nested record/store report](report.slice-279-nested-records.md) and
[plan279](plan.slice-279-nested-records.md). The user resumed the development loop on 2026-09-26.
Continue bounded reviewed local commits under WORKFLOW until a recorded stopping condition.
Skip Slack; no push or system changes.

1. Read STATUS, WORKFLOW and RESULTS; inspect HEAD, working changes and dependency pins.
2. Qualified279 layout is installed at `build/RelWithDebInfo`. Verify all37 runtime identities,
   all576 main inputs and22pins. Actual compiler9e013b2c/version295-g0043e8d17 and provider a861b242/ABI42
   come from source0043e8d17 plus patch62ae6473. A later commit number is not the compiled identity.
3. Builds follow `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`, native tools and RESULTS
   prerequisites. Verified277 layout is preserved under `build/nvvm-nested-records279/accepted277-layout`;
   older270 recovery remains under `build/nvvm-pch-ownership277/accepted270-layout`. Use fresh artifact roots.
4. Limit builds/jobs to4CPU workers, units to2servers, gates to30minutes. Serialize builds/GPU/suites/
   benchmarks. Preserve failed attempts and exact comparisons; no retries hiding losses.
5. Full279 preserves580cases/576sources/1740cells:1703correct,37unresolved,20resolved histories.
   Units1096pass/13skip preserve1103old IDs plus6new; semantics1170pass/78skip preserve1248IDs.
   Runtime4/toolkit18/material6 pass. Last full279/targeted233/cadence0. Frozen195 immutable; discovery128.
6. Nested FP8/BF16 admission uses canonical finite record trees and existing local ancestry. Candidate1
   exposed a libNVVM12.9 O3 wrong-output bug in valid nested stores; accepted277 integer control and
   standalone LLVM reproduce it. Provider splitting at direct struct boundaries fixes both exhaustive
   regressions. Focused14native/18GPU pass. Arrays stay opaque; all documented role boundaries remain.
7. Material NVRTC/O3 PTX/cubins are byte-identical277. TwoO0 artifacts differ in six functions per module;
   independent review finds expected field initialization/copy expansion, unchanged159/176 symbol sets
   and identical parsed resources. Original review-required comparison and acceptance decision retained.
   Material runtime lacks binding/input/output contracts; no equivalence or performance claim.
8. [Research280](report.slice-280-array-record-stores.md) and its [record](research-evidence.slice-280.json)
   add nine focused cells outside main corpus: seven pass; nested-record arrays fail NVRTC O3/NVVM O3.
   Isolated out-copy proves wrong child.first offsets in both PTX paths;277 replay proves preexisting.
   Original stage64 could be masked by stale data; use the separate diagnostic for out-copy evidence.
   Raw frozen sources/commands/outputs live under `build/nvvm-array-record-stores280`; installed279
   artifacts,8sources,2configs,576inputs and22pins are unchanged before/after research.
9. [Prototype281](report.slice-281-array-copy-prototypes.md) qualifies small standalone pointer memcpy/
   field-loop remedies on both backends and a typed NVVM noinline/optnone helper with callerO3.
   Seventeen effective cells:9correctGPU/2wrong-baselineGPU/6largecompile-only; six initial declaration
   failures retained. Raw root `build/nvvm-array-store-prototype281`. Pointer IR102/114instructions
   stays constant at65536elements; storage grows to786432/2359296bytes, with no largeGPU execution.
10. [Gate282](report.slice-282-ssa-copy-boundary.md) qualifies the small typed helper for constructed/
    earlier-load/phi SSA and offset1 storage/canaries. N17caller expansion and N65536O0 timeout under
    120seconds/4GiB reject the general method; O3 explicitly not run. No cutoff/fallback selected.
    Eight exact dispositions/raw sources at `build/nvvm-ssa-copy-boundary282`; accepted identities unchanged.
11. [Gate283](report.slice-283-array-store-alignment.md) changes only four/two whole-store alignment
    annotations4→1, with actualroots still4aligned. Both small candidates passO0/O3, controls remainwrong.
    N17both/N65536O0 compile; N65536O3 times out120s/4GiB. Rawroot
    `build/nvvm-array-store-alignment283`; no new authored ABI, but NVVM argument promotion remains.
12. Next: original-large baseline under identical bounds before attributing283timeout and choosing
    production scope. No general fix/cutoff selected. Accepted279 identities and280 production defects
    remain unchanged. This comparison is new evidence, not a retry of the failed candidate.
13. Prefer fresh bounded delegation when available;283 used fresh author and separate reused reviewer.
    Skip Slack, no push/system changes, continue the authorized loop.

[HISTORY](HISTORY.md) owns earlier PCH277, accessor/generic278 and material investigations.
[The record contract](../docs/design/nvvm-substandard-record-contract.md) separates new279 scope from
flat270 cache-order evidence. The inheritance-cache lead closed in274 without an optimization;
each6230canonical key was computed once. Earlier timing results keep their original identity.
