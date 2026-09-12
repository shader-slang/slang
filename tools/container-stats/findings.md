<!--
SPDX-FileCopyrightText: The Khronos Group, Inc.
SPDX-License-Identifier: CC-BY-4.0
-->

# What the container statistics found

Results from two profiles taken with `SLANG_ENABLE_CONTAINER_STATS=ON`, a Debug build:

| Profile | Workload | Processes | Records | Events |
| --- | --- | ---: | ---: | ---: |
| **core** | `slang-bootstrap -compile-core-module`, a single 6-second front-end-heavy run | 1 | 1,020 | 36,966,155 |
| **tests** | the whole `slang-test` suite, 9,535 tests | 297 | 1,843 | 74,471,168 |

Both are reproduced by the commands in `README.md`. Every figure below is from one of these two
and is labelled with which; where they disagree, that disagreement is usually the finding.

## The opportunity is concentrated, not spread out

Converting every eligible site at its recommended capacity:

| | Sites | Allocations avoidable | Inline memory added |
| --- | ---: | ---: | ---: |
| tests | 622 | 10,645,408 | 20.7MB |
| core | 287 | 3,073,880 | 189.3MB |

In the test suite **one site is 89% of the total**. There are 497 convertible `List` sites; the 482
outside the top fifteen are worth 118,218 allocations between them — about 1%. `Dictionary`
contributes 61,486 across 50 sites and `HashSet` 35,477 across 75.

So the useful conclusion is not "convert containers to `Short*`". It is that a short list of
specific declarations carries nearly all of the benefit, and a general conversion campaign would be
mostly churn.

## 1. `Val::m_operands` — the largest single finding, and a genuine trade-off

`source/slang/slang-ast-base.h:480`, a `List<ValNodeOperand>` member of `Val`. The record resolves
to the class line 382 because a member field reports its enclosing class, as documented in
`README.md`.

| | tests | core |
| --- | ---: | ---: |
| instances | 9,589,393 | 915,736 |
| live at high-water | 141,213 | **915,736** |
| fits in 8 | 99.8% | 99.7% |
| allocations avoided at C=8 | 9,432,567 | 912,982 |
| inline memory added | 17.2MB | **111.8MB** |

`ValNodeOperand` is 16 bytes, so `ShortList<ValNodeOperand, 8>` adds **128 bytes to every `Val`**.

The two profiles disagree sharply on what that costs, and the core module is the honest one: there
`liveHighWater == instances`, meaning every `Val` constructed during the compile is still alive at
the end. `Val`s are owned by the `ASTBuilder` and are not freed as compilation proceeds, so the
inline arrays accumulate. 111.8MB is resident, not transient.

Note that `ValNodeDesc::operands` at `slang-ast-base.h:248` is *already*
`ShortList<ValNodeOperand, 8>`, so the pattern is established; `Val::m_operands` is the one that
was left as a plain `List`.

The sweep offers a middle option: C=4 keeps 94-95% of the benefit for 64 bytes per `Val` instead of
128. This site is worth doing, but it is a memory-for-allocations trade that should be decided
deliberately, not waved through because 9.4M is a big number.

## 2. Existing `Short*` capacities are systematically too large

This is the finding that the per-site tables hide, because no individual site looks alarming.

| Profile | Sites well sized | Larger than needed | Cumulative unused footprint |
| --- | ---: | ---: | ---: |
| tests | 2 | 64 | 798MB |
| core | ~1 | 51 | 924MB |

The cumulative figure is unused inline bytes summed over every construction. It is not resident
memory: it is object size, paid on every construction, in stack frames and in cache.

| Site | Container | Declared | Wants | Instances (tests) | Slack per object |
| --- | --- | ---: | ---: | ---: | ---: |
| `slang-ast-base.h:241` | `ShortList<ValNodeOperand, 8>` | 8 | 4 | 11,732,485 | 64B |
| `slang-check-constraint.cpp:4344` | `ShortList<FlattenedTypeRangePair, 16>` | 16 | 2 | 37,647 | 448B |
| `slang-ast-decl-ref.cpp:920` | `ShortList<GenericDecl*, 16>` | 16 | 2 | 148,655 | 112B |
| `slang-check-constraint.cpp:1038` | `ShortList<SolverConstraint, 16>` | 16 | 4 | 15,633 | 672B |
| `slang-check-impl.h:3175` | `ShortList<SolverConstraint, 8>` | 8 | 4 | 37,201 | 224B |

A capacity of 16 recurs at sites whose data never exceeds 2, which suggests the number was chosen
once and copied rather than measured. Three of these — `slang-check-constraint.cpp:4344`, `:4345`
and `:4346` — are also in the always-empty list below: they declare 16 inline slots and are empty
in over 99% of instances.

## 3. Exactly one capacity is too small, and it cross-validates the instrumentation

`source/slang/slang-ast-substitution.h:51`, `ShortDictionary<SubstitutionCache::Key, Result, 8>`:

| Profile | Instances | Promotion rate | Wants |
| --- | ---: | ---: | ---: |
| tests | 290,886 | 17.1% | 32 |
| core | 2,210,382 | 10.0% | 64 |

It is the only site in either profile promoting in more than 5% of instances.

The same source line also produces a `Dictionary` record, which is the `ShortDictionary`'s overflow
member — `ShortDictionary` forwards its site to the dictionary it spills into, so both carry the
same location. The two records have *identical* instance counts (290,886 and 2,210,382), and the
`Dictionary`'s "49,850 allocations avoidable" matches 17.1% × 290,886 = 49,741 promotions to within
merge rounding.

That agreement between two independently recorded families is worth more than the finding itself:
it is evidence the accounting is sound. It is also a trap. Read without knowing about the
forwarding, the `Dictionary` record looks like the top independent `Dictionary` conversion
candidate, and "convert it to a `ShortDictionary`" is not a coherent change — it is already the
overflow of one. **The action is to raise the existing capacity from 8 to 32, which removes those
promotions and the overflow dictionary allocations together.**

## 4. Containers constructed, never used, destroyed

175 sites in the test suite are empty in at least 99% of instances.

| Site | Container | Constructions (tests) |
| --- | --- | ---: |
| `slang-ast-base.h:765` | `List<ProvenenceNodeWithLoc>` | 2,768,166 |
| `slang-dictionary.h:710` | `OrderedDictionary<Decl*, RequirementWitness>` | 2,307,728 |
| `slang-ast-decl.h:76` | `Dictionary<Name*, Decl*>` | 1,287,476 |
| `slang-ast-decl.h:76` | `List<Decl*>` | 1,287,476 |
| `slang-allocator.h:100` | `List<void*>`, `Dictionary<void*,void*>`, `HashSet<void*>` | 932,864 each |

These want removal or lazy allocation rather than conversion. Two cautions. `slang-allocator.h:100`
is the `new (rs + i) T();` inside `allocateArray`, so it is library code standing in for many
callers — the backtrace names the real one. And an always-empty container may be a feature the test
suite does not exercise rather than dead weight, so each needs reading before removal.

## 5. Blocked by a single operation

56 sites stay within 8 elements in over 95% of instances and are disqualified by exactly one
operation. These are interesting because the obstacle is one call, not the shape of the data.

| Site | Container | Instances | Blocked by |
| --- | --- | ---: | --- |
| `slang-uint-set.h:66` | `List<Element>` | 6,511,579 | `contiguousBuffer` |
| `slang-uint-set.h:67` | `List<Element>` | 439,719 | `contiguousBuffer` |
| `slang-serialize-source-loc.cpp:281` | `List<LineInfo>` | 43,637 | `sort` |
| `slang-check-impl.h:3175` | `Dictionary<Decl*, Val*>` | 37,201 | `indexUpdate` |

`UIntSet`'s backing list is the standout at 6.5M instances, held back only by its public
`getBuffer()`.

## 6. Strings: two populations that must not be averaged

| Inline capacity | core | tests |
| ---: | ---: | ---: |
| 4 | — | 2.8% |
| 8 | **52.3%** | 7.3% |
| 16 | 61.5% | 18.3% |
| 24 | 74.5% | 27.0% |
| 32 | 75.8% | 33.9% |
| 64 | — | 49.9% |

A small-string optimisation looks compelling on the core module and unimpressive on the test suite.
The entire difference is one function:

```
~2,899,392 of 4,417,531 test-suite allocations (66%), 0% of which fit in 16 characters
   String::String(UnownedStringSlice const&)
   UnownedStringSlice::getHashCode()
   IRModule::buildMangledNameToGlobalInstMap()   at slang-ir.cpp:5177
```

`m_mapMangledNameToGlobalInst` is a `Dictionary<ImmutableHashedString, List<IRInst*>>` filled by
`m_mapMangledNameToGlobalInst[linkageDecor->getMangledName()]`. `getMangledName()` returns an
`UnownedStringSlice` into the `IRStringLit`'s own storage — already owned by the module and
outliving the map — but `ImmutableHashedString` copies it into a fresh heap `String`. The map is
`clear()`ed and rebuilt in full on each call, so every rebuild re-copies every mangled name.
Mangled names encode type signatures and are long, so no inline buffer of any plausible size helps.

Excluding that one function, the test suite looks like the core module.

Two further callers, both of which allocate a `String` purely to look one up:

| Caller | Allocations | Fits in 16 |
| --- | ---: | ---: |
| `NamePool::getName` (`slang-name.cpp:27` and `:32`) | ~749K (tests), ~325K (core) | 66-92% |
| `emitQualifiedName` (`slang-mangle.cpp:560`) | ~199K (core) | 100% |
| `ManglingContext::ManglingContext` | ~42K (core) | 0% |

`NamePool::getName` builds a `String` from an `UnownedStringSlice` only to probe a
`Dictionary<String, RefPtr<Name>>` and then discards it; a heterogeneous lookup removes the
allocation outright, with no memory cost and no inline buffer. It is the one string finding that
appears at the top of every profile taken so far.

The third is different in kind: `StringBuilder`'s `InitialSize` is 1024, so every `ManglingContext`
allocates a kilobyte up front regardless of what it goes on to hold.

## 7. `ImmutableHashedString` — a measured negative result

Of 9,729,681 constructions in the test suite, only 29.6% build a buffer at all (the rest share an
existing one), and only 16.1% would fit in 32 characters. The population is dominated by mangled
names from the map above, not by identifiers.

**An inline buffer on `ImmutableHashedString` is not indicated.** This was worth measuring
precisely because the intuition — "interned identifiers are short" — is wrong here.

## What to distrust in these numbers

- **Per-caller string counts are sampled 1 in 64** and scaled, so they are estimates. Totals and
  per-line records are exact. Rankings between callers are sound; absolute figures are not.
- **Member fields resolve to their class**, so two fields of the same type in one class merge into
  one record. `analyze.py` prints a warning when it detects this.
- **The operation mask understates contiguity for `List`.** `begin()`/`end()` hand out a `T*` and
  are deliberately not recorded, or the disqualifying bit would be set nearly everywhere. Every
  `List` candidate must be read before being converted.
- **`reserve` records the call, not its argument.** A site that reserves 1,000 and peaks at 8 is
  indistinguishable from one that reserves 8, so a `reserve` note is a prompt to read the code.
- **`liveHighWater` is a maximum, not an average**, so the inline memory column is a worst case —
  except where `liveHighWater == instances`, as with `Val`, where nothing is ever freed and the
  worst case is the real case.
- **Both profiles are Debug builds.** The statistics are independent of optimisation level, but the
  cost of collecting them (1.07x) is understated relative to a release build.

## What the instrumentation does not measure

- **Cache effects.** Inline storage improves locality; that benefit is real and is not counted
  here, so conversions may be worth more than the allocation count suggests.
- **Destruction cost**, which inline storage also reduces.
- **`reserve` arguments**, as above.
- **When a high-water mark occurred**, so short-lived and long-lived populations cannot be
  distinguished from the peak alone.
- **`mdl_dxr`**, the one compile-perf workload needing a corpus that is not checked in, and any
  real-world shader corpus. Both profiles here are Slang's own code and tests, which may not
  resemble what users compile.

## An independent check

These datasets and `analyze.py` were given to a separate agent with no indication of what had
already been found. It independently reached findings 1, 4, 6 and 7, including the conclusion that
`ImmutableHashedString` should *not* get an inline buffer, and the `NamePool` heterogeneous-lookup
recommendation.

It missed finding 2 entirely, and made two errors worth recording because they are the failure
modes this data invites:

- It computed `Val`'s inline cost as "~2 bytes per instance" by dividing 18MB by 9.5M
  *constructions*, when the divisor is 141,213 *live* objects and the true figure is 128 bytes each.
  It also used only the test-suite profile and so never saw the 111.8MB core-module cost.
- It recommended converting the `Dictionary` at `slang-ast-substitution.h:51` into a
  `ShortDictionary`, not recognising it as the overflow member of the `ShortDictionary` already
  declared at that line.

Both mistakes come from reading a single number without the structure around it, which is the main
risk in using this data.
