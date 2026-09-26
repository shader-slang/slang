# Start a fresh NVVM session

Current baseline: [validation270](runtime-validation.slice-270.json),
[FP8/BF16 record report](report.slice-270-fp8-aggregate.md) and
[completed plan270](plan.slice-270-fp8-aggregate.md).
The user resumed the development loop on 2026-09-26. Continue bounded reviewed local commits under
WORKFLOW until a recorded stopping condition. Skip Slack notifications; no push or system changes.

1. Read STATUS, WORKFLOW and RESULTS; inspect HEAD, changes and pins before selecting a slice.
2. Qualified270 compiler/provider/module layout is installed at `build/RelWithDebInfo`. Verify all 37
   runtime provenance entries and 22pins, including the loaded compiler rather than only its launcher.
   Version metadata identifies the precommit source plus exact patch. Full gates used the pre-phi
   focused fixture; its equivalent single-return refinement passed separate fixture/raw GPU gates.
3. Accepted269 layout is preserved at `build/nvvm-fp8-aggregate270/accepted269-layout`. Builds follow
   `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`, native tools and RESULTS prerequisites.
4. Use unique raw roots, max four CPU workers, two unit servers, serial GPU suites and30-minute gate
   bounds. Preserve failed/review-required attempts and compare exact outcomes/input identities.
5. Accepted270 has 580cases/576sources/1,740cells:1,703correct,37unresolved and20resolved histories.
   All576 input hashes remain unchanged. Units1,087pass/13skip; semantics1,170pass/78skip. Runtime4,
   toolkit18 and material compile/assembly6 pass. Last full270, targeted233, cadence zero.
6. Keep frozen `census.slice-195.tsv` immutable and discovery at its 128-source cap. The new270 fixture
   and two268 regression sources remain focused tests outside those inventories.
7. [Research271](report.slice-271-language-breadth.md) completes the four-source breadth probe:
   all twelve cells pass, with actual output inspection supplementing the weak original tuple CHECK.
   [Its evidence](research-evidence.slice-271.json) inherits accepted270 and leaves full cadence zero.
8. [Material profile272](report.slice-272-material-profile.md) and
   [timing evidence](timing-evidence.slice-272.json) qualify identical outputs and restore the complete
   accepted100-entry layout. Raw evidence/snapshots are at `build/nvvm-material-profile272`.
   Next: investigate semantic checking (385–397ms) with bounded call-path sampling and reduction.
   Perf monitoring is blocked by host policy; owned-child GDB works. Qualify sampling bias and exact
   outputs; do not change host settings or present debugger sampling durations as benchmark timings.
   [Research273](report.slice-273-semantic-profile.md) completes16 exact-output debugger profiles:
   982stacks/912sensitivity show generic overload/inheritance paths in every profile. No concrete
   key/cache-outcome or representative reduction is established. Next bounded gate observes canonical
   identities, source/candidate context and cold/valid/computing/stale/incomplete cache outcomes.
   [Research274](report.slice-274-inheritance-observation.md) closes that lead: all8 runs compute
   each of6230canonical inheritance keys once. All42controls and exact1100unit/1248semantic maps
   pass; accepted100-entry layout is restored exactly. No optimization or reduction. Reviewer-agent
   thread limit required separate local audits. Next: bounded concurrent NVRTC automatic-PCH incident
   investigation. Next real compiler build must refresh version metadata and rebuild restored observer
   objects; current installed accepted270 bytes need no rebuild.

The270 compact record preserves the original two-transition review-required comparison and both
resolved failure histories. Raw artifacts and local closeout are under `build/nvvm-fp8-aggregate270`.
Earlier267 timing/quality and262 AST proof retain their original source identities. Material GPU
runtime contracts remain unavailable; concurrent NVRTC automatic-PCH reliability remains open.
Earlier presentation packages were not refreshed. [HISTORY](HISTORY.md) indexes prior evidence.

Research275 completes the bounded PCH reproduction: all40 obligations retained,24 serial/private
controls pass; shared16 yield6pass/8reuse assertion failures/2SIGSEGV. Same-path cross-process
replacement/truncation is traced; original262 deletion signature remains unclosed. See
[report275](report.slice-275-pch-reproduction.md) and [evidence275](research-evidence.slice-275.json).
Next bounded276 must settle private namespace lifetime and explicit user-directory precedence before
production changes. Compiler destruction need not unload NVRTC; existing shared-library wrappers do
not establish ownership across arbitrary loader references. Preserve source identity and same-owner
reuse. All accepted270 bytes/pins/inputs/configuration remain exact; next real build refreshes version
metadata and restored observer objects. Root used separate local audits due reviewer thread limit.
