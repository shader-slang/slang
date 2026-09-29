# 07 — `IRConstantKey` recomputes its hash on every probe, walking a decoration list each time

**Status:** not started
**Estimated size:** M
**Impact:** every IR constant lookup pays a pointer chase plus a decoration-list
walk; every growth of the constant map re-hashes every string literal in the
module

---

## Summary

`IRConstantKey` is a bare pointer with a `getHashCode()` that forwards to the
pointee and computes the hash fresh every time:

```cpp
// source/slang/slang-ir.h:1986-1992
struct IRConstantKey
{
    IRConstant* inst;

    bool operator==(const IRConstantKey& rhs) const { return inst->equal(rhs.inst); }
    HashCode getHashCode() const { return inst->getHashCode(); }
};
```

For a string or blob literal, `IRConstant::getHashCode()` calls
`getStringSlice()`, which begins with a **linear walk of the instruction's
decoration list**, and then hashes every byte of the string.

Because the key stores no cached hash, the map calls `getHashCode()` afresh on
every probe _and on every stored key during every table growth_. Growing the
constant map therefore re-walks every decoration list and re-hashes every string
literal in the module.

Contrast `IRInstKey`, sitting fifty lines above in the same header, which _does_
cache (`slang-ir.h:1947`) and is therefore free to rehash. The asymmetry looks
accidental.

---

## Background

### The hash

```cpp
// source/slang/slang-ir.cpp:2354-2389
HashCode IRConstant::getHashCode()
{
    auto code = Slang::getHashCode(getOp());
    code = combineHash(code, Slang::getHashCode(getFullType()));

    switch (getOp())
    {
    case kIROp_BoolLit:
    case kIROp_FloatLit:
    case kIROp_IntLit:
        {
            SLANG_COMPILE_TIME_ASSERT(sizeof(IRFloatingPointValue) == sizeof(IRIntegerValue));
            // ... we can just compare as bits
            return combineHash(code, Slang::getHashCode(value.intVal));
        }
    case kIROp_PtrLit:
        {
            return combineHash(code, Slang::getHashCode(value.ptrVal));
        }
    case kIROp_BlobLit:
    case kIROp_StringLit:
        {
            const UnownedStringSlice slice = getStringSlice();
            return combineHash(code, Slang::getHashCode(slice.begin(), slice.getLength()));
        }
    case kIROp_VoidLit:
        {
            return code;
        }
    default:
        {
            SLANG_ASSERT(!"Invalid type");
            return 0;
        }
    }
}
```

### The decoration walk hidden inside `getStringSlice`

```cpp
// source/slang/slang-ir.cpp:2251-2260 (abridged)
UnownedStringSlice IRConstant::getStringSlice()
{
    SLANG_ASSERT(getOp() == kIROp_StringLit || getOp() == kIROp_BlobLit);
    // If the transitory decoration is set, then this is uses the transitoryStringVal for the text
    // storage. This is typically used when we are using a transitory IRInst held on the stack (such
    // that it can be looked up in cached), that just points to a string elsewhere, and NOT the
    // typical normal style, where the string is held after the instruction in memory.
    //
    if (findDecorationImpl(kIROp_TransitoryDecoration))
    {
        ...
    }
    ...
}
```

```cpp
// source/slang/slang-ir.cpp:252-260
IRDecoration* IRInst::findDecorationImpl(IROp decorationOp)
{
    for (auto dd : getDecorations())
    {
        if (dd->getOp() == decorationOp)
            return dd;
    }
    return nullptr;
}
```

So the hash of a string literal is: dereference the `IRConstant` (cold, scattered
in the arena), walk its decoration list (pointer chasing, each decoration another
cold line), then scan the string bytes.

The `TransitoryDecoration` mechanism exists so that a _stack-allocated_ key
instruction can point at a string held elsewhere. That mechanism is only used on
the probe key, never on the stored constants — but `getStringSlice()` cannot
know that, so every stored constant pays the check too.

### Where it is used

```cpp
// source/slang/slang-ir.h:2032
typedef Dictionary<IRConstantKey, IRConstant*> ConstantMap;
```

```cpp
// source/slang/slang-ir.cpp:2402-2420 (abridged)
IRConstant* IRBuilder::_findOrEmitConstant(IRConstant& keyInst)
{
    // We will check for such an instruction in a slightly hacky
    // way: we will construct a temporary instruction and
    // then use it to look up in a cache of instructions.
    // The 'fake' instruction is passed in as keyInst.
    IRConstantKey key;
    key.inst = &keyInst;

    IRConstant* irValue = nullptr;
    if (m_dedupContext->getConstantMap().tryGetValue(key, irValue))
        return irValue;
    ...
}
```

`_findOrEmitConstant` is behind `IRBuilder::getIntValue`, `getBoolValue`,
`getFloatValue`, `getStringValue`, `getPtrValue` — i.e. every constant materialised
anywhere in the front end, in lowering, and in every IR pass. It is also reached
indirectly from `canonicalizeInstOperands` in the hoistable-inst path
(`source/slang/slang-ir.cpp:2711-2731`, see issue
[11](11-findoremithoistableinst-per-operand-probes.md)).

### The equality side

```cpp
// source/slang/slang-ir.cpp:2348-2352
bool IRConstant::equal(IRConstant* rhs)
{
    // TODO(JS): Only equal if pointer types are identical (to match how getHashCode works below)
    return isValueEqual(rhs) && getFullType() == rhs->getFullType();
}
```

`isValueEqual` for a string literal calls `getStringSlice()` on **both** sides
(`slang-ir.cpp:2331-2334`), so an equality check is another two decoration walks
plus a byte comparison. Every hash collision, and every genuine match, pays this.

---

## Why this is independent of the selected hash function and map

The decoration walk and the repeated invocation of `getHashCode()` happen
outside the hash function. `SLANG_HASH` changes only the `hashBytes` call at the
end. And no choice of map avoids re-invoking a key's `getHashCode()` on rehash;
that is what caching in the key is for.

---

## Proposed change

### Step 1 — cache the hash in the key

Mirror `IRInstKey`:

```cpp
struct IRConstantKey
{
private:
    IRConstant* inst = nullptr;
    HashCode hashCode = 0;

public:
    IRConstantKey() = default;
    IRConstantKey(IRConstant* i) : inst(i), hashCode(i->getHashCode()) {}

    IRConstant* getInst() const { return inst; }
    HashCode getHashCode() const { return hashCode; }

    bool operator==(const IRConstantKey& rhs) const
    {
        if (hashCode != rhs.hashCode)
            return false;
        return inst->equal(rhs.inst);
    }
};
```

This alone fixes the rehash cost entirely (stored keys carry their hash) and adds
a cheap early-out to `operator==`.

Update the one construction site (`slang-ir.cpp:2412-2413`) from
`IRConstantKey key; key.inst = &keyInst;` to `IRConstantKey key(&keyInst);`, and
check for any other place that assigns `.inst` directly.

### Step 2 — get the decoration walk out of the hash path

Options, in preference order:

**(a) Cache the slice on the transitory key rather than checking a decoration.**
The `TransitoryDecoration` mechanism exists only to let a stack-built key point
at external string storage. Give `IRConstant` an explicit way to be constructed
as a transitory key that stores the slice directly, so `getStringSlice()` becomes
a branch on a flag rather than a list walk. Look at how the transitory constant
is built — grep `kIROp_TransitoryDecoration` — and see whether the decoration is
load-bearing for anything other than `getStringSlice`.

**(b) Hoist the check.** Have `getHashCode` take the slice as a parameter, or
split into `getHashCodeForStoredConstant()` (no transitory check) and the
existing form. Less clean.

**(c) Leave it.** With step 1 done, the walk happens once per key construction
rather than once per probe and once per rehash. That may be enough; measure
before doing more.

**Recommendation:** do step 1, measure, then decide whether (a) is worth it.
Step 1 is small and safe; (a) touches a mechanism whose full purpose needs
understanding first.

### Step 3 (optional) — reconsider the key entirely

The deeper question: why is the constant map keyed by a _structural_ key at all,
given that string literals could be interned once at creation and then compared
by pointer? That is a larger design change; note it, do not attempt it as part of
this issue.

---

## Risks and things to watch

- **The cached hash must be computed after the key instruction is fully
  populated.** `_findOrEmitConstant` receives `keyInst` already filled in by its
  callers, so constructing the key from it is safe — but verify each caller
  (`getIntValue`, `getStringValue`, …) does not mutate `keyInst` after the key is
  built.
- The stored `IRConstant*` in the map must never have its value mutated after
  insertion, or its cached hash goes stale. Constants are immutable by
  construction in the IR, but confirm — grep for writes to `IRConstant::value`.
- `IRConstant::getHashCode()` is non-`const`. That is why `Hash<IRConstant>` is
  not used directly and `IRConstantKey` wraps it. Keep `IRConstantKey::getHashCode()`
  `const` so `Slang::Hash<IRConstantKey>` continues to work — `HasSlangHash<T>`
  requires the call to be valid on a `const T&` (`slang-hash.h:33-38`).

---

## Validation

1. Microbenchmark: build a module with many distinct string literals, time
   `_findOrEmitConstant` throughput before and after.
2. Count `findDecorationImpl` calls (temporary instrumentation) on a large
   compile, before and after.
3. Full `sti` — constant deduplication is load-bearing for IR structural
   identity, so any behaviour change shows up immediately.
4. Confirm the change is neutral across `SLANG_HASHMAP` values (the rehash saving
   should show up most on the growth-heavy backends).

---

## Related

- [01](01-combine-hash-no-finalisation.md) — the `combineHash` used here.
- [10](10-irinstkey-hash-recomputed-per-operation.md) — the sibling key that
  already caches, and its own separate problem.
- [11](11-findoremithoistableinst-per-operand-probes.md) — reaches the constant
  map from the hoistable-inst path via `canonicalizeInstOperands`.
