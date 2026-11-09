# Event tracking

Event tracking exposes resource allocations, pipeline creation, GPU work, and command
submissions. You can view individual events with `slangi` traces, aggregate them with
`-stats`, or receive them through the core interpreter's Events API.

## Event traces

`slangi -trace-all` prints runtime events alongside program output. In this `main.slang`,
the trace shows a buffer allocation, a compute dispatch, and completion before readback:

```slang
import slang.runtime;

[host]
void main()
{
    let values = RWStructuredBuffer<int>.alloc(4);
    Grid(4).dispatch((uint i) =>
    {
        values[i] = int(i) * 2;
    });
    print(values[3]); // 6. Reading the result waits for the dispatch.
}
```

The `batch` and `execution` IDs connect the dispatch to its submission and completion.
Here is an abbreviated trace from D3D12:

```sh
slangi -trace-all -color never -api d3d12 main.slang
# [hgvm] buffer-create ... bytes=16 element-size=4 ...
# [hgvm] pipeline-create ... kind=compute name=dispatchGridCS<...> ...
# [hgvm] dispatch batch=1 execution=1 ... groups=1x1x1
# [hgvm] command-submit batch=1 operations=1 ...
# [hgvm] pipeline-complete batch=1 execution=1 ... gpu-duration-ms=...
# 6
# [hgvm] buffer-recycle ... bytes=16
```

`-trace-n N` limits the number of printed events without changing program execution.

```sh
slangi -trace-n 10 -api d3d12 main.slang
# Prints only the first 10 events, then the program still prints 6 and finishes.
```

Traces go to stdout, so redirection captures both events and program output.
`-color never` produces a log without ANSI color codes.

```sh
slangi -trace-all -color never -api d3d12 main.slang > trace.log
# trace.log contains both [hgvm] events and program print output.
```

| Trace field/event                                 | What it tells you                                                   |
| ------------------------------------------------- | ------------------------------------------------------------------- |
| `module-load`, `buffer-create`, `pipeline-create` | Creation attempts; `duration-ms` measures CPU elapsed time          |
| `buffer-reuse`, `buffer-recycle`, `buffer-evict`  | How allocations move through the buffer pool                        |
| `batch`                                           | Connects recorded GPU work to its command submission                |
| `execution`                                       | Connects a draw/dispatch to its `pipeline-complete` event           |
| `gpu-duration-ms`                                 | Measured GPU duration, present when timestamp queries are available |

`-full-names` includes module-qualified names. Source input gets debug information
automatically; precompiled HGBC retains source attribution when built with `-g1` or higher.

## Aggregate statistics

`-stats` shows a live dashboard on stderr, refreshed approximately once a second, and a
cumulative summary on exit. `-stats-top N` controls the ranking size (default: 5).
The short example above may finish before the first live refresh.

```sh
slangi -stats -stats-top 3 -color never -api d3d12 main.slang
# 6
# HGVM Live Statistics | final | elapsed ... s
# CPU Heap Statistics
#     Allocs: 1
#     Frees: 1
# ...
# Device Statistics
#     Buffers: create 1; reuse 0; recycle 1; evict 1
#     Commands: submit 1; draw 0; dispatch 1; blit 0; clear 0
# ...
# Top pipelines by measured GPU time
#     Count   Total ms   Mean ms  Pipeline
#         1        ...       ...  compute dispatchGridCS<...>
```

Redirected statistics contain only the final summary. Program output stays on stdout.

```sh
slangi -stats -api d3d12 main.slang 2> stats.log
# The terminal still prints 6. stats.log receives the final summary without live redraws.
```

**Reading the totals.** Allocation traffic is not physical GPU memory usage. Pipeline rankings
sum measured GPU durations; untimed completions still count but have no timing row. Creation
times measure CPU work. GPU durations are not a utilization percentage. Tracking collects
completions during existing polls/waits and adds no GPU waits.

**Separate runs.** `-stats` cannot be combined with tracing or `-debug`. Debugging can be
combined with `-trace-all` or `-trace-n N`.

## Custom event handlers

The core interpreter's **Events API** delivers structured events to `IEventSink::onEvent`.
A custom handler can log selected operations, collect counters, or integrate with another
profiler. This handler prints a message for each recorded dispatch:

```cpp
#include "slang-hgvm/slang-hgvm.h"
#include "slang-hgvm/slang-hgvm-events.h"
#include <cstdio>

using namespace Slang;
using namespace Slang::HGVM;

struct MyEvents : IEventSink
{
    void onEvent(const TrackingEvent& event) noexcept override
    {
        if (event.kind == TrackingEventKind::Dispatch)
            std::puts("dispatch recorded"); // dispatch recorded
    }
};

// Interpreter configuration with an existing RHI device:
RefPtr<MyEvents> events = new MyEvents;
Desc desc;
desc.device = device;
desc.eventSink = events;
desc.enableEventTracking = true;
Interpreter interpreter(desc);
// Invoking the example above through this interpreter triggers the handler once.
```

The interpreter retains the sink. Callbacks run synchronously on the calling thread and
must not throw or re-enter execution. Borrowed event data needs to be copied if retained
beyond the callback.

## Tracking builds

Tracing, statistics, and custom handlers require a [native build](installation.md#build-slang)
with `SLANG_ENABLE_HGVM_EVENT_TRACKING=ON`. The option is off by default.
For the default CMake preset, the configuration is:

```sh
cmake --preset default -DSLANG_ENABLE_SLANGI=ON -DSLANG_ENABLE_SLANG_RHI=ON -DSLANG_ENABLE_HGVM_EVENT_TRACKING=ON
cmake --build --preset releaseWithDebugInfo --target slangi
```

The corresponding Windows presets are `vs2022-dev` and `vs2022-dev-releaseWithDebugInfo`.
Native applications linked against the `slang-hgvm` CMake target inherit the matching
tracking configuration. Builds without tracking report a configuration error for trace
and statistics options.

See the [Events API reference](../../source/slang-hgvm/README.md#hgvm-event-tracking)
and [statistics reference](../hgvm-stats.md) for payloads, source attribution, and timing details.
