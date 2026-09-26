# Start a fresh NVVM session

Current accepted baseline: [267](runtime-validation.slice-267.json). The bounded
[helper-value optimization](report.slice-267-receiver-snapshot.md) is complete; the general development
loop is **stopped**. No next implementation is authorized. STATUS records the qualified source patch,
actual compiler library, acceptance counts and known gaps. Closing commit and Slack delivery live in
`build/nvvm-receiver-snapshot267/closeout.json`; check that before sending any completion notification.

1. Read STATUS and WORKFLOW. Distinguish a results refresh, bounded maintenance and explicit
   development resume. Do not turn an old report's candidate into an active task.
2. Inspect branch/HEAD, working changes, submodule pins and any active plan. Preserve unrelated work.
   The accepted267 layout is installed at `build/RelWithDebInfo`; accepted262 is backed up under
   `build/nvvm-receiver-snapshot267/baseline-layout`. Paths alone do not establish identity.
3. For a build, read the platform-specific `slang-build` skill. On this host it is at
   `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Native Linux uses native tools;
   WSL uses Windows tools per AGENTS. Refresh CMake's cached version metadata as RESULTS describes.
   The267 build deliberately retains its pre-commit version plus an exact qualified dirty patch.
4. Follow RESULTS for refresh/checkpoint commands. Use unique raw output roots, retain failures,
   cap CPU workers at four, use two unit servers, serialize GPU suites and isolate measurements.
5. Compare exact per-cell outcomes and compiler/provider/toolkit/cache/input identities. Keep all
   39 unresolved and 18 resolved histories. Accepted267 has a fresh full checkpoint, so the
   implementations-since-full cadence is zero. AST proof evidence remains inherited from262.
6. Complete the bounded plan, five-part report and one structured outcome record, then the authorized
   local commit and once-per-slice Slack notification under WORKFLOW. Check prior delivery first.
   Stop at the requested boundary; no push is implied.

Slice267 narrows eligible internal struct value parameters to directly extracted fields before
`deferBufferLoad`. It preserves original SSA snapshots, external signatures and unsupported field
interfaces, using NVVM's existing helper-value classifier. Both original material entries at O3 lose
six exponentials and their784-byte stack allocation. Registers fall67→52 and86→62; paired compile
medians fall1.45–2.35%. The36-cell quality subset is unchanged and no spills are added. At O0, entry
stacks grow320bytes and modules grow because existing helpers remain separate. This reviewed tradeoff
and the raw resource review flag are retained in the accepted ledger.

An explicit copied constref snapshot exposes a separate correctness issue: a float3-containing helper
struct is mistaken for compact parameter-group storage, while float4 succeeds. If a new bounded
correctness investigation is requested, start from267's retained minimal probe and audit
`_getNVVMStructFieldAddress` → `_getNVVMCompactParameterGroupVectorPointer`. Do not broaden this into
storage-layout or provider support work without a new plan. Material GPU execution still lacks its
binding/texture/LUT/input/output contract. Concurrent NVRTC automatic-PCH reliability remains open.

[Research266](report.slice-266-material-reproducer.md) and
[source counterfactuals267](experiments/receiver-snapshot/README.md) retain their accepted262 identities;
use that preserved compiler to reproduce the historical0/3 exponential difference. The accepted267
compiler now optimizes the original source too. Earlier instrumentation remains under
`build/nvvm-material-followup`. Historical [package263](results/2026-09-26/README.md) and
[attribution package265](results/2026-09-26-attribution/README.md) have not been refreshed; use their
commands only for a separately requested results refresh. [HISTORY](HISTORY.md) indexes prior evidence.
