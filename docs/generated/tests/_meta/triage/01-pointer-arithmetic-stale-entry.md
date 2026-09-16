# 01 — `pointer-arithmetic-spirv-emission.slang` is a stale expected-failure

**Verdict: stale-entry.** Delete the line from `expected-failures.txt`.

## Evidence

```
$ build/Release/bin/slang-test -bindir build/Release/bin/ \
    docs/generated/tests/conformance/types-pointer/pointer-arithmetic-spirv-emission.slang
100% of tests passed (3/3)
```

All three directives pass. It is the only one of the 21 listed entries that
does.

## Why it matters

`verify` reported `expected-fail: 20` against 21 listed entries. That
arithmetic is the tell: an entry that never appears in the failure list is
invisible, so a test that has been fixed keeps its entry indefinitely and the
list slowly decays into noise.

Worth considering as a follow-up: have `verify` warn when a listed entry did
not fail, which turns this class of staleness into a signal instead of
something found by hand.
