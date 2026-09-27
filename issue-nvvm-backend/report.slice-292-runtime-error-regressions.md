# Slice 292: preserve runtime error-path regressions

## Motivation

The runtime-loaded probes from slice 291 proved success and error paths that older literal-input
tests could optimize away. Keeping those probes only under ignored build directories would leave
those paths without persistent regressions.

## Proposed solution

Add two native CUDA fixtures, each with NVRTC O3 and NVVM O0/O3 directives. Preserve the qualified
source tokens, runtime inputs, noinline calls and complete anchored hexadecimal output checks.
The generic/aggregate fixture expects decimal `[5,1,2,1,2,19,20,4]`; the synthesized-witness fixture
expects `[272,4,11,272]`. Every output word begins with a distinct nonexpected sentinel.

## Change summary

- `tests/cuda/nvvm-runtime-errors.slang` preserves generic float rethrow and two-field integer errors.
- `tests/cuda/nvvm-runtime-throwing-witness.slang` preserves success and error results through a
  synthesized mutating witness, including the catch-only offset that exposes tag handling.
- This report, completed plan, compact evidence and navigation retain acceptance and inherited limits.

No compiler, maintained runner, main-corpus selection or installed binary changes are needed.

## Concepts and vocabulary

A **result tag** distinguishes success from an error in the lowered `Result<T,E>` representation.
A **synthesized mutating witness** adapts a nonmutating implementation to a mutating interface
requirement. Here the interface call uses generic specialization, not runtime existential dispatch.

## Process report

For runtime receiver values `[-1,0,7,-3]`, the witness implementation throws error 16 on negative
inputs and returns `value + sizeof(int)` otherwise. Its caller makes the two outcomes observably
different:

```slang
int helper<T : IFoo>(T receiver)
{
    do
    {
        return try receiver.dosomething<int>();
    }
    catch (error : WitnessError)
    {
        return 0x100 + reinterpret<int>(error);
    }
}
```

Slice 291's initial catch returned the integer payload unchanged, allowing valid elimination of the
consumer tag test. Its refined catch adds 256; misreading a tag changes the checked result. This
promotion retains that refinement and its original qualification identity. The generic/aggregate
fixture likewise consumes runtime inputs and checks both success and error payloads.

`ErrorHandlingLoweringContext` creates canonical result operations; `ResultTypeLoweringContext`
packs their boolean tag and Natural-sized payload. Slice 291 already traced live loads, calls and
tag-dependent consumers through LLVM and PTX. These are valid compiler inputs; no representation
repair, compiler helper or special case is introduced. Token equivalence connects the new files to
that evidence, while fresh native directives validate persistent harness integration.

All six native tests pass with 36 exact output words, one executed/passed test each and no skips,
timeouts or retries. Before/after checks preserve 100 installed layout entries, 37 runtime artifacts,
11 qualified sources, 2 configuration files, 576 main inputs and 22 dependency pins. A fresh author
and separate reused reviewer checked the freeze before root execution; final evidence review accepted all outcomes without findings.
Original 291 experiments remain inherited under their original hashes. Coverage remains four lanes per fixture, including only error code 1 with aggregate
parameters 3/4 and witness enum 16. It does not establish arbitrary exception behavior, existential
dispatch, numerical accuracy or performance. The main 580-case corpus and checkpoint cadence remain
unchanged. The authorized loop continues, with Slack notifications skipped.
