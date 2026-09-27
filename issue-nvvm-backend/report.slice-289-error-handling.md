# Slice289: qualify six error-handling compute contracts

## Motivation

Inventory269 found nine error-handling compute files and no direct main-corpus selection. Earlier271
qualified a defer/throw interaction, leaving typed catches, generic rethrows, aggregate errors and
synthesized throwing witnesses without this focused three-mode evidence.

## Proposed solution

Run six unchanged authored compute contracts at NVRTC O3 and NVVM O0/O3. Preserve the original
shader-object flags, inputs and CHECKs, and independently derive every output word. All18 GPU cells
and54 output words pass on unchanged accepted285 binaries. No compiler change is justified.

| Existing source        | Full result, in decimal | Observed source behavior                                |
| ---------------------- | ----------------------- | ------------------------------------------------------- |
| basic                  | 2,0,17,18,6,1           | Two typed catches, success and a contained throw        |
| catch-all              | 1,16,48,7               | Typed catch, catch-all and success                      |
| generics               | 5,1,0                   | Generic success, catch/rethrow and outer typed catch    |
| non-trivial-error-type | 2,19,0                  | Success and aggregate code/parameter payload            |
| synthesized-witness    | 4                       | Synthesized mutating wrapper forwarding generic success |
| throws-with-params     | 7                       | Parameterized throwing function's success path          |

## Change summary

Only the completed plan, this report, compact [evidence](research-evidence.slice-289.json) and navigation
are committed. Original tests, compiler/provider and main manifests are unchanged. Raw source snapshots,
mirrors, runner, full outputs and reviews stay under `build/nvvm-error-handling289`.

## Concepts and vocabulary

**Native ordinal** selects the authored harness directive, including earlier nonselected directives.
**Untyped output** prints uint32 words in hexadecimal: CHECK11/12 means decimal17/18.
A **synthesized witness** adapts the concrete method to its interface requirement; this fixture uses
generic specialization and does not establish runtime existential dispatch.

## Process report

Consider the aggregate-error test's complete relevant source operation:

```slang
struct MyError { int code = 0; int param = 0; };
int func(int val) throws MyError
{
    if (val >= 3) throw MyError(1, val);
    return val * 2;
}
```

The caller first stores `try func(1)` as2, then calls `try func(3)`. Its typed handler writes
`err.code * 0x10 + err.param`, giving19. The final third word remains its input initialization0.
The generic test likewise leaves its third word untouched; these zeros do not prove executed writes.

Source inspection shows `ErrorHandlingLoweringContext::processTryCall` converts `IRTryCall` into a
call returning `Result<T,E>`, a result-error test and branches to the existing success/failure blocks.
`processThrow` returns `makeResultError`; `processReturn` wraps the success value. These are canonical
inputs to existing lowering. No helper, fallback, alternate representation or producer repair is added.
This source trace is not an emitted-IR proof: all branch-selecting arguments here are literal and can
be optimized away. The runs establish compiled observable behavior, not retained runtime exception
machinery. The witness and parameterized-call fixtures take success only; their fallback10/-1 paths
are untested, and no general throwing-witness claim follows.

A fresh author spawn hit the thread limit, so a reused bounded author and separate reused reviewer
prepared and checked the contracts. Root independently derived full expectations. The maintained
directive enumerator selects CPU ordinals2/2/2/2/2/0; existing adaptation replaces CPU with CUDA while
preserving shader-object and untyped-output flags. Eighteen isolated mirrors retain exact bodies and
all input/CHECK comments; there are no imports or sidecars. Each cell executes serially with retries
disabled and a180-second owned process-group limit. All report return0, one executed/passed, zero
ignored and empty failure diagnostic/shape. No timeout, failure, retry or unrun obligation occurred.

`catch-all.slang` has `// CHECK-NEXT 7` without a colon. The experiment deliberately preserves this
original defect and supplements FileCheck with the full four-word oracle; all modes actually return7
at the final position. The next bounded slice repairs that weak native oracle and proves it rejects a
deliberately wrong final value. Runtime-loaded error paths remain a later qualification opportunity.

Live before/after100layout/37runtime/11qualifiedsource/2config/576maininput identities and22pins are
exact285. Full285 remains inherited:580cases/576sources/1740cells,1703correct/37unresolved/20histories.
These six focused sources stay outside the main corpus and its128-source discovery cap. No arbitrary
error types, pointers, existential catches, arithmetic accuracy or performance claim. The loop continues;
Slack notifications stay skipped.
