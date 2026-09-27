# Slice291: qualify runtime-loaded error paths

## Motivation

Slice289's original tests use literal inputs, which can fold error control flow. Its synthesized
throwing witness also succeeds unconditionally. This slice uses runtime buffer inputs to observe
success and failure through generic rethrows, aggregate errors and the synthesized witness.

## Proposed solution

Two raw fixtures pass at NVRTC O3 and NVVM O0/O3. Every output starts at a distinct sentinel and has
an anchored full-word CHECK. The final qualified cases are:

| Path                                        | Runtime inputs | Complete decimal results |
| ------------------------------------------- | -------------- | ------------------------ |
| Generic float rethrow                       | 2.5,3.5,1,4    | 5,1,2,1                  |
| Integer aggregate error                     | 1,3,4,2        | 2,19,20,4                |
| Synthesized throwing witness, refined catch | -1,0,7,-3      | 272,4,11,272             |

The first fixture writes its generic and aggregate results in that order. LLVM/PTX inspection proves
runtime input-to-call-to-result flow, including the tag-dependent witness catch result. No compiler
change is needed. Across the initial and refined experiments, all nine GPU cells and48 output words
pass; all three IR captures compile. Final coverage uses the unchanged generic/aggregate v2 fixture
and refined witness v3 fixture.

## Change summary

Only this report, the completed plan, compact [evidence](research-evidence.slice-291.json) and navigation
are committed. Raw fixtures, full oracles, versioned freezes, outputs, IR and reviews remain under
`build/nvvm-runtime-errors291`. No production source, maintained test or main-corpus change occurs.
The next bounded slice promotes the final two fixtures as native regressions.

## Concepts and vocabulary

**Result tag** marks success or failure. `Result<T,E>` lowers to a boolean tag and an AnyValue payload
large enough for either alternative. **Synthesized mutating witness** adapts a nonmutating concrete
method to a mutating interface requirement. This test uses generic specialization; it does not prove
runtime existential/type-ID dispatch. **Observable tag** means a wrong success/error choice changes
the checked output, rather than both arms interpreting the payload identically.

## Process report

The concrete witness method reads its receiver's runtime-loaded integer. Negative values throw enum
error16; nonnegative values return `value + sizeof(int)`. The interface requires a mutating generic
method while the implementation is nonmutating. The final caller is:

```slang
int helper<T: IFoo>(T receiver)
{
    do
    {
        return try receiver.dosomething<int>();
    }
    catch (error: WitnessError)
    {
        return 0x100 + reinterpret<int>(error);
    }
}
```

Initially, the catch returned the error payload unchanged. Those v2 GPU outputs `[16,4,11,16]` were
correct, and LLVM retained the synthesized adapter's tag forwarding. Independent IR review found that
optimized PTX discarded consumer tag use because both success and catch returned the same integer
payload. This was valid optimization and an oracle limitation, not a compiler failure.

Before further execution, root amended the bounded plan with exactly four witness-only obligations.
The v3 catch adds256, predicting `[272,4,11,272]` independently. Misreading errors as success would
produce16; misreading success as error would produce260/267. The three new GPU modes pass without
rerunning the unchanged generic/aggregate fixture. Final witness PTX loads the runtime input, calls
the noinline method, loads its returned tag and payload, masks/tests the tag, computes payload+256,
selects adjusted/original payload and stores it. The callee retains the negative-input decision and
returns error16 or value+4. Synthesized adapter/helper inlining does not remove this distinction.

The generic and aggregate v2 modules also retain live input loads and noinline calls. Generic output
uses the returned tag to choose error integer1 or conversion of its float success payload. Aggregate
output chooses the success integer or `code * 16 + param`; code is1 and the two exercised error
parameters are3 and4. This does not qualify arbitrary error codes or input ranges.

Source inspection connects `lowerErrorHandling`/`processTryCall` to canonical `Result<T,E>` operations.
`ResultTypeLoweringContext` creates the tag plus Natural-sized AnyValue and emits value/error packing
and extraction. Actual LLVM returns are `{i1,{i32}}` and `{i1,{i32,i32}}`; live callers consume these
results. Canonical receiver, generic witness and result types are valid input. No representation
repair, custom marshaller, fallback or new compiler helper is introduced.

A fresh bounded author and separate reused reviewer derived inputs/oracles and checked every freeze
before root execution. An unexecuted v1 proposal was refined for formatting/directive protection and
an explicit interface semicolon; it is not failed compiler evidence. V2's eight and v3's four executed
obligations retain separate version-qualified identities. All run serially with180-second bounds and
retries disabled; there are no failures, timeouts, ignored or unrun cells. Two prose line references in
the first authored proof have a retained errata record; its hashed semantic excerpts were already correct.
Raw IR records retain their initial pending-review marker; subsequent proofs own live-path acceptance.

Installed100 layout entries,37 runtime,11 qualified source,2 configuration,576 main input hashes and
22 pins remain exact285 after both stages. Full285 and broader289/290 evidence stay inherited under
original identities. No pointers, FP8 errors, arbitrary exception forms, new existential dispatch,
arithmetic accuracy or performance claim. The authorized loop continues; Slack stays skipped.
