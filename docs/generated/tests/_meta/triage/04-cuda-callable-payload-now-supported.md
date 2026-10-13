# 04 — CUDA gained callable-shader support; both the test and a design doc are stale

**Verdict: doc-and-test-stale.** This one is worth reading even if the others
are skipped, because it is a defect in the design-doc regeneration that
shipped in `a680b2b50f` — my own work in the companion PR.

## What the test asserts

`design/pipeline/04c-layout-ir/raytracing-callable-payload-rejected-on-cuda.slang`
expects CUDA to _reject_ a `callable` entry point that takes an `inout`
payload:

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry main -stage callable
[shader("callable")]
void main(inout CallData p)
//CHECK:  ^ target does not support ray tracing entry point parameters
```

## What actually happens

The compile succeeds and emits CUDA:

```
$ build/Release/bin/slangc <test> -target cuda -entry main -stage callable
warning[E40100]: entry point 'main' has been renamed to 'main_0'
#include ".../prelude/slang-cuda-prelude.h"
struct CallData_0 { float4 color_0; };
```

`E39032` is never raised. slang-test reports
`No diagnostics found on line 28` plus an exhaustive-mode failure for the
unannotated rename warning.

## Root cause — an intentional compiler change

```
$ git log -L 2569,2575:source/slang/slang-type-layout.cpp
5ed83a468c Add callable shader support to CUDA/OptiX backend (#12182)
 LayoutRulesImpl* CUDALayoutRulesFamilyImpl::getCallablePayloadParameterRules()
```

That commit changed the accessor from returning `nullptr` to returning real
rules. Checking all four families at HEAD:

| family | RayPayload | CallablePayload | HitAttributes |
| ------ | ---------- | --------------- | ------------- |
| CUDA   | rules      | **rules**       | rules         |
| Metal  | null       | null            | null          |
| CPU    | null       | null            | null          |
| LLVM   | null       | null            | null          |

CUDA now supplies **all three**. It is not a regression — the diagnostic is
correctly absent.

## The doc is wrong, and I wrote it that way

`docs/generated/design/pipeline/04c-layout-ir.md:279-284` still says:

> The Metal, CPU, and LLVM layout-rules families return `nullptr` from
> all three accessors, **and the CUDA family from
> `getCallablePayloadParameterRules` alone**; the SPIR-V/GLSL, HLSL/DXIL,
> and WGSL families supply all three.

The claim about Metal/CPU/LLVM is right. The CUDA clause has been wrong since
`5ed83a468c` and survived the regeneration in `a680b2b50f`.

**This was already known, and I missed that too.** The entry's own comment in
`expected-failures.txt` diagnosed both halves before I started:

> Obsolete test. https://github.com/shader-slang/slang/pull/12182 added
> CUDA/OptiX callable support, so
> `CUDALayoutRulesFamilyImpl::getCallablePayloadParameterRules()` returns real
> rules and the `E39032` rejection this test asserts no longer happens. **The
> doc the test is anchored to went stale the same way** — 04c-layout-ir.md
> still lists CUDA as returning `nullptr` from that accessor. Remove once that
> doc and this bundle are regenerated.

So the correction was written down and waiting. My doc pass did not find it
because it never read `expected-failures.txt` as an input: sections were
classified by whether their _cited line numbers_ moved and whether a recorded
doc-gap touched them, and this section had neither. That is the real lesson —
`expected-failures.txt` is a triage source, not just a suppression list, and a
doc regeneration should read it. The same comment block also contains the
correct fix for `classdecl-reference-type` (see 03), which I likewise
rediscovered independently rather than read.

## Fix, in order

1. Correct the doc sentence: CUDA supplies all three ray-tracing rule
   accessors; only Metal, CPU and LLVM return `nullptr`.
2. Rewrite the test. The claim it was pinning — "a target without callable
   payload rules rejects the parameter with E39032" — is still true; it just
   needs a target that actually lacks them (`-target metal` or `-target cpp`),
   which is what the same doc paragraph already says.
3. Delete its expected-failure entry.
