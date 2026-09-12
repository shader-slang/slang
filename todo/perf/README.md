# Hashing / hash-container performance work

This directory collects hashing-related performance issues found in a review of
`source/core/slang-hash.h`, `source/core/slang-dictionary.h`,
`source/core/slang-hashmap-impl.h`, and their consumers across the compiler.

Every issue here is **independent of which hash function or hash map
implementation is selected** by the `SLANG_HASH` / `SLANG_HASHMAP` CMake options.
They are costs that every combination in that matrix pays.

Each file is written to be self-contained: it restates the background you need,
quotes the relevant code, states the evidence, proposes a change, and describes
how to validate it. They can be picked up in any order unless a "Depends on"
section says otherwise.

---

## Index, by how each relates to the benchmark matrix

The categories below say _how each issue relates to the matrix_. For _when to
land each one_, see [Suggested commit stack](#suggested-commit-stack) — the two
groupings are deliberately different, because a couple of matrix-critical items
are also the riskiest and want their own PR.

### Category 1 — Hard blockers: these change the _ranking_ of the matrix

Do these before the results are used to pick a default. Both are cheap.

| #                                        | Title                                                                          | Size | Regression risk                         |
| ---------------------------------------- | ------------------------------------------------------------------------------ | ---- | --------------------------------------- |
| [01](01-combine-hash-no-finalisation.md) | `combineHash` has no finalisation; low bits of every structured key degenerate | M    | **Real** — see "Regression risk" below  |
| [14](14-khasuniformhash-audit.md)        | The three `kHasUniformHash` markers disable the map's own mixing               | S    | **Real** — adds a remix per string hash |

Why they are blocking:

- **01** directly reorders the `SLANG_HASHMAP` axis. ankerl
  (`unordered_dense.h:896-910`) and boost (`foa/core.hpp:1428-1434`) remix a
  non-avalanching hash; absl and `tsl::robin_map` do not. `tsl_robin` will score
  badly for a reason that is Slang's fault, not tsl's.
- **14** means ankerl honours the `kHasUniformHash` marker while boost ignores it
  (boost keys off `boost::hash_is_avalanching`, not off the `is_avalanching`
  typedef). So for every string-keyed map, those two backends are effectively
  hashing differently.

**[15](15-structured-keys-bypass-slang-hash-axis.md) is not implementable** — it
is a framing document explaining that ~58 hand-written `getHashCode()`s never
see the `SLANG_HASH` selection at all. Read it before interpreting results;
there is nothing to land except a comment in `slang-hash-impl.h`.

### Category 2 — Change _what the matrix measures_, not (mostly) the ranking

These do not invalidate the current run's ordering, but they shift the workload
mix enough that the winner could change afterwards.

| #                                                    | Title                                        | Size | Effect on the benchmark                                                                                                                                                       | Regression risk |
| ---------------------------------------------------- | -------------------------------------------- | ---- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------- |
| [03](03-hashset-uses-dictionary-with-dummy-value.md) | `HashSet<T>` is `Dictionary<T, _DummyClass>` | M    | **Could reorder.** Entry size is a first-order input to flat-vs-node performance. Halving `HashSet<IRInst*>` from 16 B to 8 B systematically favours the flat maps.           | Low             |
| [06](06-string-hash-not-cached.md)                   | `String` never caches its hash               | M    | The matrix currently over-weights string-hash throughput. Cache it and the `SLANG_HASH` axis barely matters for strings — you could pick a hash for a reason that evaporates. | **Real**        |
| [02](02-transparent-heterogeneous-lookup.md)         | Heterogeneous lookup silently disabled       | M    | Slice lookups are currently dominated by `malloc`, masking the hash's contribution. Removing it raises signal-to-noise rather than reordering.                                | None            |

Of these, **03 is the one to worry about for ranking.** If flat-vs-node is close
in your results, do not trust that comparison until it lands. 06 and 02 are "your
conclusion may not age well" rather than "your conclusion is wrong".

### Category 3 — Pure general wins, matrix-neutral

These reduce the _count_ of map operations, or the work _outside_ the map. They
scale every configuration down roughly proportionally, so they neither reorder
nor reweight anything.

| #                                                           | Title                                                                  | Size | Regression risk                             |
| ----------------------------------------------------------- | ---------------------------------------------------------------------- | ---- | ------------------------------------------- |
| [04](04-dictionary-find-iterator-api-and-double-lookups.md) | No `find()`-returning-position API forces double probes                | L    | None                                        |
| [05](05-dictionary-insert-eager-value-type-construction.md) | Insert helpers materialise a `value_type` before probing               | S    | None                                        |
| [07](07-irconstantkey-hash-not-cached.md)                   | `IRConstantKey` rehashes and walks a decoration list per probe         | M    | Mild — key grows 8→16 B                     |
| [08](08-irsimplespecializationkey-list-key.md)              | `IRSimpleSpecializationKey` is a heap `List` with an uncached hash     | M    | **Real** if `ShortList` capacity is guessed |
| [09](09-spvinstkey-list-key-and-double-insert.md)           | `SpvInstKey` holds two `List`s, uncached hash, double insert           | M    | **Real**, same reason as 08                 |
| [10](10-irinstkey-hash-recomputed-per-operation.md)         | `IRInstKey` rebuilt per map operation; removal probes twice            | M    | Part A none; Part B **real**                |
| [11](11-findoremithoistableinst-per-operand-probes.md)      | `_findOrEmitHoistableInst` does N+1 probes and an arena alloc per inst | L    | None                                        |
| [12](12-namepool-double-allocation.md)                      | `NamePool::getName` allocates the name text up to three times          | S    | None                                        |
| [13](13-ir-link-mangled-name-lookups.md)                    | IR linking allocates and rehashes mangled names per lookup             | M    | None                                        |

These do not all land at the same time: the risk-free ones go in Phase 1 of the
commit stack below, and the ones needing measurement go in Phase 4, after a
backend has been chosen.

**04 Part A is the single highest value-per-effort item in the whole set**: it
needs no API change at all, because `HashSet::add` already returns whether the
insert happened (`slang-dictionary.h:427`). It is 262 call sites of mechanical,
behaviour-preserving rewrite.

---

## Regression risk

Most of these are strictly-less-work changes. Six are genuine trade-offs and
must be measured rather than assumed. Listed worst-first.

### 01 — `combineHash` finalisation: the most likely way this whole effort produces a regression

Two compounding effects:

1. Option B (fix the fold step) adds ~2 operations per folded value. For
   `IRInstKey` on a many-operand instruction (`slang-ir.cpp:2236-2249`) that is
   measurable.
2. More importantly: **ankerl currently remixes these keys with wyhash**,
   because none of them declare `is_avalanching`. If the fold becomes good and
   you do not then mark the composite keys as avalanching, you pay good-fold
   _plus_ wyhash-remix — strictly more work than today, for distribution you
   already had.

So 01 should be landed and then immediately followed by a decision about whether
the composite keys can now declare `kHasUniformHash`. Landing 01 alone and
benchmarking it in isolation may well show a regression on the ankerl and boost
rows. Budget for both halves.

### 06 — `String` hash caching: +8 bytes on every string

`StringRepresentation` is allocated as
`sizeof(StringRepresentation) + capacity + 1` (`slang-string.h:337-346`). Most
`String`s in the compiler are never used as a dictionary key and are hashed zero
times; those pay a larger allocation and worse cache density for nothing.

This is why the issue recommends **Alternative A** (migrate the hot maps to
`ImmutableHashedString`, which already caches) rather than changing
`StringRepresentation`. Alternative A has no regression risk.

### 14 — removing/conditioning `kHasUniformHash`: +1 remix per string hash

If `String`/`UnownedStringSlice` stop claiming avalanching, ankerl starts
applying `wyhash::hash` to every string probe. That is a real added cost, taken
deliberately for better distribution.

Note the proposed form, `kHasUniformHash = HashImpl::kIsAvalanching`, is a
**no-op under `SLANG_HASH=WYHASH` and `=ABSL`** (both set
`kIsAvalanching = true`) and only costs under `=BOOST` and `=STD`. So under the
current default it is free, and it makes the other two rows honest.

### 10 Part B — caching the hash on `IRInst`: +8 bytes on every instruction

`IRInst`s are arena-allocated in enormous numbers, and
`_findOrEmitHoistableInst` hand-computes sizes as
`sizeof(IRInst) + operandCount * sizeof(IRUse)` (`slang-ir.cpp:2825`). Speeding
up hashing for the hoistable subset at the cost of memory for every instruction
could easily be a net loss. The issue says measure the operand-count
distribution first; take that seriously. **Part A of that issue has no risk and
should be done regardless.**

### 08 / 09 — `ShortList` inline storage

Moving `IRSimpleSpecializationKey::vals` and `SpvInstKey::instWords` to inline
storage makes every _stored_ entry much larger. `ShortList<IRInst*, 8>` is 64
bytes of inline payload versus a 24-byte `List` header. For a map with many
entries that trades one indirection for 40 extra bytes per slot, which can be a
net loss.

Both issues say to instrument the element-count distribution and pick the inline
capacity from the data rather than copying `ValNodeDesc`'s 8. Do that.

### 07 — `IRConstantKey` grows 8 → 16 bytes

Caching the hash doubles the key size, so the constant map's slot array doubles.
Likely still a clear win — it removes a decoration-list walk and a byte scan per
probe, and makes rehash free — but it is a trade, not a freebie.

### 03 — low but nonzero

Switching `HashSet` to the backends' real set types should be a strict
improvement. The risk is in the shim work: `clearAndDeallocate`'s
`(0, hash, eq, alloc)` constructor must exist on all eight set types, and the
`ContainerPool` recycling in `InstHashSet` must keep working.

### No regression risk

**02, 04, 05, 11, 12, 13, 10 Part A, 15.** These remove allocations, remove
redundant probes, or change documentation. There is no scenario where they cost
more than they save.

(02 carries a _correctness_ risk — a too-permissive transparent comparator turns
a "wrong key type" compile error into a silent always-miss — but no performance
risk. The issue covers the mitigation.)

---

## Rebase ordering: what can land _before_ the multi-backend commit

The "go wide" commit is `64810c098` (adds `slang-hash-impl.h`,
`slang-hashmap-impl.h`, the three submodules, the CMake options and
`extras/hashmap-matrix.sh`). Before it, `Dictionary` used
`ankerl::unordered_dense::map` directly and `Hash<T>` used
`ankerl::unordered_dense::hash` directly. `combineHash`, `Hasher` and
`kHasUniformHash` all pre-date it unchanged.

**Almost all of this work is orthogonal to that commit and should be rebased
underneath it.** That is the right shape: the go-wide commit is a
build-system/abstraction change, and these are algorithmic and representation
fixes that stand on their own merits upstream. Landing them first also means the
shim layer in `slang-hashmap-impl.h` only has to generalise APIs that have
already been proven against one backend.

### Fully rebaseable before `64810c098` — 13 of 15

| #      | Why it is backend-independent                                                                                                                      |
| ------ | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| **01** | `combineHash` in `slang-hash.h` pre-exists unchanged. Pure upstream improvement.                                                                   |
| **02** | ankerl alone also gates `find` on `is_transparent_v<H, KE>`, so the fix is identical. **Caveat below.**                                            |
| **03** | Use `ankerl::unordered_dense::set` directly; the go-wide commit then adds the other seven `Set` aliases. Strictly easier in this order.            |
| **04** | Part A is pure call-site rewrites in IR passes. Part B is _easier_ pre-go-wide: `Dictionary::begin()` returned `map.begin()` with no shim wrapper. |
| **05** | ankerl has `try_emplace`.                                                                                                                          |
| **06** | `slang-string.h`/`.cpp` only.                                                                                                                      |
| **07** | `slang-ir.h`/`.cpp` only.                                                                                                                          |
| **08** | `slang-ir-clone.h`/`.cpp` and `ShortList` only.                                                                                                    |
| **09** | `slang-emit-spirv.cpp` only. Minor textual conflict: `64810c098` touched 7 lines of that file.                                                     |
| **10** | Part A interim form (hoist the key into a local) needs nothing at all; final form needs 04 Part B.                                                 |
| **11** | Part A uses `Dictionary::getCount()`, which pre-exists. Part B needs 02.                                                                           |
| **12** | Depends only on 02.                                                                                                                                |
| **13** | Depends only on 02.                                                                                                                                |

### Must land after `64810c098` — 2 partial

- **14**, the `kHasUniformHash = HashImpl::kIsAvalanching` half.
  `HashImpl::kIsAvalanching` only exists after the go-wide commit; before it,
  ankerl was the only hash and was always avalanching, so `= true` is correct.
  The _other_ halves of 14 — normalising `SpvInstKey`'s `const static bool` to
  `static constexpr bool`, and the doc comment on `DetectAvalanchingHash`
  spelling out the contract — are fully rebaseable.
- **15**, the comment in `slang-hash-impl.h`. The general observation could go in
  `slang-hash.h` early; the part about what `SLANG_HASH` does and does not cover
  belongs with the option that introduces it.

### Caveat when doing 02 / 03 / 04B early

Write the code as if all eight backends were present, even though only ankerl is:

- **Transparent comparators must supply both argument orders.** ankerl compares
  `equal(probe, stored)`; absl and `tsl::robin_map` compare
  `equal(stored, probe)`. Supplying only ankerl's order compiles fine now and
  silently breaks the go-wide commit later. The existing comment on `ValKeyEqual`
  (`slang-ast-builder.h:193-201`) documents exactly this.
- **`erase(iterator)` returns `void` on Abseil** and an iterator elsewhere. Do
  not write `it = dict.erase(it)` in the early commits.
- **`tsl::robin_map`'s iterator dereferences to a `const` pair.** Do not assume
  `it->second` is assignable.

### Suggested commit stack

Four phases, with the benchmark run between phase 3 and phase 4. Every commit in
phases 1 and 2 is independently justifiable as an upstream PR on its own merits,
with no reference to the benchmark.

```
=== PHASE 1 — safe wins.  No measurement needed, no regression plausible. ======

 1. 04 Part A   contains-then-add -> add               262 sites, no API change
 2. 11 Part A   skip the replacement map when empty    hoist one empty() check
 3. 10 Part A   hoist IRInstKey out of                 interim form: needs
                _removeGlobalNumberingEntry           nothing at all
 4. 04 Part B   find / erase / tryEmplace on Dictionary
 5. 05          route the insert helpers through try_emplace
 6. 10 Part A   use the new API at the double-probe sites
    13 Step 3   (final form of 3, plus findSymbols)
    09 Step 1   (the SPIR-V memo double insert)
 7. 02          transparent heterogeneous lookup       both argument orders!
 8. 12          NamePool::getName                      needs 02
 9. 13          IR link mangled-name lookups           needs 02
10. 11 Part B   probe without materialising the        needs 02; largest item
                dummy IRInst                           here, safe to defer to
                                                       phase 4 if you want
                                                       phase 1 short

=== PHASE 2 — matrix prerequisites.  Neutral *as landed*, but each removes a ===
===           confound.  Land each as a unit and verify the ankerl row is   ===
===           flat before moving on.                                        ===

11. 01          combineHash finalisation          \  land together; see note
    14 partial  SpvInstKey marker + contract doc  /  below
12. 03          HashSet on a real set type             against ankerl only
13. 06 Alt A    migrate the hottest string-keyed maps  OPTIONAL, see note
                to ImmutableHashedString

=== PHASE 3 — go wide ==========================================================

14. 64810c098   rebased on top of all of the above
15. 14 rest     kHasUniformHash = HashImpl::kIsAvalanching
    15          the SLANG_HASH scope comment in slang-hash-impl.h

=== >>> RUN THE MATRIX.  PICK A BACKEND.  PIN THE DEFAULT. <<< =================

=== PHASE 4 — needs measurement.  Each may be rejected on its own numbers.  ====

16. 07          IRConstantKey hash caching             key grows 8 -> 16 B
17. 08          IRSimpleSpecializationKey              measure count distribution
18. 09          SpvInstKey rest (cached hash, inline)  measure count distribution
19. 10 Part B   cache the hash on IRInst               +8 B on every instruction
20. 06 full     cached hash in StringRepresentation    +8 B on every string
21. 11 Part B   if deferred from phase 1
```

#### Notes on the phase boundaries

**Why 01 and 14-partial land together (step 11).** Fixing the fold makes the
hash good; ankerl then remixes a hash that no longer needs remixing. Landing 01
alone can regress the ankerl and boost rows for that reason alone. Land the fold
and the avalanching-marker decision as one reviewable unit, and gate it on "the
ankerl row did not move". See the Regression risk section above.

**Why step 11 comes after phase 1, not before.** 01 changes hash values, which
changes `Dictionary`/`HashSet` iteration order, which produces test churn
wherever output depends on it. Keeping it out of phase 1 means the ten
mechanical commits stay clean, and the churn is isolated to one PR where it can
be reviewed as a determinism question in its own right. 03 has the same property
for the same reason, which is why it is step 12 rather than folded into phase 1.

**Why 03 is phase 2 and not phase 4.** Entry size is a first-order input to
flat-vs-node map performance. Benchmarking `HashSet<IRInst*>` at 16 B per entry
and then shipping it at 8 B means the flat-vs-node comparison was run on the
wrong workload. It also costs less here: you prove the `HashSet` rewrite against
`ankerl::unordered_dense::set` alone, and the go-wide commit just adds seven more
aliases.

**Why 06 Alternative A is optional.** 02 (step 7) removes the `malloc` from
string lookups, which _raises_ the weight of string hashing in the matrix; 06
removes the byte scan, which _lowers_ it again. Doing neither, or both, leaves
the weighting roughly where it is today. If you do 02 without 06, be aware the
matrix will slightly over-reward a fast string hash. The full 06 (changing
`StringRepresentation`) stays in phase 4 because of its memory cost.

**Why phase 4 is after the benchmark.** Every item there is matrix-neutral — it
reduces work outside the map, or the number of map operations — so none of them
affects which backend wins. They all carry a real or mild size/memory trade-off,
so each needs its own before/after measurement, which is easier once the backend
is pinned and the numbers stop moving underneath you.

---

## On the run currently in flight

Do not kill it. It is a valid answer to "how well does each map tolerate a weak
hash", which is useful in its own right, and it is the baseline you will diff
against after 01 lands. Just do not pick a default from it.

---

## Measurement harness used during the review

A standalone allocation-counting probe was used to confirm several of these.
It compiles against the in-tree headers and links the already-built core library:

```bash
g++ -std=c++17 -O2 -I source -I include -I external/unordered_dense/include \
    -c probe.cpp -o probe.o
g++ probe.o -o probe -L build/Debug/lib -lcore -lslang-rt
LD_LIBRARY_PATH=$PWD/build/Debug/lib ./probe
```

with a global `operator new` / `operator new[]` override incrementing a counter.

**Caveat carried into several issues below:** `Slang::List` does _not_ route its
storage through the global `operator new`, so this probe cannot see `List`
copies. Claims about `List`-valued copies in issues 05, 08 and 09 are from
reading the code, not from measurement, and should be confirmed with a profiler
or a `List`-level counter before being used to justify effort.
