# 15 — ~58 hand-written `getHashCode()`s bypass the `SLANG_HASH` axis entirely

**Status:** not started (analysis / decision, not a code change per se)
**Estimated size:** S for the analysis; the follow-on work is issue 01
**Blocks:** drawing conclusions from the `SLANG_HASH` × `SLANG_HASHMAP` matrix

---

## Summary

The `SLANG_HASH` CMake option selects between wyhash, `boost::hash`,
`absl::Hash` and `std::hash`. But `Slang::Hash<T>` only consults that selection
for types that _do not_ define their own `getHashCode()`:

```cpp
// source/core/slang-hash.h:92-113
template<typename T>
struct Hash : DetectAvalanchingHash<T>
{
    auto operator()(const T& t) const
    {
        // Our preference is for any hash we've defined ourselves
        if constexpr (HasSlangHash<T>)
            return t.getHashCode();
        // Otherwise fall back to the hash provided by the selected hash
        // library
        else if constexpr (HasLibraryHash<T>)
            return HashImpl::LibraryHash<T>{}(t);
        ...
    }
};
```

There are roughly 58 `getHashCode()` implementations across `source/`. They
cover essentially every composite key the compiler uses. For all of them, the
`SLANG_HASH` selection is invisible — the hash is whatever `combineHash` and
`Hasher` produce, which is identical in all 32 matrix configurations.

This is not a bug. It is a fact about what the benchmark is measuring, and it
should be written down before the matrix results are used to choose a default.

---

## What `SLANG_HASH` actually varies

Only three things reach `HashImpl::LibraryHash`:

1. **Integer and enumeration keys.** e.g. `Dictionary<int, …>`,
   `Dictionary<IROp, …>`, `Dictionary<SpvWord, …>`.
2. **Pointer keys.** e.g. `Dictionary<IRInst*, …>`, `HashSet<IRInst*>`,
   `Dictionary<Decl*, …>`, `Dictionary<GenericDecl*, …>` — a _large_ fraction of
   the compiler's maps by volume.
3. **String bytes**, via `HashImpl::hashBytes` reached from
   `UnownedStringSlice::getHashCode()` / `String::getHashCode()`.

The membership test is deliberately narrow and well-documented:

```cpp
// source/core/slang-hash-impl.h:54-74
/// True for the types we hand to the selected library's hash function rather
/// than to a Slang-defined `getHashCode()`: the built-in integer, enumeration
/// and pointer types, the smart pointer types, and the standard string types.
///
/// The membership test is "does `ankerl::unordered_dense::hash<T>` advertise
/// itself as avalanching", which is exactly the set of types that library
/// specialises rather than forwarding to `std::hash`. [...]
template<typename T, typename = void>
constexpr static bool isLibraryHashable = false;
template<typename T>
constexpr static bool
    isLibraryHashable<T, typename ankerl::unordered_dense::hash<T>::is_avalanching> = true;
```

That covers (1), (2) and the `std::string`/`std::string_view` case; Slang's own
string types route through `hashBytes` explicitly. So the axis is real and
meaningful for pointer- and integer-keyed maps, which are numerous. It just does
not reach the composite keys.

---

## What it does not vary

Everything built with `combineHash` / `Hasher`. A non-exhaustive list of the
keys involved, with their hash sites:

| Key                                         | Hash                                                                                                                                                          | Used by                                  |
| ------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------- |
| `IRInstKey`                                 | `slang-ir.cpp:2236-2249`                                                                                                                                      | global value numbering (IR hash-consing) |
| `IRConstantKey` → `IRConstant::getHashCode` | `slang-ir.cpp:2354-2389`                                                                                                                                      | constant dedup                           |
| `AnnotationCacheKey`                        | `slang-ir.h:1994-2010`                                                                                                                                        | `m_annotationLookupCache`                |
| `IRSimpleSpecializationKey`                 | `slang-ir-clone.cpp:442-451`                                                                                                                                  | generic specialisation caches            |
| `ValNodeDesc`                               | `slang-ast-val.cpp:20-34`                                                                                                                                     | `ASTBuilder::m_cachedNodes` (AST dedup)  |
| `ValKey`                                    | `slang-ast-builder.h:139-161`                                                                                                                                 | same map, stored-key side                |
| `SpvInstKey`                                | `slang-emit-spirv.cpp:1940-1950`                                                                                                                              | SPIR-V instruction memoisation           |
| `BasicTypeKeyPair`                          | `slang-check-impl.h:206`                                                                                                                                      | conversion/coercion caches               |
| `KeyValuePair`                              | `slang-dictionary.h:57-60`                                                                                                                                    | anywhere a pair is a key                 |
| `SLANG_COMPONENTWISE_HASHABLE_2` users      | `slang-hash.h:178-183`                                                                                                                                        | `slang-spirv-core-grammar.h:51,59`       |
| various                                     | `slang-ir-any-value-marshalling.cpp:86`, `slang-ir-lower-buffer-element-type.cpp:285,481,510`, `slang-ir-typeflow-specialize.cpp:144`, `slang-emit-vm.cpp:28` | assorted pass-local caches               |

To enumerate the full set:

```bash
grep -rn "HashCode\(64\)\? getHashCode() const\|HashCode getHashCode()" \
    source/ --include=*.h --include=*.cpp
```

(58 matches at time of writing.)

These are, by and large, the _structurally interesting_ keys — the ones whose
distribution is hardest to reason about and where a bad hash costs the most. And
they are held constant across the entire matrix.

---

## Why this matters

Three consequences, in decreasing order of importance:

1. **The matrix cannot tell you whether `combineHash` is good enough.** It can
   only tell you which map best tolerates it. Issue
   [01](01-combine-hash-no-finalisation.md) shows the answer is "not good
   enough": the low 3 bits of a `ValNodeDesc` hash are a constant determined by
   `(tag, arity)` alone.

2. **The map axis is partly measuring compensation for `combineHash`.** ankerl
   and boost remix non-avalanching hashes; absl and tsl do not. So the ranking of
   maps will partly reflect the quality of a hash that the hash axis never
   touches. See the table in issue 01.

3. **A future default chosen from this matrix could be wrong after `combineHash`
   is fixed.** If `combineHash` is improved, the maps that were remixing to
   compensate lose their advantage, and the ranking may change.

---

## Proposed action

This issue is an analysis and a decision, not primarily a code change. Three
things to do:

### 1. Record the scope of the axis in `slang-hash-impl.h`

The file already has an excellent comment explaining _why_ the hash and the map
are chosen independently (`slang-hash-impl.h:4-19`). Extend it to say what the
hash axis does _not_ cover:

> Note that this selection only affects types that do not define their own
> `getHashCode()`. The compiler's composite keys — `IRInstKey`, `ValNodeDesc`,
> `SpvInstKey`, and ~55 others — build their hash from `combineHash`
> (`slang-hash.h`), which is the same in every configuration. So this option
> varies integer, enumeration and pointer hashing, plus the byte hash used for
> strings; it does not vary the hashing of structured keys.

This is the highest-value part of this issue: it stops the next person drawing
the wrong conclusion.

### 2. Decide whether to bring composite keys into the axis

Two possible directions:

**(a) Leave them out, and just make `combineHash` good** (issue 01). Simplest.
The composite-key hash becomes a fixed, known-good quantity, and the `SLANG_HASH`
axis keeps its current, well-defined meaning for the primitive types. This is
almost certainly the right call.

**(b) Route composite keys through the selected library's combiner.** `boost`
has `boost::hash_combine`, `absl` has `absl::HashState`/`AbslHashValue`, ankerl
has no combiner. This would make the axis cover everything, but it means
rewriting ~58 `getHashCode()`s against a per-library API, and ankerl has no
answer, so it would need a fallback anyway. Not worth it.

**Recommendation: (a).**

### 3. Re-run the matrix after issue 01

The numbers gathered before `combineHash` is fixed answer a different question
from the numbers gathered after. Keep both, and be explicit about which is
which when choosing a default.

---

## A related observation worth recording

`Hash<T>`'s dispatch has a subtlety documented at `slang-hash.h:159-170`:

> These spell the return type out as `HashCode64` rather than deducing it with
> `auto`. A deduced return type is only known once the function body has been
> parsed, and the body of a member function of a nested class is not parsed
> until the _enclosing_ class is complete. So with `auto`, `HasSlangHash<T>` is
> false for any such nested type while the enclosing class is still being
> defined, and `Hash<T>` silently falls through to "No hash implementation found
> for this type".

The failure mode there is a hard error, so it is self-announcing. But it is a
reminder that `HasSlangHash<T>` is a SFINAE probe whose answer can change
depending on where it is asked. If any type's `getHashCode()` is non-`const`
(as `Val::getHashCode()` and `IRConstant::getHashCode()` both are), then
`HasSlangHash<T>` is **false** for it — because the probe is
`std::declval<const T&>().getHashCode()` (`slang-hash.h:33-38`) — and
`Hash<T>` would fall through to the library hash or fail. That is why
`IRConstantKey` exists as a `const`-callable wrapper. Worth knowing when
auditing which keys actually go through which path.

---

## Related

- [01](01-combine-hash-no-finalisation.md) — the actual defect this framing
  exposes.
- [14](14-khasuniformhash-audit.md) — the other benchmark confound.
