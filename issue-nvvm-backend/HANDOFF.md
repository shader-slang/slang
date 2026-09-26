# Start a fresh NVVM session

Current accepted baseline: [262](runtime-validation.slice-262.json). Latest package:
[attribution refresh265](results/2026-09-26-attribution/README.md), which retains original263
compile/quality results and adds qualified264 explanations. The finite follow-up is complete and the
development loop is **stopped**. Recheck STATUS; this header is navigation, not renewed authority.

1. Read STATUS and WORKFLOW. Confirm the request's authority: results refresh, bounded maintenance,
   or explicit development resume. The finite maintenance and material follow-up sequences are complete. Do
   not choose another compiler slice merely because an old report lists a candidate.
2. Inspect branch/HEAD, working changes, submodule pins and active plan. Preserve unrelated work.
   Finish outstanding acceptance before treating a pending checkpoint as a baseline.
3. Find the `slang-build` skill (`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md` on this
   host), read it, and build matching optimized tools when source changed. Native Linux uses native
   tools; WSL uses Windows tools per AGENTS. Never reuse binaries simply because their paths exist.
   Refresh CMake's cached version metadata as described in RESULTS and verify `slangc -version`
   against the compiler source revision before freezing new evidence.
4. Follow RESULTS for exact refresh/checkpoint commands. Keep new output roots unique, retain failed
   runs, cap CPU workers at four, run GPU suites sequentially, and isolate all performance work.
5. Review compiler/provider/toolkit/cache/input identities and exact old per-cell obligations.
   Keep unknown upstream transitions as review-required until resolved. Update histories rather than
   resetting the baseline. Report fresh and inherited evidence separately.
6. Update the bounded plan, compact report, structured results and STATUS; obtain lead acceptance,
   then make the authorized local commit. Send the slice-completion Slack DM under WORKFLOW's
   notification policy; the maintainer's authorization persists across sessions. Check prior delivery
   before resending, and report unavailable delivery in the handoff. Stop at the requested boundary.
   No push is implied.

For a future explicitly resumed development loop, select one workload-driven slice using WORKFLOW's
correctness priority and material cadence. Use one implementation writer and separate review; stop
at independent blockers. For a results-only refresh, use the same accepted source and fixed workload
manifests, and refresh provenance and measurements without extending support or restarting the loop.

The current material follow-up promoted no compiler optimization. The default build is restored to
accepted262; research instrumentation and its binaries are preserved separately under
`build/nvvm-material-followup`. Use RESULTS's optional stage reporting and the retained experiment
patch only for an authorized new attribution run, with fresh boundary/output qualification. Regenerate
CMake version metadata before the next real compiler build; do not infer identity from the path.

The strongest future material question is a differential reproducer for lost aggregate constants
across helper boundaries. The general grouped/separated probes demonstrate an opportunity but do
not reproduce the NVRTC/NVVM asymmetry. One-shot serialization is a separate sub-percent opportunity;
compact local storage has broader ABI/representation scope. These are discussion items, not active
slices. Accepted262's39 gaps and PCH concurrency limitation remain unchanged.
