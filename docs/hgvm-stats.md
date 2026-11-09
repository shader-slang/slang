# HGVM statistics

Run `slangi -stats -stats-top 5 -api vulkan program.slang` to display live HGVM statistics.
`-stats-top` is optional and defaults to five entries per ranking. Statistics require a build
with `SLANG_ENABLE_HGVM_EVENT_TRACKING=ON` and cannot be combined with `-trace-all`, `-trace-n`,
or `-debug`.

The dashboard updates on stderr approximately once per second. Program output stays on stdout.
On exit it prints a cumulative summary, including events from interpreter teardown. Redirecting
stderr, or using a terminal without cursor-control support, produces only the final plain-text
summary by default. Small terminals show fewer ranking entries or shader components and indicate
when the live rankings are clipped. The final summary uses the requested count and includes every
component of those pipelines, without a terminal-height limit.

Trace (`-trace-all` or `-trace-n N`) and statistics output share these display options:

- `-color auto|always|never` defaults to `auto`. Color is enabled when the destination is a
  supporting terminal: stdout for tracing, stderr for statistics. A nonempty `NO_COLOR` or
  `TERM=dumb` disables automatic color. `always` explicitly enables ANSI color, including in
  redirected output; `never` disables it. Compiler diagnostics use the separate
  `-diagnostic-color` option.
- `-full-names` shows module-qualified pipeline/type names and the module field on type events.
  By default, known module prefixes are omitted, including in generic arguments. Namespaces,
  nested types, generic arguments, and lambda source locations remain visible. Module-load
  events always name the loaded module. Compact labels do not merge distinct statistics entries.

Types and pipeline names are cyan, quoted strings green, durations yellow, and counts/sizes
magenta. Event names and headings are bold; addresses and separators are dim. Program `print()`
output is preserved. Color never changes table widths or clipping, and styles reset at line ends.
Trace lines start with `[hgvm]` followed directly by the event name. Source locations remain in
lambda names and in the statistics allocation rankings, without a source-path prefix on each event.
The live dashboard still works with `-color never` or `NO_COLOR`. Forcing color on redirected
stderr adds color to the final summary without cursor movement or periodic redraws.

For example, `slangi -stats program.slang` displays `dispatchGridCS<...>` instead of
`runtime/compute/grid::dispatchGridCS<...>`. Use `slangi -stats -full-names program.slang` to show the
module, or `slangi -trace-all -color always program.slang > trace.ansi` to capture colored traces.

Live counts and rankings describe the latest interval, whose elapsed time appears in the header.
The final summary uses the same layout for the entire run. The display includes:

- Allocation counts, allocation traffic, frees, and top source lines.
  These are allocation traffic statistics, not physical GPU memory usage or live-memory estimates.
- Completed pipeline executions, ranked by summed measured GPU duration (`Total ms`).
  Mean durations include only executions with available timestamps. Untimed
  completions still contribute to the overall count; pipelines without any measured duration
  are omitted from the ranking.
- Module, pipeline, buffer, and texture creation; buffer reuse/recycling/eviction; command
  submissions, draws, dispatches, blits, and clears. Creation durations are
  CPU time, separate from GPU execution duration.
- Acceleration-structure builds in **Device Statistics**, with recorded, completed, and timed
  counts and summed GPU duration. Only completed builds with available timestamps contribute
  to the duration; otherwise the display says `GPU time unavailable`.

Acceleration-structure tracking measures native BLAS and TLAS build commands. A TLAS
`scene.build(instances)` snapshots its inputs; the native build is recorded later when a shader
needs that realization. Reusing a cached realization does not count as another build. The timed
interval brackets the RHI build command, including GPU synchronization within that interval,
and excludes host allocation and input preparation. Builds remain separate from pipeline rankings.
Tracing reports `acceleration-structure-build` and `acceleration-structure-build-complete`, with
matching batch and execution IDs, `kind=blas` or `kind=tlas`, and optional `gpu-duration-ms` on
completion. Abandoned command batches do not report successful build completions.

GPU completions are counted when HGVM observes completion, which can occur in batches. Statistics
do not force extra GPU waits. Summed device durations are not a measurement of GPU utilization.
The summary counts creation attempts, including failures, and times those attempts.

Source inputs automatically request minimal debug information. For precompiled `.hgbc` files,
compile with `slangc -target hgbc -g1` to preserve source attribution. Events without line
information remain in the totals and the `no source` count. Pipeline identity uses the staged
descriptor for compute and graphics. Dynamically assembled ray-tracing pipelines use distinct
creation identities, so two pipelines with the same ray-generation shader but different miss or
hit shaders have separate timing rows. Cached pipeline reuse accumulates in the same row; changing
only SBT data does not create a new identity. Display names use specialized entry-point names and
the trace formatter's lambda source locations. Shader names remain available without line info.

Composite pipelines occupy multiple lines in the pipeline ranking. The timing row measures the
whole pipeline execution; the indented rows describe its shaders, not separate stage timings:

```text
    Count   Total ms   Mean ms  Pipeline
        3      0.600     0.200  graphics
            vertex meshVertex<...>
            fragment meshFragment<...>
        2      0.400     0.200  ray-tracing
            raygeneration dispatchGridRayGeneration<...>
            miss contextMiss<...>
            hitgroup[0] closesthit contextClosestHit<...>
            hitgroup[1] closesthit contextClosestHit<...>
```

`-stats-top` counts pipelines, not shader lines. RT entries list the native pipeline's shader
inventory, with zero-based native hit-group indices. A shader used by several groups is listed
in each group; repeated SBT records do not duplicate the listing. These indices are not instance,
geometry, ray-type, or SBT record indices. The display also supports `anyhit`, `intersection`, and
`callable` roles when present in pipeline metadata; it does not change which combinations the
runtime currently supports. Compute pipelines retain their single-line names. Both compact names
and `-full-names` apply to every component.
