# Start a fresh NVVM session

Current full baseline: [validation277](runtime-validation.slice-277.json),
[PCH ownership report](report.slice-277-pch-ownership.md) and
[completed plan277](plan.slice-277-pch-ownership.md).
The user resumed the development loop on 2026-09-26. Continue bounded reviewed local commits under
WORKFLOW until a recorded stopping condition. Skip Slack; no push or system changes.

1. Read STATUS, WORKFLOW and RESULTS; inspect HEAD, working changes and dependency pins.
2. Qualified277 layout is installed at `build/RelWithDebInfo`. Verify all37 runtime provenance entries,
   all576 main inputs and22 pins. Actual compiler aa1fe42e/version293-g9210ef5a1 comes from recorded
   source9210ef5a1 plus patch30e61108; provider ABI42/hash fbef1a9e is unchanged. Do not treat a launcher
   hash or later commit number as the compiled identity.
3. Builds follow `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`, native tools and RESULTS
   prerequisites. Previous270's100-entry installed layout is preserved under
   `build/nvvm-pch-ownership277/accepted270-layout`. Use fresh ignored artifact roots.
4. Limit builds/jobs to four CPU workers, units to two servers, and gates to30 minutes. Serialize
   builds/GPU/suites/benchmarks. Preserve failed attempts and exact comparisons; no retries hiding losses.
5. Full277 preserves580 cases/576 sources/1740 cells:1703 correct,37 unresolved,20 resolved histories.
   Units1090pass/13skip preserve1100 old IDs plus3 additions; semantics1170pass/78skip preserve1248 IDs.
   Runtime4/toolkit18/material6 pass. Material PTX/cubin/resources equal270. Last full277/targeted233,
   implementation cadence0. Frozen195 is immutable; discovery remains at128 sources.
6. Research275 reproduced8 native failures+2SIGSEGV among16 shared-cwd processes, with24 controls
   passing. Lifecycle276 qualified20 processes/100 direct NVRTC compiles. Implementation277 passes the
   same40-process comparison with40 private retired namespaces and120 PCH creations. Actual-adapter
   tests cover same-owner reuse, named diagnostics, surviving owners/external library refs and explicit
   caller directories. Exact262 deletion signature is retained without claiming causal reproduction.
7. [Research278](report.slice-278-accessor-generics.md) qualifies six existing accessor/generic sources
   outside the main corpus:18 cells and72 independent output words pass, with unchanged277 identities.
   No compiler defect found. Next: probe nested FP8/BF16 local-record composition, a documented270
   boundary. Establish a small independent output oracle and canonical IR trace before implementation.
   Preserve current leaf/value/storage/pointer domains and material acceptance obligations.
8. Delegation recently hits thread limits. Use a fresh bounded worker when available; otherwise record
   reuse and separate root local audits. Author self-review is not independent-agent review.

[HISTORY](HISTORY.md) owns older evidence. FP8/BF16 record scope remains in
[the contract](../docs/design/nvvm-substandard-record-contract.md) and validation270; focused268/270
fixtures remain outside the main corpus. Breadth271's12 cells passed. Material272 sampling and273/274
semantic investigation are complete; each of6230 canonical inheritance keys was computed once, so
that cache-duplication lead is closed. Earlier267/272 timing retains original identity and does not
measure277. No new performance claim or material GPU execution; binding/input/output contracts remain
unavailable. Windows private-directory acquisition and its failure diagnostic are source-audited only.
