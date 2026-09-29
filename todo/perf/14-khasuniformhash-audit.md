# 14 — Audit the `kHasUniformHash` declarations; they disable the map's own mixing

**Status:** not started
**Estimated size:** S
**Blocks:** clean interpretation of the `SLANG_HASH` × `SLANG_HASHMAP` matrix

---

## Summary

`kHasUniformHash = true` on a type is a _promise to the hash map_ that the type's
`getHashCode()` is already well-distributed, and the map responds by skipping its
own mixing step. Three types make that promise. One of them makes it about a
value produced by `combineHash`, which — per issue
[01](01-combine-hash-no-finalisation.md) — is not avalanching.

A wrong `kHasUniformHash` is invisible in correctness testing and shows up only
as unexplained probe-length differences between hash map backends — i.e. exactly
the signal the current benchmark is trying to read.

---

## Background

### The mechanism

```cpp
// source/core/slang-hash.h:46-65
// We want to have an associated type 'is_avalanching = void' iff we have a
// hash with good uniformity, the two specializations here add that member
// when appropriate (since we can't declare an associated type with
// constexpr if or something terse like that)
template<typename T, typename = void>
struct DetectAvalanchingHash
{
};
template<typename T>
struct DetectAvalanchingHash<
    T,
    std::enable_if_t<HasLibraryHash<T> && HashImpl::kIsAvalanching>>
{
    using is_avalanching = void;
};
// Have we marked 'getHashCode' as having good uniformity properties.
template<typename T>
struct DetectAvalanchingHash<T, std::enable_if_t<T::kHasUniformHash>>
{
    using is_avalanching = void;
};
```

`Slang::Hash<T>` inherits from `DetectAvalanchingHash<T>`
(`slang-hash.h:92-93`), so the marker propagates to the functor the map sees.

### What each backend does with it

```cpp
// external/unordered_dense/include/ankerl/unordered_dense.h:896-910
// The goal of mixed_hash is to always produce a high quality 64bit hash.
template <typename K>
[[nodiscard]] constexpr auto mixed_hash(K const& key) const -> uint64_t {
    if constexpr (is_detected_v<detect_avalanching, Hash>) {
        // we know that the hash is good because is_avalanching.
        if constexpr (sizeof(decltype(m_hash(key))) < sizeof(uint64_t)) {
            // 32bit hash and is_avalanching => multiply with a constant to avalanche bits upwards
            return m_hash(key) * UINT64_C(0x9ddfea08eb382d69);
        } else {
            // 64bit and is_avalanching => only use the hash itself.
            return m_hash(key);
        }
    } else {
        // not is_avalanching => apply wyhash
        return wyhash::hash(m_hash(key));
    }
}
```

```cpp
// external/boost/unordered/include/boost/unordered/detail/foa/core.hpp:1428-1434
using mix_policy = typename std::conditional<
    boost::hash_is_avalanching<Hash>::value,
    no_mix,
    mulx_mix
>::type;
```

(Boost keys off `boost::hash_is_avalanching`, not off ankerl's `is_avalanching`
typedef, so a type marked `kHasUniformHash` is _not_ currently recognised by
boost — it will be remixed regardless. That asymmetry is itself worth noting: the
marker is honoured by ankerl but ignored by boost, so the two backends see
different effective hashes for the same key.)

absl, tsl and std do no mixing at all and ignore the marker entirely.

### The three declarations

```bash
$ grep -rn "kHasUniformHash" source/ include/ | grep -v "slang-hash.h"
source/core/slang-string.h:207:    static constexpr bool kHasUniformHash = true;     # UnownedStringSlice
source/core/slang-string.h:797:    static constexpr bool kHasUniformHash = true;     # String
source/slang/slang-emit-spirv.cpp:1934:        const static bool kHasUniformHash = true;  # SpvInstKey
```

plus the macro that sets it:

```cpp
// source/core/slang-hash.h:150-157
// Use in a struct to declare a uniform hash which doens't care about the
// structure of the members.
#define SLANG_BYTEWISE_HASHABLE                   \
    static constexpr bool kHasUniformHash = true; \
    ::Slang::HashCode64 getHashCode() const       \
    {                                             \
        return ::Slang::hashObjectBytes(*this);   \
    }
```

used at `source/slang/slang-ir-autodiff-primal-hoist.cpp:719`.

---

## Assessment of each

### `UnownedStringSlice` (`slang-string.h:207-208`) — **justified**

```cpp
static constexpr bool kHasUniformHash = true;
HashCode64 getHashCode() const { return Slang::getHashCode(m_begin, size_t(m_end - m_begin)); }
```

goes straight to `HashImpl::hashBytes`. Under `SLANG_HASH=WYHASH` and
`SLANG_HASH=ABSL` that is genuinely avalanching
(`slang-hash-impl.h:81, 109`). Under `SLANG_HASH=BOOST` and `SLANG_HASH=STD`
the file explicitly records that they are _not_:

```cpp
// source/core/slang-hash-impl.h:94-97
// `boost::hash` is the identity function for the integer types, so its low bits
// carry no more entropy than the key's do.
constexpr bool kIsAvalanching = false;
```

```cpp
// source/core/slang-hash-impl.h:122-123
// libstdc++ and MSVC both use the identity function for the integer types.
constexpr bool kIsAvalanching = false;
```

Those comments are about _integer_ hashing, and `hashBytes` for boost is
`boost::hash_range` and for std is `std::hash<std::string_view>` — both of which
are reasonable byte hashes even where the integer hash is the identity. So the
marker is probably fine. **But it is unconditional**, while
`HashImpl::kIsAvalanching` is exactly the flag that says whether the selected
implementation avalanches. The two should be connected:

```cpp
static constexpr bool kHasUniformHash = HashImpl::kIsAvalanching;
```

That is the minimal, principled fix: it makes the promise track the thing it is
promising about.

### `String` (`slang-string.h:797-801`) — **same as above**

Delegates to `UnownedStringSlice::getHashCode()`. Whatever is decided for the
slice applies verbatim.

### `SpvInstKey` (`slang-emit-spirv.cpp:1934-1950`) — **questionable**

```cpp
const static bool kHasUniformHash = true;
HashCode64 getHashCode() const
{
    const auto instWordsHash = Slang::getHashCode(...);
    const auto extraKeyDataHash = Slang::getHashCode(...);
    return combineHash(instWordsHash, extraKeyDataHash);
}
```

The two component hashes are avalanching (they are `hashBytes` results), but the
combination is `(a * 16777619) ^ b` (`slang-hash.h:196-209`), which is not an
avalanching operation. In this specific case it is _nearly_ fine — `b` is a
full-entropy 64-bit value and dominates the low bits via the xor — but the claim
is being made without justification, and it does not follow from anything stated
about `combineHash`.

Note also `const static bool` rather than `static constexpr bool`: this differs
from the other two declarations and from the `SLANG_BYTEWISE_HASHABLE` macro.
`std::enable_if_t<T::kHasUniformHash>` requires a constant expression; `const
static bool` initialised in-class with a constant does qualify, but the
inconsistency is worth normalising.

### `SLANG_BYTEWISE_HASHABLE` (`slang-hash.h:152`) — **justified**

`hashObjectBytes` is a direct `hashBytes` over the object representation, with a
`std::has_unique_object_representations_v` static assert
(`slang-hash.h:141-148`). Same `HashImpl::kIsAvalanching` caveat as the string
types. Used once, at `slang-ir-autodiff-primal-hoist.cpp:719`.

### What is _not_ marked, and is fine

`SLANG_COMPONENTWISE_HASHABLE_1` / `_2` (`slang-hash.h:171-183`) deliberately do
_not_ set `kHasUniformHash`, even though `_2` uses `combineHash`. That is the
correct choice and should be preserved.

Likewise `IRInstKey`, `ValKey`, `ValNodeDesc`, `AnnotationCacheKey`,
`IRSimpleSpecializationKey` and the other `combineHash`-based keys do not claim
it, so ankerl remixes them. This is what saves them from issue
[01](01-combine-hash-no-finalisation.md)'s low-bit degeneracy under ankerl.

---

## Why this matters for the benchmark specifically

Under ankerl, a type marked `kHasUniformHash` bypasses `wyhash::hash`, while an
unmarked type does not. So the _same_ key behaves differently on the `ankerl`
row of the matrix depending on a marker that has nothing to do with the
`SLANG_HASH` column. And boost ignores the marker entirely, so boost and ankerl
are not comparing like with like for the marked types.

This is a small effect relative to issue 01, but it is a confound in the same
family and costs almost nothing to remove.

---

## Proposed change

1. Change the string types' declarations to track the implementation:

   ```cpp
   static constexpr bool kHasUniformHash = HashImpl::kIsAvalanching;
   ```

   applied at `slang-string.h:207` and `:797`, and inside
   `SLANG_BYTEWISE_HASHABLE` (`slang-hash.h:153`).

   Note `slang-string.h` includes `slang-hash.h`, which includes
   `slang-hash-impl.h`, so `HashImpl::kIsAvalanching` is in scope — verify the
   include order, since `String` is defined before `getHashCode` is used.

2. For `SpvInstKey`: either remove the marker, or — preferably — do it _after_
   issue [01](01-combine-hash-no-finalisation.md) lands a properly avalanching
   fold, at which point the claim becomes true by construction and can be
   documented as such. Normalise `const static bool` to `static constexpr bool`
   either way.

3. Optionally, teach boost about the marker by specialising
   `boost::hash_is_avalanching<Slang::Hash<T>>`, so the marker means the same
   thing on every backend. This makes the comparison fair rather than making it
   faster; do it only if the matrix is going to be re-run.

4. Add a comment at `DetectAvalanchingHash` (`slang-hash.h:60-65`) spelling out
   the contract: _declaring `kHasUniformHash` means the map will not mix your
   hash; only declare it if the hash is genuinely avalanching, and if it is built
   from `combineHash`, it is not._

---

## Risks and things to watch

- Removing a marker makes the map do more work per lookup but produces better
  distribution. Whether that is a net win is empirical; measure rather than
  assume.
- Changing the marker changes bucket assignment and therefore iteration order
  for the affected maps. Expect test churn wherever output depends on hash-map
  iteration order — which is itself a latent determinism problem worth knowing
  about.
- Do **not** bundle this with issue 01. Both change effective hash distribution;
  landing them together makes the benchmark movement unattributable.

---

## Validation

1. Confirm each of the three types still compiles under all four `SLANG_HASH`
   values, since `kHasUniformHash` would now be a non-constant-looking expression
   in a template context.
2. Re-run the matrix and compare the `ankerl` row before and after — this is the
   row the marker affects.
3. Full `sti` for iteration-order churn.

---

## Related

- [01](01-combine-hash-no-finalisation.md) — the reason `SpvInstKey`'s claim is
  doubtful, and the change that would make it true.
- [06](06-string-hash-not-cached.md) — `String`'s hash, the other property of the
  same declaration.
- [09](09-spvinstkey-list-key-and-double-insert.md) — `SpvInstKey`'s other
  problems.
- [15](15-structured-keys-bypass-slang-hash-axis.md) — the wider framing.
