# 03 — `classdecl-reference-type.slang` is a test bug, not a compiler bug

**Verdict: test-bug.** Fix the test, then delete its expected-failure entry.

## What the test asserts

```slang
//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute
class C { int v; __init(int x) { v = x; } }

[shader("compute")][numthreads(1,1,1)]
void main()
{
    //CHECK: new C
    C a = new C(7);
}
```

The intent is sound — a `class` is the reference-type aggregate, so it should
allocate with `new` rather than being copied like a `struct`.

## Why it fails

`a` is never read, so the allocation is dead and is eliminated. The emitted
C++ entry point has an empty body:

```cpp
void _main_0(void* _S1, void* entryPointParams_0, void* _S2)
{
    return;
}
```

No `class C_0` is emitted at all, so `CHECK: new C` cannot match.

## The compiler is right

Give the object an observable use and the expected shape appears:

```slang
C a = new C(7);
gOut[0] = a.v;
```
```cpp
class C_0 : public RefObject
static RefPtr<C_0> C_x24init_0(int32_t x_0)
    RefPtr<C_0> _S1 = new C_0();
```

`new C_0()` matches the existing `CHECK: new C` pattern as a prefix, so the
only change needed is to consume the value.

## Already diagnosed

This verdict was not new. The entry's comment in `expected-failures.txt`
already said to "make the allocation live (for example storing `a.v` into a
buffer)". I reached the same conclusion independently before reading it, which
says the comment was right, not that the analysis was needed twice.

## Fix

Add a buffer store so the allocation is live:

```slang
RWStructuredBuffer<int> gOut;
...
    C a = new C(7);
    gOut[0] = a.v;
```

This is the general hazard for any characterization test whose subject is a
value rather than a diagnostic: DCE is entitled to delete anything unobserved,
so the test has to make the thing it is pinning reachable from an output.
