# 01 — `combineHash` has no finalisation step; the low bits of every structured key are degenerate

**Status:** not started
**Estimated size:** M
**Blocks:** meaningful interpretation of the `SLANG_HASH` × `SLANG_HASHMAP` benchmark matrix

---

## Summary

`Slang::combineHash` folds values with a multiply-then-xor step and never applies
a finalisation/avalanche pass. Multiply-then-xor propagates entropy only
_upwards_. When the values being folded are pointers — which they are for almost
every structured key in the compiler — the low bits of the pointers are always
zero, so they contribute nothing to the low bits of the combined hash.

The measurable consequence is that **for a `ValNodeDesc` with a given node tag
and a given operand count, the bottom three bits of the hash are a constant**,
regardless of what the operands actually are. The same holds for any other key
built by folding aligned pointers through `combineHash` or `Hasher`.

Hash map implementations that re-mix the user hash (ankerl, boost) are immune.
Implementations that use the hash bits directly (tsl::robin_map, and to a lesser
extent absl) are not. That means the current benchmark matrix is partly measuring
_which library compensates for `combineHash`_, rather than the libraries
themselves.

---

## Background

### Where the hash for a composite key comes from

`Slang::Hash<T>` (`source/core/slang-hash.h:92-113`) dispatches like this:

```cpp
template<typename T>
struct Hash : DetectAvalanchingHash<T>
{
    auto operator()(const T& t) const
    {
        // Our preference is for any hash we've defined ourselves
        if constexpr (HasSlangHash<T>)
            return t.getHashCode();
        // Otherwise fall back to the hash provided by the selected hash library
        else if constexpr (HasLibraryHash<T>)
            return HashImpl::LibraryHash<T>{}(t);
        ...
    }
};
```

So any type with a `getHashCode()` member gets _its own_ hash, and the
`SLANG_HASH` selection (wyhash / boost / absl / std) never sees it. There are
roughly 58 such `getHashCode()` implementations in `source/` — see issue
[15](15-structured-keys-bypass-slang-hash-axis.md) for the full framing of that
point. Essentially all of them are built out of `combineHash` or `Hasher`.

### The combining primitive

```cpp
// source/core/slang-hash.h:196-209
template<typename H1, typename H2, typename... Hs>
auto combineHash(H1 n, H2 m, Hs... args)
{
    static_assert(...);
    return combineHash(
        (static_cast<std::make_unsigned_t<H1>>(n) * 16777619U) ^
            static_cast<std::make_unsigned_t<H2>>(m),
        args...);
}
```

`16777619` is the 32-bit FNV prime. The pattern is `h = (h * prime) ^ next`,
left-folded over the inputs, with a base case that returns `h` unchanged
(`slang-hash.h:185-193`). There is no final mix.

`Hasher` (`slang-hash.h:232-261`) is a thin accumulator over the same primitive:

```cpp
void hashValue(T const& value) { m_hashCode = combineHash(m_hashCode, getHashCode(value)); }
void addHash(HashCode hash)    { m_hashCode = combineHash(m_hashCode, hash); }
HashCode getResult() const     { return m_hashCode; }   // no finalisation
```

Note that `Hasher` starts at `m_hashCode = 0`, and `combineHash(0, x) == x`, so
the first value hashed is passed through completely unchanged.

### The keys that use it

Representative consumers, all of which fold _pointers_:

```cpp
// source/slang/slang-ast-val.cpp:20-34
void ValNodeDesc::init()
{
    Hasher hasher;
    hasher.hashValue(type.getTag());
    for (Index i = 0; i < operands.getCount(); ++i)
        hasher.hashValue(operands[i].values.intOperand);   // usually a Val*/NodeBase*
    hashCode = hasher.getResult();
}
```

```cpp
// source/slang/slang-ast-builder.h:139-151 (ValKey constructor)
Hasher hasher;
hasher.hashValue(v->astNodeType);
for (auto& operand : v->m_operands)
    hasher.hashValue(operand.values.intOperand);
hashCode = hasher.getResult();
```

```cpp
// source/slang/slang-ir.cpp:2236-2249
HashCode IRInstKey::_getHashCode()
{
    auto code = Slang::getHashCode(inst->getOp());
    code = combineHash(code, Slang::getHashCode(inst->getFullType()));   // IRType*
    code = combineHash(code, Slang::getHashCode(inst->getOperandCount()));
    for (UInt aa = 0; aa < argCount; ++aa)
        code = combineHash(code, Slang::getHashCode(args[aa].get()));    // IRInst*
    return code;
}
```

```cpp
// source/slang/slang-ir.h:1994-2010 (AnnotationCacheKey)
Hasher hasher;
hasher.hashValue(inst);                                // IRInst*
hasher.hashValue(static_cast<int>(associationKind));
return hasher.getResult();
```

```cpp
// source/slang/slang-ir-clone.cpp:442-451
HashCode IRSimpleSpecializationKey::getHashCode() const
{
    HashCode hash = Slang::getHashCode(valCount);
    for (Index ii = 0; ii < valCount; ++ii)
        hash = combineHash(hash, Slang::getHashCode(vals[ii]));          // IRInst*
    return hash;
}
```

---

## Evidence

### The arithmetic

`combineHash(n, m) = (n * 16777619) ^ m` in 64-bit arithmetic.

- `16777619 mod 8 == 3`, so the bottom 3 bits of `n * 16777619` are
  `(n_low3 * 3) mod 8` — a permutation of `n`'s bottom 3 bits, carrying no new
  information.
- Heap and arena pointers in Slang are at least 8-byte aligned, so `m_low3 == 0`
  for every pointer operand.
- Therefore the bottom 3 bits of the result depend only on the bottom 3 bits of
  the accumulator going in — i.e. only on whatever was hashed _before_ the
  pointers.

### Measured

Emulating `ValNodeDesc::init()` over 20 000 randomly generated 4-operand
descriptors sharing one node tag, with 8-byte-aligned operand pointers:

```
distinct low-3-bit values over 20000 random 4-operand keys with same tag: [7]
low3 by operand count: {1: [5], 2: [7], 3: [5], 4: [7], 5: [5]}
```

One single value. All of the operand entropy lives at bit 3 and above. The low
3 bits are a deterministic function of `(node tag, operand count)` alone.

(Reproduce with the Python snippet at the bottom of this file.)

### What each backend does with that

| Backend                                            | What it does with the user hash                                                                                                                                                                                                                                                        | Sensitivity                                                                                                                                                                                                             |
| -------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ankerl::unordered_dense`                          | `mixed_hash()` applies `wyhash::hash(m_hash(key))` unless the hash declares `is_avalanching` (`external/unordered_dense/include/ankerl/unordered_dense.h:896-910`)                                                                                                                     | **Immune.** `Slang::Hash<StructuredKey>` does not declare `is_avalanching`, so it is remixed.                                                                                                                           |
| `boost::unordered_flat_map` / `unordered_node_map` | `mix_policy = conditional<hash_is_avalanching<Hash>, no_mix, mulx_mix>` (`external/boost/unordered/include/boost/unordered/detail/foa/core.hpp:1429-1434`)                                                                                                                             | **Immune**, same reason.                                                                                                                                                                                                |
| `boost::unordered_map` (closed addressing)         | prime-modulus bucket count                                                                                                                                                                                                                                                             | Mostly immune.                                                                                                                                                                                                          |
| `std::unordered_map`                               | `hash % prime_bucket_count`                                                                                                                                                                                                                                                            | Mostly immune.                                                                                                                                                                                                          |
| `absl::flat_hash_map` / `node_hash_map`            | No mixing. `probe_seq::offset_ = hash & capacity_` uses the **low** bits for the probe start; `H2(hash) = hash >> 57` uses the **top** 7 bits for the control-byte fingerprint (`external/abseil-cpp/absl/container/internal/raw_hash_set.h:984-989` and the `probe_seq` constructor). | **Moderate.** Only 1/8 of slots are reachable as probe-start positions, though they remain evenly spread across the table, and the fingerprint keeps full entropy. Expect a measurable but not catastrophic regression. |
| `tsl::robin_map`                                   | No mixing. `power_of_two_growth_policy::bucket_for_hash(h) { return h & m_mask; }` (`external/robin-map/include/tsl/robin_growth_policy.h:124`), default policy per `robin_map.h:90`                                                                                                   | **Significant.** Only 1/8 of buckets are ever a starting bucket, so load concentrates 8x on those, and robin-hood linear probing has to displace entries far out from them. Expect long probe chains.                   |

So the matrix as it stands will likely rank `tsl::robin_map` (and to a lesser
extent `absl`) poorly for reasons that have nothing to do with those libraries.

---

## Why this is independent of the selected hash function

`SLANG_HASH` only selects `HashImpl::LibraryHash<T>`, which `Hash<T>` consults
_only_ when `T` has no `getHashCode()`. Every key discussed above has one.
`combineHash` is compiled into all 32 matrix configurations identically.

---

## Proposed change

Add a finalisation mix. Two options, in preference order:

### Option A — finalise in `Hasher::getResult()` and in the terminal `combineHash`

Give `combineHash`'s base case and `Hasher::getResult()` an avalanche step, e.g.
the `splitmix64`/`murmur3` finaliser:

```cpp
inline HashCode64 avalanche(HashCode64 h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}
```

Careful: `combineHash` is _also_ used as a fold step in some places where the
result is fed back in (`Hasher::addHash`), so finalising inside the fold would
change the result on every call and finalise repeatedly. It is cleaner to keep
the fold unfinalised and finalise once at the boundary — which means the
finalisation has to live in `Hasher::getResult()` **and** at every hand-written
`getHashCode()` that uses raw `combineHash` without `Hasher`. That is the main
cost of this option: ~58 call sites to audit.

### Option B — fix the fold step itself

Replace the FNV-style step with one that mixes downward, e.g.

```cpp
h = (h ^ m) * 0x9e3779b97f4a7c15ULL;
h ^= h >> 29;
```

This makes every intermediate value well-distributed, so no separate
finalisation and no call-site audit is needed. It is more arithmetic per fold
step, which matters for `IRInstKey` on many-operand instructions, but it is
still a couple of cycles.

**Recommendation: Option B.** It is a single-point change with no call-site
audit, and it makes the "is this key's hash good?" question answerable once
rather than 58 times.

### Also consider

Once the fold is good, the `kHasUniformHash` marker becomes _more_ meaningful
rather than less: a key whose hash is now genuinely avalanching could correctly
declare it and skip the backend's redundant remix. That is issue
[14](14-khasuniformhash-audit.md); do not combine the two changes in one commit,
because you want to be able to attribute the benchmark movement.

---

## Risks and things to watch

- **Nothing in Slang may depend on specific `HashCode` values.** Verify: any
  serialized format, cache key, or on-disk artifact that embeds a `HashCode`
  would change. `StableHashCode64` in `source/core/slang-stable-hash.h` is the
  type intended for stable/persisted hashing and is a _separate_ mechanism —
  confirm that `getStableHashCode64` is the only thing used for persisted
  hashes and that `HashCode`/`combineHash` values never escape a process.
  `getHashedName` in `source/slang/slang-mangle.cpp:1099` uses
  `getStableHashCode64`, which is the right precedent.
- Iteration order of every `Dictionary`/`HashSet` will change. Anywhere the
  compiler's output depends on hash-map iteration order, output will change.
  This is worth knowing about regardless — it is a latent determinism bug — but
  it will show up as test churn.
- Expect the `SLANG_HASH`/`SLANG_HASHMAP` matrix numbers to move substantially,
  particularly for `tsl_robin` and the `absl` maps. That is the point.

---

## Validation

1. Re-run the low-bit emulation (below) against the new fold; confirm the bottom
   3 bits are uniformly distributed over random operand sets.
2. Add a unit test under `tools/slang-unit-test/` that builds N distinct keys of
   a representative shape (same tag, same arity, varying pointer operands) and
   asserts the chi-squared distribution of `hash & 0x3F` is not degenerate.
   This guards the invariant going forward.
3. Re-run the full `SLANG_HASH` × `SLANG_HASHMAP` matrix via
   `extras/hashmap-matrix.sh` and compare against the pre-change numbers,
   especially for `tsl_robin` and `absl_flat`.
4. Full `sti` run to catch iteration-order-dependent test failures.

### Reproduction snippet

```python
M = (1 << 64) - 1
def comb(n, m): return ((n * 16777619) & M) ^ m

import random
tag_h = random.getrandbits(64)
lows = set()
for _ in range(20000):
    h = tag_h
    for _ in range(4):
        h = comb(h, random.getrandbits(61) << 3)   # 8-byte-aligned pointer
    lows.add(h & 7)
print(sorted(lows))    # -> [7]  : a single value
```

---

## Related

- [14](14-khasuniformhash-audit.md) — the `kHasUniformHash` markers that suppress
  backend remixing.
- [15](15-structured-keys-bypass-slang-hash-axis.md) — why the `SLANG_HASH` axis
  does not cover these keys at all.
