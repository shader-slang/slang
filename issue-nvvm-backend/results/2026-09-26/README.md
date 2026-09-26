# NVVM backend: Monday results package

The merged backend preserves the accepted runtime coverage and compiles the 7,113-line material
shader through both entry points. Fresh-process O3 compilation is near parity for `eval_buffer` and
about 5% slower for `sample_buffer` than NVRTC O3. The fixed simple-shader subset shows mostly equal
register allocation with several smaller executable outputs. These measurements do not establish
an overall compile-speed or generated-code advantage.

Use [PRESENTATION](PRESENTATION.md) for a six-slide speaking outline. Shareable PNG/SVG figures and
complete tables are below. [RESULTS](../../RESULTS.md) contains the maintained rerun and package-refresh
commands; [HANDOFF](../../HANDOFF.md) explains how a new session continues development when authorized.

## Material compilation

| Entry           | NVRTC O3 median | NVVM O3 median | NVRTC/NVVM time ratio | Interpretation                |
| --------------- | --------------: | -------------: | --------------------: | ----------------------------- |
| `eval_buffer`   |       1362.76ms |      1355.04ms |                 1.006 | Near parity; overlapping IQRs |
| `sample_buffer` |       1390.16ms |      1460.18ms |                 0.952 | NVVM takes 5.04% longer       |

Ratio means NVRTC median divided by NVVM median; above 1 favors NVVM. The evaluation difference is
only 7.71ms (0.57%). Sampling is slower with NVVM in both rounds (4.48–5.50%). These are paired settings
in one host session, not a causal improvement over an earlier compiler revision.

![Material compile wall time](material/wall-time.svg)

[Material table and phases](material/summary.md) · [structured results](material/summary.json) ·
[PNG](material/wall-time.png) · [SVG](material/wall-time.svg)

Two entries × three modes × two opposite-order rounds, each with 2 warmups and 9 measured fresh
processes:132 compile attempts,108 measured. Separate assembly has66 attempts,54 measured.
All attempts passed and each cell's PTX/cubin hashes stayed stable. Wall time covers process creation
through exit and excludes log writing. Filesystem/toolkit caches are warmed; IQR is sample spread,
not a confidence interval. No outliers were removed or failed samples retried.

Assembly also favors NVRTC O3: evaluation 174.09ms versus NVVM 210.81ms; sampling 254.31ms versus 295.17ms.
Do not add these independent medians and label the sum a measured compile-to-cubin latency.
NVVM O0 compiles faster here, but its assembly is substantially slower and its code is much larger;
that is an optimization-level tradeoff, not an equivalent O3 speedup.

Material O3 named-entry resources:

| Entry           | NVRTC registers / stack | NVVM registers / stack | Spill stores/loads |
| --------------- | ----------------------: | ---------------------: | ------------------ |
| `eval_buffer`   |            48 /592bytes |           67 /784bytes | Zero in both       |
| `sample_buffer` |            63 /624bytes |           86 /784bytes | Zero in both       |

NVVM uses more registers and stack for this material. Material runtime correctness and GPU speed
remain unassessed because the binding, texture/LUT, input and expected-output contracts are missing.

## Simple-shader code observations

The fixed 12-source/36-mode subset has accepted runtime correctness for these exact sources and
compiler/provider bytes. Comparing O3 to O3, NVVM register counts are lower in 1 fixture, equal in 10,
and higher in 1. Executable `.text` is smaller in 3 fixtures and equal in 9. All 24 O3 entries have zero
spill stores/loads. These are resource/size observations, not kernel speed or occupancy measurements.

![Paired O3 entry registers](quality/entry-registers.svg)

[All quality rows, including O0](quality/summary.md) · [structured results](quality/summary.json) ·
[PNG](quality/entry-registers.png) · [SVG](quality/entry-registers.svg)

| Fixture                 | NVRTC O3 → NVVM O3                                          | Tradeoff                               |
| ----------------------- | ----------------------------------------------------------- | -------------------------------------- |
| Helper copyable values  | Registers12→11; stack32→8bytes; executable text768→640bytes | Favorable resource/size observations   |
| Half values             | Registers13→14; stack8→0bytes; text896→768bytes             | Smaller code/stack, one extra register |
| Runtime vector indexing | Registers27→27; stack0→80bytes; text2560→2048bytes          | Smaller code, extra stack              |

Registers and stack refer to the named kernel entry; executable text covers the module. Cubin bytes
also contain metadata, and PTX byte counts include formatting/symbols. SASS instruction counts are
unavailable because this selected CUDA installation has no `cuobjdump`; no zero counts are inferred.
Each quality cell is compiled once, so its latency is not a speed benchmark.

## Correctness and integration

[Accepted262](../../runtime-validation.slice-262.json) records 1713 cells with 1674 correct,
39 unresolved and 18 retained resolved histories. This is a full checkpoint **plus explicit serial
infrastructure closure**: the full run had 1673 correct and one extra NVRTC PCH deletion failure;
three predeclared serial rounds passed all three modes (9/9), and only that NVRTC outcome was
substituted. The original failure and comparison remain recorded. Concurrent automatic-PCH
reliability is still open; this is not a clean parallel-run claim.

The six upstream texture diagnostic transitions affect already unresolved cells and add no support.
The 39 remaining gaps comprise 32 infrastructure,4 preflight and3 column-major runtime mismatches.
Units pass 1086 with 13 skips; semantic suites 1170 with 78 skips; focused integration 29, runtime smoke 4,
toolkit 18 and material support 6 pass. Exact identities and additions, 14 changed fixture hashes,
runner contracts and the 705-tag/497025-pair AST proof are in the ledger and
[integration report](../../report.slice-262-master-integration.md).

## Reproduction and limits

Measurements use workspace revision `201cea6c96e09345700b77f2837404249bdd2deb`, compiler source
`49593da724e172838bd65b9eb48b2a3f11334522`, and runner source `c3455e606`.
The binary reports `2026.18.3-275-g49593da72`; provider ABI42 remains unchanged.
Upstream master `6eb89786ca882d71049c8568638e247f60864b6f` is merged; all 22 dependency pins match.
Native Ubuntu24.04, RelWithDebInfo, LLVM14, CUDA12.9.2/NVRTC12.9.86, targetSM80,
L4 deviceSM89, driver580.126.09. Matplotlib3.11.2 generated the figures.

[Package metadata](package.json) links accepted correctness and measurement gates. Each generated
summary retains compiler/provider/cache/toolkit/tool identities plus the hashed full raw measurement.
Raw evidence remains under `build/nvvm-results/2026-09-26-integration`; paths in JSON are local evidence
references, not portable download links. No build, test or profiler competed with measurements.

The material commands use default floating-point mode. Source inspection indicates neither explicit
`--fmad=false` nor `--use_fast_math` is requested; no downstream option trace is claimed. Automatic
NVRTC PCH state is not directly observable in these CLI logs; missing markers do not mean disabled
caching. Normal cache behavior was retained.

The [shared-session appendix](shared-session.md) measures a distinct fixed-order six-request process.
Its request latencies have different initialization exposure and must not replace the fresh-process
comparison. It includes references, all batch lifetimes and whole-command cost.

Refresh using [RESULTS](../../RESULTS.md) with a new output/package directory and the current accepted
baseline. Preserve previous packages and failed attempts. The development loop is **stopped**;
measurements did not authorize additional optimization or feature work.

SVG exports have trailing line whitespace normalized for repository formatting; XML tokens and chart
values are unchanged. Generated summaries and PNGs remain byte-identical to the raw report exports.
