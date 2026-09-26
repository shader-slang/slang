# Start a fresh NVVM session

Current accepted baseline: [262](runtime-validation.slice-262.json). Latest package:
[2026-09-26](results/2026-09-26/README.md). The finite maintenance sequence is complete and the
development loop is **stopped**. Recheck STATUS; this header is navigation, not renewed authority.

1. Read STATUS and WORKFLOW. Confirm the request's authority: results refresh, bounded maintenance,
   or explicit development resume. The finite maintenance sequence is complete. Do
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
   then make the authorized local commit. Stop at the requested boundary. No push is implied.

For a future explicitly resumed development loop, select one workload-driven slice using WORKFLOW's
correctness priority and material cadence. Use one implementation writer and separate review; stop
at independent blockers. For a results-only refresh, use the same accepted source and fixed workload
manifests, and refresh provenance and measurements without extending support or restarting the loop.
