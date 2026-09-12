<!--
SPDX-FileCopyrightText: The Khronos Group, Inc.
SPDX-License-Identifier: CC-BY-4.0
-->

# Container statistics

Records, for every place a container is **declared** in the source, how large that container
actually gets and which operations it sees, so that declarations which never grow past a handful of
elements can be converted to the inline-storage `ShortList` / `ShortDictionary` variants. The same
machinery records every string-buffer allocation, so that the case for a small-string optimization
can be argued from measurements rather than intuition.

Enabled by the `SLANG_ENABLE_CONTAINER_STATS` CMake option. When the option is off, every macro
expands to nothing, no field is added to any container, and there is no run-time cost.

## Quick start

```bash
# A separate build directory, so that toggling the option does not clobber a normal build.
cmake -S . -B build-cstats -G "Ninja Multi-Config" -DSLANG_ENABLE_CONTAINER_STATS=ON
cmake --build build-cstats --config Debug --parallel --target slangc

# Any run with this variable set writes <path>.<pid>.json at exit.
SLANG_CONTAINER_STATS=/tmp/stats ./build-cstats/Debug/bin/slangc foo.slang -target spirv -o /tmp/o.spv

# Merge and rank. Pass a glob; the analysis merges across processes and runs.
python3 tools/container-stats/analyze.py '/tmp/stats.*.json' --top 20
```

To collect over a realistic workload, point the compile-perf suite at the instrumented binary. It
invokes `slangc` as a subprocess, so each workload writes its own file and they merge naturally:

```bash
SLANG_CONTAINER_STATS=/tmp/cstats-run/perf python3 tools/compile-perf/bench.py \
    --slangc "$PWD/build-cstats/Debug/bin/slangc" --label cstats --samples 1 --warmup 0 \
    --out /tmp/cstats-run/results
python3 tools/container-stats/analyze.py '/tmp/cstats-run/perf.*.json' --top 20
```

`--family List` restricts the report to one family; `--json` writes the full ranking for further
processing. `Slang::dumpContainerStats(path)` is also callable directly, for embedders and for
dumping at a point other than exit.

## Collecting from an application that embeds Slang

Nothing needs to change in the host application. Only Slang itself is rebuilt; the statistics are
written by the library at process exit when an environment variable names a destination.

```bash
# 1. Build Slang with the option on, in its own directory.
cmake -S . -B build-cstats -G "Ninja Multi-Config" -DSLANG_ENABLE_CONTAINER_STATS=ON
cmake --build build-cstats --config Debug --parallel

# 2. Confirm the resulting library really is instrumented.
strings build-cstats/Debug/lib/libslang-compiler.so | grep -q '^SLANG_CONTAINER_STATS$' \
    && echo instrumented

# 3. Point the application at that library and give it somewhere to write.
LD_LIBRARY_PATH=$PWD/build-cstats/Debug/lib \
SLANG_CONTAINER_STATS=/tmp/mystats/run \
    ./my-application

# 4. Merge and rank. One file is written per process, so pass a glob.
python3 tools/container-stats/analyze.py '/tmp/mystats/run.*.json'
```

The host application does **not** need to be recompiled, and does not need the option defined. The
public API in `include/slang.h` is a COM-style interface that does not include any of the
instrumented container headers, so enabling the option changes nothing about the types an embedder
sees. The one exception is an embedder that includes Slang's internal headers directly or links
Slang statically into its own objects: that is the ODR hazard described against the CMake option,
and such a build has to define `SLANG_ENABLE_CONTAINER_STATS=1` throughout, exactly as Slang's own
build does.

`dlopen`/`dlclose` works as well as linking directly; the dump is written when the library is
unloaded rather than at process exit.

Things worth knowing before reading the output:

- **One file per process**, named `<path>.<pid>.json`, so the destination directory must exist and
  the same `SLANG_CONTAINER_STATS` value can be used for a whole run of many processes. Distinct
  processes cannot overwrite one another unless the operating system recycles a pid within the run.
- **The process must exit normally.** A crash or a kill leaves no file, or an empty one; `analyze.py`
  reports and skips those rather than failing.
- **Symbolizing the sampled backtraces needs debug info** in the same build, and `addr2line` on the
  path. Without it the report still ranks sites correctly but cannot name the calling function, so
  `RelWithDebInfo` is the better configuration for a large collection.
- Expect roughly **1.07x** on compile time and a few megabytes per process on disk.

## What is instrumented

| Family                                | What a record is                              | What the report asks                                    |
| ------------------------------------- | --------------------------------------------- | ------------------------------------------------------- |
| `Dictionary`, `HashSet`               | one declaration site                          | could it be a `ShortDictionary`, and at what capacity   |
| `List`                                | one declaration site                          | could it be a `ShortList`, and at what capacity         |
| `ShortList`, `ShortDictionary`        | one declaration site                          | was the capacity it already declares a good choice      |
| `OrderedDictionary`, `OrderedHashSet` | one declaration site                          | how large do these get (there is no inline variant)     |
| `ImmutableHashedString`               | one declaration site                          | how long are interned identifiers                       |
| string buffers                        | one allocation site, plus sampled call stacks | would a small-string optimization pay, and at what size |

## What gets recorded, and why it is shaped this way

The constraint that determines the whole design is **volume**. A single `slangc` invocation
constructs millions of containers — the compile-perf suite produced 107 million events — so nothing
may be recorded per instance.

Instead, each construction resolves to a `SiteRecord`, one per `(file, line, container type)`, and
updates counters inside it. Each destruction folds that instance's peak size into the record's
histogram. In the measured run, **106,918,151 events collapsed into 1,542 records**. The number of
records is bounded by the size of the source code, not by the workload, so the output stays a few
megabytes and is written once, at exit, in a single pass.

The cost of that choice is that instances cannot be correlated after the fact. Every question the
ranking asks has to be expressible as a site-level accumulator. The one join that matters — how
many lookups happened in containers that would have stayed inline, versus in containers that would
have been promoted — is preserved explicitly by binning lookups and insertions by the _instance's_
final peak bucket (`lookupsByPeak`, `insertsByPeak`). That is what lets the report sweep candidate
inline capacities offline without having kept per-instance data.

Per record:

| Field                                                      | Purpose                                                                                       |
| ---------------------------------------------------------- | --------------------------------------------------------------------------------------------- |
| file, line, function, container type                       | identifies the declaration                                                                    |
| `peakHistogram`                                            | exact buckets 0..32, then one per power of two to 2^20                                        |
| `lookupsByPeak`, `insertsByPeak`                           | the join described above, for any candidate capacity                                          |
| `instances`, `movedAway`, `everAllocated`, `liveHighWater` | benefit numerator and memory cost                                                             |
| `opMask`                                                   | hard disqualifiers, per family — `ShortDictionary` is add-only, `ShortList` is not contiguous |
| `inlineCapacity`                                           | for a `Short*` type, the capacity it declares, so promotion is `peak > inlineCapacity`        |
| `sampleRate`                                               | how many real events each recorded one stands for; 1 except for sampled string stacks         |
| key/value size and default-constructibility                | `Short*` requires default-constructible types                                                 |
| up to 4 sampled backtraces                                 | context, and provenance for sites that resolve to library code                                |

The per-instance counters are plain integers, not atomics; they are folded into the shared atomic
record exactly once, in the destructor. A container shared between threads through a `const&` can
therefore lose counter updates to a race, which makes the totals approximate. That is acceptable for
a ranking and is what keeps the hot path cheap.

### Reconciliation invariant

`sum(peakHistogram) == instances - movedAway` holds for every record, and is worth re-checking after
any change to the instrumentation. A moved-from container is detached so it folds nothing (its
statistics transfer to the destination, which is the same logical container continuing its life
elsewhere), and `movedAway` counts those so the numbers still add up.

`ContainerStatsProbe` deliberately has its copy and move operations deleted. A copied probe would
fold a second peak without a matching increment to `instances`, breaking that invariant silently;
deleting them turns the mistake into a compile error in any container that has not declared its own
copy and move constructors — which is exactly the set of containers that need to capture a fresh
site rather than inherit one.

### A per-record bit cannot answer a per-event question

`opMask` is a union over everything a record ever saw, so it can only answer "did this ever
happen", never "how often". Two facts that look like they belong there are therefore recorded
differently:

- A `Short*` container spilling past its inline storage is not a bit. It is already decided by the
  data: an instance promoted exactly when its peak exceeded the type's `inlineCapacity`.
- A string buffer replacing one that already existed is not a bit either, because the same line in
  `ensureUniqueStorageWithCapacity` both gives a string its first buffer and reallocates an
  existing one. It is separated by recording the two cases under different type tags, so they
  become different records.

The second of these was a real bug before it was fixed: with a single bit, one growth from a line
marked every allocation from that line as a growth, and the report claimed 99.9% of string
allocations were reallocations when the true figure is 0.6%.

## How a declaration site is identified, and how precise it is

Sites are identified by `__builtin_FILE()` / `__builtin_LINE()` / `__builtin_FUNCTION()` as
defaulted constructor arguments, which are evaluated at the point of call. A probe across g++ 14.3
and clang++ 21.1 at both `-O0` and `-O2` — all four combinations agreed — established exactly how
precise that is:

| Declaration shape                                                                                    | What is reported                                                                          |
| ---------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| local variable, temporary, copy, function-local or file-scope `static`                               | **the exact declaration line**                                                            |
| member field (implicit default ctor, default member initializer, class template, implicit copy ctor) | **the enclosing class's `struct X {` line**                                               |
| member field omitted from a user constructor's mem-init list                                         | a line inside that constructor (gcc: the mem-init line, clang: the ctor declaration line) |
| member field explicit in a mem-init list (`: m()`)                                                   | that mem-init line exactly                                                                |

So **locals are exact**, which is the case that matters most, since the prime `Short*` candidates
are function-local containers constructed per operation. **Member fields resolve to class
granularity.** Because the record key includes the container type, two fields of _different_ types
in one class stay separate records; two fields of the _same_ type merge, and `analyze.py` prints an
explicit warning when it detects that:

```
WARNING: the class at this line declares 2 fields of this type; their statistics are merged
```

No mechanism fixes that merge — a backtrace does not either, since both fields are constructed from
the same constructor frame.

Two further cases resolve to _library_ code rather than to user code, and are what the sampled
backtraces are for:

- containers constructed through a generic helper, e.g. `slang-allocator.h:100`, which is the
  `new (rs + i) T();` inside `allocateArray`;
- containers constructed inside a standard library type, e.g. `stl_pair.h:295`, which is a
  `HashSet` being moved as a `Dictionary`'s value type.

For these the `context:` line in the report, symbolized from the backtrace, names the real caller.

### Constructors that cannot learn their caller

A constructor taking a parameter pack — `List(const T& val, Args... args)`,
`ShortList(const T& val, Args... args)`, `OrderedDictionary(Arg arg, Args... args)` — has nowhere to
put a defaulted site parameter, because anything appended to the signature is swallowed by the pack.
Containers built that way are still counted, but are attributed to the container's own header rather
than to the user's declaration. `List<int> a(1, 2, 3);` is the shape in question; `List<int> a;`
followed by `add` calls is attributed exactly.

### Wrappers must forward their site

A container held as a member of a wrapper reports the _wrapper's_ constructor, not the user's
declaration. `HashSet` is built on `HashSetBase`, which holds a `Dictionary`, so without forwarding
every `HashSet` in the codebase would collapse into one record per element type located in
`slang-dictionary.h`. `HashSetBase` and `HashSet` therefore capture a site themselves and pass it
down; `ShortDictionary` does the same for the `Dictionary` it overflows into. Any future wrapper
around an instrumented container has to do the same.

Two details of that forwarding are easy to get wrong and are commented in the source: default, copy
and move constructors are never inherited, so `using Base::HashSetBase` does not bring them in and
they must be declared explicitly; and the base must be passed as `static_cast<const Base&>(rhs)`,
or `HashSetBase`'s variadic constructor template becomes a better match than its copy constructor.

## Reading the report

```
site                                container          inst   C   fit%  allocs saved    mem
source/slang/slang-ir.cpp:9041      HashSet<IRInst*> 365,703   2  99.6%      364,191     16B
                                    sweep: C=2:100% C=4:100% C=8:100% C=16:100% C=32:100%
                                    context: Slang::_replaceInstUsesWith(...) at slang-ir.cpp:9041
```

- **C** — the recommended inline capacity. Not the highest-scoring one: the score counts avoided
  allocations against extra comparisons but not the memory an inline array costs in every instance,
  so it rises almost monotonically with capacity. The report instead recommends the _smallest_
  capacity that still achieves 95% of the available benefit.
- **fit%** — the fraction of instances whose peak would have stayed inline at that capacity.
- **allocs saved** — instances that allocated at all but would have stayed inline.
- **mem** — `C * elementSize * liveHighWater`, the memory the inline arrays would add.
- **sweep** — fit at every candidate capacity, so the choice can be made per site rather than
  defaulting to 8 or 16.

The disqualifying operations differ per family. `ShortDictionary`'s entire API is `add` and
`tryGetValue`, so a `Dictionary` site that does anything else is hard-disqualified. `ShortList`
splits its elements between an inline array and an overflow buffer, so a `List` site is
disqualified by anything needing one contiguous range (`getBuffer`, `getArrayView`, `sort`,
`attachBuffer`) or needing to shift elements about (`insert`, ordered removal, `swapWith`);
indexing, iteration, `clear` and `setCount` are all fine. `reserve` is reported as a note rather
than a disqualifier, because such a call could simply be deleted.

The anti-candidate sections matter as much as the ranking: sites that are almost always empty (pure
size overhead in their parent), and sites with very high lookup counts that should explicitly keep
their hash table.

## What the measurement found

From the compile-perf suite (30 of 31 workloads; `mdl_dxr` needs a corpus that is not checked in),
a Debug instrumented build, 1,542 records over 106.9M events:

| Family                  | Records |     Events |
| ----------------------- | ------: | ---------: |
| `List`                  |     764 | 45,860,877 |
| `ShortList`             |      61 | 26,561,862 |
| `Dictionary`            |     293 | 10,442,214 |
| string buffers (fresh)  |     157 |  7,582,922 |
| `ImmutableHashedString` |      14 |  6,274,346 |
| `OrderedDictionary`     |      55 |  4,226,267 |
| `ShortDictionary`       |       2 |  4,045,158 |
| `HashSet`               |     156 |  1,848,186 |
| string buffers (growth) |      28 |     51,428 |

### `List` is where the volume is

| Site                                | Container                  | Instances |   C |   Fit | Allocations saved |
| ----------------------------------- | -------------------------- | --------: | --: | ----: | ----------------: |
| `slang-ast-base.h:382` (`Val::Val`) | `List<ValNodeOperand>`     | 6,540,420 |   8 | 99.8% |         6,529,282 |
| `slang-ir-dce.cpp:294`              | `List<IRInst*>`            | 6,660,016 |   8 | 98.9% |         1,096,927 |
| `slang-ast-support-types.h:1213`    | `List<DeclRef<ParamDecl>>` |   650,201 |   2 |  100% |           648,395 |
| `slang-check-overload.cpp:2940`     | `List<QualType>`           |   646,529 |   2 |  100% |           646,529 |
| `slang-ast-expr.h:205`              | `List<Expr*>`              |   571,233 |   2 | 99.9% |           569,297 |

Of 764 `List` sites, 198 are disqualified — 146 of them because something takes the contiguous
buffer.

### `Dictionary` and `HashSet`

| Site                                         | Container                   | Instances |   C |   Fit | Allocations saved |
| -------------------------------------------- | --------------------------- | --------: | --: | ----: | ----------------: |
| `slang-ir.cpp:9041` (`_replaceInstUsesWith`) | `HashSet<IRInst*>`          |   365,703 |   2 | 99.6% |           364,191 |
| `slang-ir-sccp.cpp:29`                       | `HashSet<IRBlock*>`         |   145,090 |   2 | 97.6% |            50,802 |
| `slang-ir-ssa.cpp:578`                       | `HashSet<IRBlock*>`         |    39,583 |   2 | 94.5% |            37,123 |
| `slang-check-decl.cpp:1029`                  | `HashSet<Val*>`             |    36,351 |  16 | 93.0% |            31,875 |
| `slang-ir-dce.cpp:711`                       | `Dictionary<IRInst*, bool>` |    90,402 |   4 | 98.2% |            16,391 |

### The existing `Short*` uses are sound, and several are over-provisioned

Every `Short*` site measured promotes rarely, so the existing choices are not wrong; but the
capacities are mostly larger than the data needs.

| Site                              | Container                               |  Instances | Declared | Promoted | Suggested |
| --------------------------------- | --------------------------------------- | ---------: | -------: | -------: | --------: |
| `slang-ast-base.h:241`            | `ShortList<ValNodeOperand, 8>`          | 10,175,603 |        8 |     0.2% |         8 |
| `slang-ir.cpp:2815`               | `ShortList<IRInst*, 8>`                 |  3,821,868 |        8 |     0.1% |         4 |
| `slang-ast-decl-ref.cpp:920`      | `ShortList<GenericDecl*, 16>`           |  2,255,237 |       16 |     0.0% |         2 |
| `slang-check-constraint.cpp:4344` | `ShortList<FlattenedTypeRangePair, 16>` |  1,113,169 |       16 |     0.0% |         2 |
| `slang-ast-substitution.h:51`     | `ShortDictionary<Key, Result, 8>`       |  4,001,944 |        8 |     2.3% |         8 |

Three `ShortList` sites in `slang-check-constraint.cpp` declare a capacity of 16 and are empty in
over 99% of their 1.1M instances each — they pay for 16 inline slots that are essentially never
used.

### Strings: half of all allocations would fit in 32 characters

3,818,670 string-buffer allocations, of which only 22,116 (0.6%) replaced a buffer the string
already had. So the doubling growth policy is almost never exercised; nearly every allocation is a
string being born at its final size.

| Inline capacity | Allocations removed |
| --------------: | ------------------: |
|               8 |               28.8% |
|              16 |               37.0% |
|              24 |               44.8% |
|              32 |               50.0% |

The sampled call stacks separate two very different populations, which is the reason for keying
them on the stack rather than on a source location:

- **`IRModule::buildMangledNameToGlobalInstMap`** — ~1.81M allocations, **0%** of which would fit in
  16 characters. These are mangled names, which are long by construction. No inline buffer helps
  here; not allocating a `String` at all would.
- **`NamePool::getName`** — ~1.05M allocations, **92%** of which would fit in 16 characters. This
  constructs a `String` from an `UnownedStringSlice` purely to look it up in a
  `Dictionary<String, RefPtr<Name>>`. A heterogeneous lookup, or an inline buffer, removes a
  million allocations a run.

`ImmutableHashedString` tells the same story from the other side: of 6.27M constructions only 28.6%
build a buffer at all (the rest share one), and only 17.1% of them would fit in 32 characters —
because the dominant producer is again `buildMangledNameToGlobalInstMap`. An inline buffer on
`ImmutableHashedString` is **not** indicated by this data.

### Always-empty containers

286 sites are empty in at least 99% of their instances. These are pure size overhead in their parent
objects and want removal or lazy allocation rather than conversion. The largest are
`slang-ast-base.h:765` (`List<ProvenenceNodeWithLoc>`, 1,881,125 instances),
`slang-dictionary.h:710` (`OrderedDictionary<Decl*, RequirementWitness>`, 1,461,644) and
`slang-check-impl.h:3175` (`Dictionary<Decl*, Val*>`, 1,257,501).

## Cost

Measured over the compile-perf suite, Debug build, comparing the same source compiled with the
option off and on (`compileInner`, 30 workloads):

|              |                                     |
| ------------ | ----------------------------------- |
| total        | **1.07x**                           |
| per-workload | median 1.12x, max 1.48x (`minimal`) |

The largest relative costs are on the smallest workloads, where a fixed start-up cost dominates and
the absolute difference is a few milliseconds. Note that these are single-sample runs with no warm-up,
so an individual workload's ratio is noisy — one workload measured _faster_ than the baseline. The
total is the figure to trust.

Read the figure with one further caveat: a Debug baseline is itself slow, so the _relative_ overhead
is understated. Expect a larger ratio in an optimized build.

## Limitations

- Backtraces require `SLANG_HAS_BACKTRACE`, which is Linux-glibc and Android only. On Windows only
  the `__builtin_FILE/LINE` data is available. `__builtin_FILE/LINE` on MSVC is documented as
  working from 19.26 but has not been verified here.
- Frames are recorded as `<module>+0x<offset>` rather than as raw addresses, because the modules
  are position independent; `analyze.py` resolves them with `addr2line`. The measurement build
  therefore needs debug info. `RelWithDebInfo` is the better choice for large corpora — the
  statistics themselves are independent of optimization level, so a Debug build is valid but slower.
- Containers destroyed after the `atexit` handler runs (file-scope statics destroyed late) do not
  fold their final peak.
- The operation mask **understates** contiguity requirements for `List`. `begin()` and `end()` hand
  out a `T*`, but they are not recorded as requiring a contiguous buffer, because a range-based
  `for` goes through them and would work just as well over `ShortList`'s iterator — recording them
  would set the disqualifying bit on nearly every site and make it meaningless. A candidate `List`
  site therefore still has to be read before it is converted.
- `reserve` records only the operation, not its argument. The statistic is how many elements a
  container actually holds; a `reserve` states what the caller anticipated, and folding it into the
  peak would overstate peaks and bias the ranking against conversion.
- String-allocation records attributed to a call stack are sampled one in 64, so their counts are
  estimates. They are sound for ranking callers against one another, but the exact totals come from
  the per-line records, which are not sampled.
- A string buffer's recorded size is the number of characters that had to fit, not the number
  allocated. `ensureUniqueStorageWithCapacity` rounds its request up to a minimum of 16 and
  thereafter doubles; recording the allocated size would put a floor under the histogram and make a
  small inline buffer look useless when the strings are in fact short.

## Results

`findings.md` records what two profiles -- a core-module compile and the whole test suite --
actually showed, together with what to distrust in those numbers and what the instrumentation
does not measure.

`findings-falcor.md` repeats the exercise against a real application embedding Slang, which is 128
times the test suite by event count. It confirms most of `findings.md`, overturns its largest
recommendation, and finds the one class of result a self-compile cannot show: what happens when the
host keeps the compiler alive between compiles.

## Status

All of `Dictionary`, `HashSet`, `List`, `ShortList`, `ShortDictionary`, `OrderedDictionary`,
`OrderedHashSet`, `ImmutableHashedString` and string-buffer allocation are instrumented, along with
the CMake option, the JSON dump and `analyze.py`.
