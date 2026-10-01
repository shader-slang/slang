<!--
SPDX-FileCopyrightText: The Khronos Group, Inc.
SPDX-License-Identifier: CC-BY-4.0
-->

# What the container statistics found in a real application

`findings.md` records two profiles of Slang compiling its own code: the core module and the
`slang-test` suite. Its closing caveat is that neither resembles what a user compiles. This report
closes that gap. Both profiles below are Falcor 2 driving Slang as a shared library through `sgl`,
which is the embedding case the instrumentation was designed to reach but had never been pointed at.
It drives Slang from Python, so a single process compiles hundreds of shaders and keeps the sessions
alive between them — a lifetime pattern no `slangc` invocation can produce, and one that turns out to
matter more than anything else here.

| Profile    | Workload                                                                                                                                      |           Processes | Records |        Events |
| ---------- | --------------------------------------------------------------------------------------------------------------------------------------------- | ------------------: | ------: | ------------: |
| **falcor** | Falcor's pytest suite — 1,687 passing tests over shaders, materials, MaterialX, scene import, render graphs, path tracing, tools and examples | 19 (14 substantive) |   2,255 | 9,517,084,455 |
| **bench**  | Falcor's benchmark suite in benchmark mode, single process, `--nightly`                                                                       |                   1 |   1,840 |   670,052,058 |

The falcor profile is **128 times the whole `slang-test` suite** by event count, and it is a single
corpus rather than 9,535 unrelated ones. The bench profile is not a subset: it is a separate run, in
a different process shape, and its value here is as an independent check. It is used that way
throughout — every conclusion below is stated for falcor and confirmed against bench, and where
they diverge that is said.

## The headline: what changes, and what does not

Four of `findings.md`'s conclusions survive unchanged, one is overturned, one is re-diagnosed, and
one entirely new class of finding appears that Slang's own tests could not show.

| `findings.md` conclusion                                        | Verdict here                                                                                                                                          |
| --------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| The opportunity is concentrated, not spread out                 | **Confirmed, and much more so**                                                                                                                       |
| `Val::m_operands` is worth converting to `ShortList<_, 8>`      | **Overturned** — at embedder scale the memory cost is gigabytes                                                                                       |
| Existing `Short*` capacities are systematically too large       | **Confirmed**, and the cause is now known: it is the library default. But see the correction under finding 3 — lowering that default is _not_ the fix |
| `ShortDictionary` at `slang-ast-substitution.h:51` is too small | **Overturned** — the promotions are real but cost less than the zeroing a larger capacity would add; see the correction under finding 4               |
| An inline buffer on `ImmutableHashedString` is not indicated    | **Confirmed**, more strongly                                                                                                                          |
| Strings: `NamePool::getName` allocates to do a lookup           | **Confirmed**, and it is now the single largest string caller                                                                                         |
| —                                                               | **New**: two more allocate-to-look-up sites, and a generic-specialisation path, together 74M allocations                                              |

## 1. Ten declarations are 58% of all container traffic

|                                                                       | Constructions | Share | Cumulative |
| --------------------------------------------------------------------- | ------------: | ----: | ---------: |
| `slang-ir-dce.cpp:294` `List<IRInst*>`                                | 1,030,642,317 | 10.8% |      10.8% |
| `slang-ast-decl-ref.cpp:920` `ShortList<GenericDecl*>`                | 1,025,086,687 | 10.8% |      21.6% |
| `slang-ast-base.h:241` `ShortList<ValNodeOperand, 8>`                 |   900,793,759 |  9.5% |      31.1% |
| `slang-ast-substitution.h:51` `ShortDictionary<Key, Result, 8>`       |   537,093,597 |  5.6% |      36.7% |
| — and its overflow `Dictionary`                                       |   537,093,597 |  5.6% |      42.4% |
| `slang-ir.cpp:2815` `ShortList<IRInst*, 8>`                           |   427,593,742 |  4.5% |      46.8% |
| `slang-uint-set.h:67` `List<Element>`                                 |   382,680,354 |  4.0% |      50.9% |
| `slang-ir.h:887` `List<IRInst*>`                                      |   298,928,658 |  3.1% |      54.0% |
| `slang-check-constraint.cpp:835` `ShortList<QualType, 8>`             |   206,595,850 |  2.2% |      56.2% |
| `slang-check-constraint.cpp:4344` `ShortList<FlattenedTypeRangePair>` |   202,599,804 |  2.1% |      58.3% |

bench produces the same ten in almost the same order, cumulative 59.1%.

Six of those ten are **already** `Short*` types. So the remaining work at the top of the profile is
not conversion — it is choosing the right capacity, and asking why some of these are constructed a
billion times at all. `createDefaultSubstitutionsIfNeeded` (`slang-ast-decl-ref.cpp:912`) builds its
local list 1.03 billion times in one run, and since that happens after an early return it was called
at least that often. That is a fact about the substitution machinery, which the container statistics
can only point at.

Against `findings.md`'s test-suite figures the whole addressable opportunity has grown by two orders
of magnitude, and its memory cost by more than that:

|                       | Sites | Allocations avoidable | Inline memory added |
| --------------------- | ----: | --------------------: | ------------------: |
| tests (`findings.md`) |   622 |            10,645,408 |              20.7MB |
| **falcor**            |   626 |       **868,374,825** |       **4,255.8MB** |
| bench                 |   533 |            55,422,408 |           2,472.0MB |

The same number of sites, 82x the allocations, 206x the memory. That divergence is finding 2 below.

## 2. `Val::m_operands`: the earlier recommendation does not survive

`findings.md` called this "the largest single finding, and a genuine trade-off", recommending
`ShortList<ValNodeOperand, 8>` at a cost of 111.8MB. On a real application that trade is far worse in
**both** directions, and the conversion should not be made as described.

|                      |     tests |    core |      **falcor** |      bench |
| -------------------- | --------: | ------: | --------------: | ---------: |
| instances            | 9,589,393 | 915,736 | **180,158,737** | 14,439,011 |
| live at high-water   |   141,213 | 915,736 |  **16,911,543** |  9,729,072 |
| fits in 8            |     99.8% |   99.7% |       **91.4%** |      90.5% |
| inline memory at C=8 |    17.2MB | 111.8MB |     **2,064MB** |    1,188MB |

The full sweep on falcor, where `ValNodeOperand` is 16 bytes:

| Capacity |    Fit | Bytes added per `Val` | Inline memory at the peak |
| -------: | -----: | --------------------: | ------------------------: |
|        2 |  34.6% |                    32 |                     516MB |
|        4 |  62.9% |                    64 |                   1,032MB |
|        8 |  91.4% |                   128 |                   2,064MB |
|       16 | 100.0% |                   256 |                   4,129MB |

Two things went wrong with the earlier reading, and both are properties of the corpus rather than of
the instrumentation.

**Falcor's `Val`s have more operands.** Slang's own tests fit in 8 slots 99.8% of the time; Falcor's
fit 91.4%. Nine per cent of 180 million is 15.5 million promotions, so the conversion does not even
deliver what it promises at C=8 — the report's own recommender picks C=16 here, doubling the cost
again.

**Falcor keeps `Val`s alive.** 16.9 million were live simultaneously in one process, out of 24.8
million ever constructed in it: 68% of everything the process built was still alive at exit, because
`Val`s are owned by the `ASTBuilder` and a host that keeps sessions and modules loaded never frees
them. The test suite's 141,213 was an artefact of 297 short-lived processes each throwing everything
away. bench, a single process, shows the same 67% survival independently.

So the honest statement is: converting `Val::m_operands` buys up to 180 million allocations and costs
between half a gigabyte and four gigabytes of resident memory in a long-running host, depending on
capacity, and no capacity is clearly right. **It should not be done on the strength of the allocation
count.** If it is done at all, C=4 is the only defensible point (63% of the benefit for 1GB), and the
decision belongs to whoever owns Slang's memory budget for embedded use.

The number `findings.md` reported, 17.2MB, was correct for its corpus and misleading about every
other one. That is the general hazard: `liveHighWater` is the only field whose meaning depends on how
the host manages lifetime.

## 3. `ShortList`'s default capacity is 16, and that is the whole of finding 2

`findings.md` observed that "a capacity of 16 recurs at sites whose data never exceeds 2, which
suggests the number was chosen once and copied rather than measured." That diagnosis is wrong in an
instructive way. `source/core/slang-short-list.h:16` reads:

```cpp
static const Index kInitialCount = 16;
```

Nobody chose 16 at those sites. They wrote `ShortList<T>` and got 16. Every row below except
`slang-check-impl.h:3175` spells its type without a capacity argument, and every site in the profile
that does name one (`ShortList<IRInst*, 8>`, `ShortList<QualType, 8>`, `ShortList<GenericDecl*, 4>`)
is closer to right.

| Site                              | Container                           | Decl | Wants |     Instances | Slack/obj |         Total |
| --------------------------------- | ----------------------------------- | ---: | ----: | ------------: | --------: | ------------: |
| `slang-ast-decl-ref.cpp:920`      | `ShortList<GenericDecl*>`           |   16 |     2 | 1,025,086,687 |      112B | **109,491MB** |
| `slang-check-constraint.cpp:4344` | `ShortList<FlattenedTypeRangePair>` |   16 |     2 |   202,599,804 |      448B |      86,560MB |
| `slang-check-impl.h:3175`         | `ShortList<SolverConstraint>`       |    8 |     4 |   118,020,436 |      224B |      25,212MB |
| `slang-check-constraint.cpp:4346` | `ShortList<Type*>`                  |   16 |     2 |   202,599,804 |      112B |      21,640MB |
| `slang-ir.cpp:8986`               | `ShortList<IRUse*>`                 |   16 |     2 |   113,975,380 |      112B |      12,174MB |
| ... 64 more                       |                                     |      |       |               |           |               |
| **total, 69 sites**               |                                     |      |       |               |           | **339,272MB** |

339GB of inline slots constructed and never filled. That is not resident memory — it is object size
paid on every construction. bench confirms the ranking exactly, with the same sites in the same
order, totalling 26,344MB.

### Correction: what that 339GB is, and what it is not

The first version of this section concluded "lower `kInitialCount` from 16 to 4" and called it a
one-line change worth 339GB. Trying to implement it showed the conclusion does not follow, for a
reason that only appears in the container's source:

```cpp
T m_shortBuffer[shortListSize];    // slang-short-list.h:524 -- note: no `= {}`
```

The inline array is **default-initialised, not value-initialised**. For a `ShortList` of pointers
that means no code at all: the capacity costs stack footprint and cache, and nothing is written.
Only for an element type with a non-trivial default constructor does a slot cost real work. The
339GB is a sum over both cases, and they are not comparable:

| Defaulted sites    | Constructions | Footprint at 16 | At 4 | Extra promotions at 4 |
| ------------------ | ------------: | --------------: | ---: | --------------------: |
| 52 pointer-element | 1,768,962,932 |           211GB | 53GB |       **+22,808,291** |
| 11 class-element   |   294,579,329 |           121GB | 30GB |            +1,302,606 |

So lowering the default globally would trade 158GB of stack space that is _never touched_ for 22.8
million real heap allocations. That is very likely a loss, and the recommendation is withdrawn.

What survives is the class-element half, where a slot really is constructed. The largest is
`slang-check-constraint.cpp:4344`: `FlattenedTypeRangePair` holds two `FlattenedTypeRange`s and each
zeroes an index and a count, so sixteen slots is 512 bytes written on every one of 202 million
constructions — and across this profile and bench, 220 million constructions between them, the list
was empty 99.98% of the time and **never held more than one element**. That one declaration is
84.5GB of the 121GB and is safe to pin at 2.

The general lesson is that **a fit percentage alone cannot rank a `Short*` capacity.** What a slot
costs depends on the element type and on whether the container initialises its inline storage, and
`ShortList` and `ShortDictionary` differ on exactly that point — see the correction under finding 4.

## 4. The substitution cache, confirmed a third time

`source/slang/slang-ast-substitution.h:51`, `ShortDictionary<SubstitutionCache::Key, Result, 8>`:

| Profile    |       Instances | Promotion rate |  Wants |
| ---------- | --------------: | -------------: | -----: |
| tests      |         290,886 |          17.1% |     32 |
| core       |       2,210,382 |          10.0% |     64 |
| **falcor** | **537,093,597** |       **4.9%** | **32** |
| bench      |      45,446,497 |           4.9% |     32 |

The rate is lower here but the absolute count is not: 4.9% of 537 million is **26.3 million
promotions** in a single profile, each one a heap `Dictionary` allocated because eight inline slots
were not enough.

The cross-validation from `findings.md` holds a third time, and is worth restating because it is the
best evidence the accounting is sound. The same source line also produces a `Dictionary` record — the
`ShortDictionary`'s overflow member, which forwards its site. The two records have identical instance
counts (537,093,597), and the `Dictionary`'s `everAllocated` is 26,275,248 against a predicted
4.9% × 537,093,597 = 26.3 million. Two independently maintained counters agree to four significant
figures. bench reproduces the same agreement at 45,446,497 and 2,207,026.

The trap `findings.md` warned about is worth restating, because this profile makes it far more
tempting: the overflow `Dictionary` now appears as the **top `Dictionary` conversion candidate in the
whole report**, worth 88% of all `Dictionary` benefit. "Convert it to a `ShortDictionary`" is not a
coherent change — it already is the overflow of one.

### Correction: 8 is already right; do not raise it

The first version of this section said "the action is unchanged: raise the capacity from 8 to 32",
carrying `findings.md`'s recommendation forward on the strength of 26.3 million promotions. That is
wrong here, and the reason is again in the container's source — where, unlike `ShortList`,
`ShortDictionary` **does** value-initialise:

```cpp
TKey m_inlineKeys[kInlineCapacity] = {};      // slang-short-dictionary.h:142
TValue m_inlineValues[kInlineCapacity] = {};
```

So on this site every slot is 32 bytes actually written, on every one of 537 million constructions:

|        Capacity |     Promotions | Inline bytes zeroed |
| --------------: | -------------: | ------------------: |
|               2 |    148,255,560 |                32GB |
|               4 |    147,487,807 |                64GB |
| **8 (current)** | **26,275,248** |           **128GB** |
|              16 |      4,829,030 |               256GB |
|              32 |        300,865 |               512GB |

Raising 8 → 32 spends **384GB of additional zeroing to avoid 26 million heap allocations**. At any
plausible rate for `memset` against `malloc`/`free` that is an order of magnitude the wrong way
round. Raising to 16 is the same trade at half scale and also loses.

8 is at the knee of the distribution: the step from 4 to 8 buys 121 million fewer promotions for
64GB, which is the one step that pays. **Leave it at 8.**

This is the same error the report identifies for `Val::m_operands` in finding 2 — reading a
promotion count without the per-construction cost beside it — made two sections later in the same
document. The earlier profiles did not expose it because `tests` and `core` construct this cache
290,886 and 2,210,382 times, three orders of magnitude below Falcor, so the per-construction term
was genuinely negligible there. It is not negligible here.

## 5. Strings: a different population, and the largest free wins in the report

153,037,280 string-buffer allocations, against 4.4 million in the test suite. Symbolising the sampled
call stacks and rolling them up by subsystem:

| Subsystem                                     | Allocations | Share | Fit@16 | Fit@32 |
| --------------------------------------------- | ----------: | ----: | -----: | -----: |
| generic-specialisation linkage names          |  37,700,544 | 24.6% |    15% |    29% |
| name mangling                                 |  28,268,608 | 18.5% |    72% |    72% |
| `NamePool::getName`                           |  21,885,632 | 14.3% |    86% |    96% |
| diagnostics (sink construction + rendering)   |  18,332,416 | 12.0% |    46% |    48% |
| IR linking / mangled-name maps                |  16,820,416 | 11.0% |     1% |    18% |
| `ASTBuilder` builtin lookup by literal        |  16,006,784 | 10.5% |     3% |   100% |
| source locations, parser, AST printing, other |  14,022,144 |  9.2% |      — |      — |

bench agrees on every share within four points except the first two, which swap.

`findings.md` concluded that a small-string optimisation looked compelling on the core module and
unimpressive on the test suite, and that the entire difference was
`buildMangledNameToGlobalInstMap`. On a real corpus that function is only 5.6% of the total and the
picture resolves differently: **a general inline buffer is still not the answer, but four specific
callers allocate purely to perform a lookup, and fixing them costs nothing.**

### 5a. `emit(ManglingContext*, String const&)` — 20.3M allocations, free

```cpp
void emit(ManglingContext* context, String const& value)   // slang-mangle.cpp:31
```

Every call site passing a literal — `emit(context, "GP")`, `emit(context, "X")`,
`emit(context, "I")` — constructs a heap `String` for a two-character constant and destroys it. The
three sampled stacks inside `emitQualifiedName` account for 15,302,784, 3,092,608 and 1,950,656
allocations, **all at fit@16 = 100%**, which is exactly what a two-character literal looks like.

Taking `UnownedStringSlice` instead, or adding a `const char*` overload, removes 20 million
allocations per run at no memory cost and with no design question attached. There are six such call
sites in `slang-mangle.cpp`. This is the largest zero-cost finding in any profile taken so far, and
it was invisible before: the same site was ~199K allocations in the core module.

### 5b. `SharedASTBuilder::findMagicDecl(String const&)` — 16.0M allocations, free

```cpp
Decl* findMagicDecl(String const& name);                    // slang-ast-builder.h:57
auto decl = m_sharedASTBuilder->findMagicDecl(builtinMagicTypeName);  // builtinMagicTypeName is const char*
```

Identical shape. `getBuiltinDeclRef` (10,121,024 + 1,196,032 allocations) and
`isDifferentiableInterfaceAvailable` (4,689,728) each build a heap `String` from a compile-time
literal solely to probe a dictionary, then throw it away. fit@32 is 100% — these are names like
`"DifferentiableType"`. A heterogeneous lookup keyed on `UnownedStringSlice` removes all of them.

### 5c. `NamePool::getName` — 21.9M allocations, free

`findings.md`'s recommendation, unchanged and now the largest single caller in the report
(19,912,320 at `slang-name.cpp:27` at fit@16 = 89%, plus ~2M more). A `String` is built from an
`UnownedStringSlice` only to probe `Dictionary<String, RefPtr<Name>>` and is then discarded. It has
appeared at the top of every profile taken.

**5a, 5b and 5c are the same bug three times**: a hash-map lookup whose key type forces an
allocation. Together they are **58 million allocations per run, 38% of all string traffic**, and all
three are fixed by the same technique with no memory cost and no trade-off to weigh.

### 5d. Generic-specialisation linkage names — 37.7M allocations, needs a design decision

The single largest string consumer, and entirely absent from the earlier profiles.
`specializeLinkageDecoration` (`slang-ir-clone.cpp:143`) runs per specialised instruction and
allocates, every time:

- `StringBuilder sb` at `:152` and `StringBuilder specLinkName` at `:160` — 5,445,568 and 5,457,088
  allocations, each **1024 bytes**, because `StringBuilder`'s `InitialSize` is 1024;
- inside `getSpecializedLinkageName`, one `StringBuilder typeNameHint` per generic argument at
  `:133` — 10,391,296 allocations, also 1KB each;
- `digestBuilder.finalize().toString()` at `:139` — 16,371,904 allocations.

Falcor's shaders are generic-heavy, so this path dominates in a way Slang's tests never showed. The
1KB `StringBuilder`s are pure waste here — a mangled-name fragment is nowhere near a kilobyte — and
either a smaller `InitialSize` or a reused buffer removes 21,293,952 kilobyte allocations.

### 5e. `DigestUtil::digestToString` — the reason 9% of allocations are regrowths

`findings.md` recorded that only 0.6% of string allocations replaced an existing buffer, so the
doubling growth policy was "almost never exercised". Here it is 9.0% — 13,738,133 regrowths — and
**79% of them come from one function**:

```cpp
String str;                                    // slang-crypto.cpp:26
for (SlangInt i = 0; i < digestSize; ++i) {
    str.append(hex[data[i] >> 4]);             // 40 single-character appends for SHA-1
    str.append(hex[data[i] & 0xf]);
}
```

A 40-character hex string built one character at a time, starting from the 16-byte minimum: three
allocations where one would do. This is why `digestToString` appears twice in the caller ranking with
fit@16 of 100% and 0% — the first allocation fits, the regrowths do not. One `reserve(2 * digestSize)`
removes about 11 million allocations.

### 5f. `StringBuilder`'s 1024-byte `InitialSize`, again

`findings.md` noted this for `ManglingContext`. The same constant now accounts for six distinct
callers inside the top twenty: `ManglingContext` (5,102,080), `getSpecializedLinkageName` and
`specializeLinkageDecoration` (21,293,952 between them), `DiagnosticSink` (2,547,840),
`ASTPrinter::getDeclSignatureString` (1,203,648), `getNameForNameHint` (1,194,880). Roughly **31
million kilobyte allocations per run** attributable to one default that nothing measured.

### 5g. `DiagnosticSink` is expensive to construct, and is constructed per overload resolution

`SemanticsVisitor::ResolveInvoke` (`slang-check-overload.cpp:3458`) constructs a `DiagnosticSink`
1,600,128 times, and `SemanticsContext::withSink` a further 947,712. Each sink carries a
`StringBuilder outputBuffer` (`slang-diagnostic-sink.h:448`, 1KB) and a
`Dictionary<int, Severity> m_severityOverrides` that the container ranking lists separately —
1,611,405 instances, 1,415,314 avoidable allocations, and a perfect fit at C=8. Two allocations per
overload resolution for a sink that in the common case reports nothing.

## 6. The one `Short*` capacity that is too small

`slang-ir-legalize-types.cpp:211`, `LegalCallBuilder::m_args`, declared `ShortList<IRInst*>` and so
inheriting the default 16:

| Profile | Instances |  Promoted | Wants |
| ------- | --------: | --------: | ----: |
| falcor  |    12,859 | **34.5%** |    32 |
| bench   |       299 | **59.9%** |    32 |

The only site in either profile where the inline array fails at its job. It is small in absolute
terms and is listed for completeness and because it is the exact opposite of finding 3 — the one
place the default is too low rather than too high.

## 7. Confirmed without change

- **`ImmutableHashedString` should not get an inline buffer.** 77,302,058 constructions, of which
  only 11.1% build a buffer at all and only 17.1% would fit in 32 characters (0.6% in 16). The
  earlier profiles said 29.6% and 16.1%; the real corpus is _worse_, because an even larger share of
  the population is mangled names. Four profiles now agree, and the intuition that interned
  identifiers are short remains wrong.
- **Containers constructed, never used, destroyed.** 276 sites are empty in ≥99% of instances. New
  entries at this scale: `slang-ir.cpp:9057` `List<IRSetBase*>` in `_replaceInstUsesWith`
  (38,798,612), `slang-capability.cpp:543` — the `CapabilitySet` default constructor's map —
  (35,918,686), and `slang-check-inheritance.cpp:303` (15,170,931). The `slang-allocator.h:100`
  triple is here too at 26,017,792 each, still library code standing in for many callers.
- **Blocked by a single operation.** 238 sites, headed by `slang-uint-set.h:67` `List<Element>` at
  382,680,354 instances blocked only by `getBuffer()`. In the test suite the sibling at `:66` led
  with 6.5M; here `:67` leads with 382M. `UIntSet`'s backing list is now the sixth-largest distinct
  declaration in the entire profile, and the obstacle is still one public accessor.
- **Sites that should keep their hash table.** `slang-check-impl.h:980`
  `Dictionary<Decl*, uint64_t>` performs **6.29 billion lookups**. That single dictionary is doing
  more work than most of the rest of the profile combined, and nothing in this report should be read
  as suggesting it be changed.

## What to distrust in these numbers

Everything in `findings.md`'s equivalent section still applies. Four things are specific to this
corpus:

- **The compiler could not compile the corpus.** These dumps are against 2026.17.1-14-g4d7939e59,
  which fails on Falcor 2's path tracer and several `render/*.slang` tests; roughly 350 test failures
  are that. The data is whatever ran before the error, so coverage is tilted towards the front end
  and away from late IR passes and emit. The bench profile is worse: all 14 of its tests failed.
  The measurable effect is smaller than it sounds — diagnostics account for **1.0% of container
  events** in both profiles — but it is **12% of string allocations**, so section 5's diagnostics row
  is inflated and the emit-side of the profile is thin. Nothing else in this report rests on either.
- **`liveHighWater` is merged by taking the maximum, not the sum.** The 4,129MB in section 2 is one
  process's peak, not nineteen processes added together (which would be 63 million live `Val`s). That
  is the right choice, and it means the memory column is a real single-process figure.
- **The per-process spread is wide.** One worker held 16.9 million live `Val`s; its siblings held
  1.6 to 8.2 million. Peak-memory conclusions rest on the worst process, which is the right one to
  plan for but is not typical.
- **An inline capacity cannot be ranked from the fit percentage alone**, which is what
  `analyze.py`'s `C` column and `wants` column both do. What a slot costs differs by container and
  by element type: `ShortDictionary` value-initialises its inline arrays and `ShortList` does not,
  and within `ShortList` a slot of pointers costs nothing to construct while a slot of a class with
  a user-written default constructor costs real stores. Two of this report's first-version
  recommendations were wrong for exactly this reason, and both are corrected in place above. Read a
  `Short*` recommendation as "look at this site", never as "make this change".
- **A `StringBuilder`'s recorded size is what it asked for, not what it held.** The constructor
  requests `InitialSize` up front, so every one of these sites reports 1024 characters regardless of
  its contents. That is why section 5f can say what the default costs but not what it should be.
- **The event total double-counts string allocations.** Each one produces a per-line record and,
  one time in 64, a sampled stack record that is scaled back up, so both appear in the family table.
  153,037,280 is the true count; the family table's 278,631,179 is not. This inflates the headline
  event total by about 1.7% and applied equally to the earlier reports.

## Reproducing

```bash
python3 tools/container-stats/analyze.py '/tmp/myprof/falcor.*.json'    --top 40
python3 tools/container-stats/analyze.py '/tmp/myprof/benchmark.*.json' --top 40
```

Five of the nineteen falcor dumps are ~84KB: pytest-xdist controller processes that barely touch the
compiler. They merge harmlessly.

One operational note, because it cost an hour and produces a silently degraded report rather than an
error. Symbolising the sampled stacks requires that the library **at the path recorded in the dump**
resolve to one with debug info. Two things can break that independently:

- the path itself — these dumps name `/tmp/slangshim/libslang-compiler.so.0.2026.17`, a renamed copy
  made so Falcor's `libsgl.so` would load it, and that copy no longer exists;
- the separate debug file — a split-DWARF build records a `.gnu_debuglink` naming a sibling
  `.dwarf`, which `addr2line` resolves **relative to the directory of the file it was given**. A
  symlink to the real library is not enough; the `.dwarf` must be reachable from the recorded
  directory too.

With either missing, `addr2line` still answers, with `??:?`, and `analyze.py` correctly discards
those frames — so the string-caller section prints counts with no names and no warning. Restoring
both symlinks turns section 5 from unusable into the most informative part of the report:

```bash
mkdir -p /tmp/slangshim
ln -s .../libslang-compiler.so.0.2026.17.1       /tmp/slangshim/libslang-compiler.so.0.2026.17
ln -s .../libslang-compiler.so.0.2026.17.1.dwarf /tmp/slangshim/libslang-compiler.so.0.2026.17.1.dwarf
```

## What to do, in order of value per unit of risk

This list has been revised after implementation; two entries in the first version were withdrawn,
and the corrections under findings 3 and 4 say why.

1. **The three allocate-to-look-up sites (5a, 5b, 5c).** 58 million allocations per run, no memory
   cost, no design question. Mechanical and independently verifiable.
2. **`digestToString`'s missing `reserve`.** ~11 million regrowths, three lines.
3. **The 1KB `StringBuilder`s on the specialisation path (5d).** Two of the three were removable
   outright rather than resizable: one existed only to be appended to another, and one was
   constructed per generic argument where a single reused buffer serves the loop.
4. **`slang-check-constraint.cpp:4344`'s inline capacity: 16 → 2.** The one `ShortList` site where
   the slots are genuinely constructed and genuinely unused — 512 bytes written per construction,
   202 million times, for a list that never held more than one element.
5. **`slang-ir-legalize-types.cpp`'s `LegalCallBuilder::m_args`: 16 → 32.** The only measured
   `Short*` capacity that is too small rather than too large.
6. **`DiagnosticSink` construction cost**, given that it happens once per overload resolution. Not
   attempted: the right fix is to allocate its buffer lazily, not to resize it, and the profile
   records what the buffer asks for rather than what it holds so it cannot size one.
7. **Leave `Val::m_operands` alone** until someone owns a memory budget for embedded use. This is a
   decision, not an optimisation.
8. **Leave `ShortList`'s `kInitialCount` and the substitution cache's capacity alone.** Both were
   recommended in the first version of this report and both are withdrawn; see the corrections.
