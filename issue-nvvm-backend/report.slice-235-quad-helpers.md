# Emit typed CUDA quad vote helpers through direct NVVM

## Motivation

The frozen quad-control kernel splits sixteen threads into four complete quads:

```slang
RWStructuredBuffer<uint> outputBuffer;

[numthreads(16, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint index = WaveGetLaneIndex();
    if (index < 4)
        outputBuffer[index] = uint(QuadAny((index % 4) == 0));
    else if (index < 8)
        outputBuffer[index] = uint(QuadAny(false));
    else if (index < 12)
        outputBuffer[index] = uint(QuadAll((index % 4) == 0));
    else
        outputBuffer[index] = uint(QuadAll(true));
}
```

Its expected output is four ones, eight zeros, then four ones. NVRTC already produces that output;
direct NVVM O0/O3 instead rejected `RequireMaximallyReconverges`. Accepted research 234 established
that the marker is part of a complete CUDA intrinsic helper, and that deleting the marker alone
would still leave the typed `_slang_quadAny`/`_slang_quadAll` helper unsupported. This slice admits
that complete helper contract and preserves unrelated marker rejection.

## Proposed solution

Resolve the existing target intrinsic with `findTargetIntrinsicDefinition`, then validate its exact
scalar `bool(bool)` signature and single-block body. The only admitted bodies contain the GenericAsm
terminator alone or that terminator with exactly one of each canonical zero-operand requirement
marker. The recognized names are `_slang_quadAny` and `_slang_quadAll`. An incomplete pair, different
name/signature, extra instruction or multiple blocks does not receive whole-helper ownership.

Preflight and emission share this resolver and its seven typed operation descriptors. Preflight
records the complete operation closure before provider creation. Emission replaces the validated
whole helper with lane-index, base/source-lane arithmetic, Boolean encoding, four synchronized
indexed uint32 shuffles, OR/AND and Boolean decoding. The existing provider appends clamp 31; the
mask is always `0xffffffff`. Neither path independently treats markers as no-ops. The frontend,
standard library, provider, ABI, intrinsic representation and corpus runners are unchanged.

## Change summary

- `source/slang/slang-emit-nvvm.cpp` adds the bounded resolver, typed recipe and whole-helper
  preflight/emission ownership. Ordinary instruction validation and emission remain unchanged.
- `tests/cuda/nvvm-quad-votes.slang` supplies dynamic truth tables and independent Boolean
  expectations for common calls, divergent inline/noinline calls and complete-quad exits.
- `tools/slang-unit-test/unit-test-nvvm-emitter.cpp` extends the existing unsupported-case matrix
  with eight valid-frontend negative cases, including an extra barrier in an otherwise matching body.
- The discovery manifest adds one identity, recorded separately in `discovery-addition.slice-235.tsv`.
  The immutable frozen inventory is unchanged. Plan, result manifest, censuses, design and STATUS
  retain validation evidence, failure histories and the bounded follow-up.

## Concepts and vocabulary

A _target requirement marker_ describes a target execution requirement; it is not itself a CUDA
warp barrier. A _GenericAsm helper_ has a target implementation selected by canonical intrinsic
lookup, so source emission can replace its complete body. A _complete source quad_ contains all
four lanes addressed by the helper. The _shuffle member mask_ identifies rendezvous participants;
it does not change the operation into a reduction of only the currently active branch. A _typed
recipe_ records provider operation descriptors once for both preflight requirements and emission.

## Process report

For the example above, `hlsl.meta.slang` produces `QuadAny`/`QuadAll` helpers with a Boolean parameter,
`RequireMaximallyReconverges`, `RequireQuadDerivatives`, and a CUDA GenericAsm terminator.
`core.meta.slang` owns the two IR marker definitions. `CLikeSourceEmitter` consumes the complete
intrinsic through `findTargetIntrinsicDefinition`; SPIR-V and GLSL instead propagate the markers to
execution modes/layouts. The canonical checked helper body intentionally serves those different
target consumers. It is not a malformed frontend value, and reconstructing syntax or changing the
standard library would move this fix to the wrong layer.

`_validateNVVMFunction` used to scan the first marker before reaching GenericAsm. Its existing
ordinary value-helper resolver also deliberately rejects any preceding executable instruction.
The new `_resolveNVVMQuadVoteOperation` uses the shared target lookup, then validates the entire
body and exact signature. Function preflight invokes it after checking parameters; only a complete
recognized helper records its recipe and finishes validation at this boundary. Function emission
uses the same classification after creating the helper's single block, emits the recipe and leaves
the ordinary instruction loop. No case was added to an ordinary marker switch, and no unrelated
instruction can be silently skipped by this path.

`_emitNVVMQuadVoteOperation` computes `base = lane & ~3`, encodes the Boolean parameter as uint32
zero/one and reads `base | 0`, `base | 1`, `base | 2`, `base | 3`. It issues all four full-mask reads
before returning the combined predicate; there is no short-circuit shuffle sequence or mask query.
The provider's existing `WAVE_READ_LANE_AT` operation emits the synchronized indexed shuffle with
clamp 31 and its existing convergence attributes. OR implements Any; AND implements All. All source
values are zero/one, so this has the prelude's Boolean conversion and combination semantics.

The initial recipe descriptor accidentally specified an unsigned lane-index operand. The semantic
catalog correctly rejected it because the existing shuffle contract requires signed i32. Smoke
passed, while focused direct compilation retained its original marker diagnostic because the whole
recipe was not supported. Correcting that descriptor follows the existing masked-wave recipe: lane
indices are bounded to 0..31, and the provider uses signless i32. No new cast operation, fallback,
provider entry or accepted signature was needed. The failed attempt's source identity, logs and
smoke are retained separately; every acceptance gate reran on the corrected compiler.

The helper/fallback inventory contains two new functions, both retained. The resolver owns canonical
helper identity, signature and complete-body validation; the emitter owns the source algebra through
existing operations. The whole-helper early-return/continue pair is retained only under the shared
validated classification. The first-read initialization in the emitter follows the four-source
reduction directly. There is no new fallback, graph search, equivalence relation, syntax reconstruction
or independent representation. The existing canonical value-helper validator was not broadened.

For the required input-shape audit: the exact producer is `hlsl.meta.slang`'s target-switch helper,
with the two core requirement instructions and typed GenericAsm. That shape is canonical and intended;
its semantic source of truth is the selected target intrinsic plus the checked Boolean signature.
The resolver reuses that source rather than redoing substitution, lookup or lowering. Before the
production edit, the final dynamic fixture passed NVRTC and failed both direct modes with E52017;
its source and TEST_INPUT directives were never changed afterward. Research 234's marker-free
helpers independently failed exact GenericAsm signatures, proving that requirement suppression alone
was insufficient. The direct whole-helper boundary owns both parts, so an assertion plus a frontend
producer change would be incorrect. Eight negative sources retain exact preflight rejection,
including standalone markers, either incomplete pair, an unrelated name, wrong return/parameter
signatures and a matching body with an extra barrier. Unit cases additionally prove rejection happens
before provider load or module creation. Six direct canonical alias compilations succeed; four
standalone CUDA-source marker compilations retain their exact E99999 diagnostics.

CUDA's source participation contract remains explicit. On SM80, synchronized shuffles may rendezvous
across divergent branches when the named non-exited lanes execute matching shuffle sequences. Every
surviving quad must supply its four source lanes. Complete-quad exits before the operation satisfy
that restriction; partial-quad exits do not. A lane calling Any can read a predicate from a lane
calling All in the other branch. The implementation must not filter that value away as an active-only
vote. It promises neither SPIR-V maximal reconvergence for the whole kernel nor defined results from
missing source lanes. See the accepted research report's specification references and control probes.

Fresh final-source smoke passes 4/4 before expensive suites. Focused fixture passes 3/3; units pass
479/479 with one existing Windows-only skip. Exact research replay reads the accepted 512 binary
input/expectation pairs unchanged and passes 3,072 launches, checking 196,608 output words and all
96-word input regions. Each public/control and NVRTC/O0/O3 group has 512 cases. The 2,048 previous
actual-buffer hashes match exactly; 1,024 direct public-helper launches are new. Twelve replay PTX
artifacts assemble. Toolkit 18, runner contracts 6 and all six material compile/assembly cells pass.

The full checkpoint freshly executes all 1,356 frozen and 324 discovery cells. It has 1,635 correct,
45 retained failures and twelve resolved histories. Every old five-field outcome is exact except
for the two original frozen quad direct cells, which now have executed 1/passed 1 and no diagnostic.
The new fixture adds three correct cells separately. The two resolutions retain their exact whole
prior failure records, transitions and fresh proof; the ten earlier histories and all remaining
first-known/reproduction records survive. There are no missing, duplicate, extra or inherited cells,
lost support, oracle changes or baseline reset. Historical healthy denominators 427/72 remain fixed.

All 555 old input hashes remain unchanged, with one added fixture. All 30 old tested source paths and
12 artifacts are captured per final gate; only the emitter, unit source and discovery registration
change among old sources, with the new fixture becoming source 31. The worker audit independently
rederives all 512 Boolean expectations and checks every full 3,072 GPU output buffer, including inputs
and sentinels. Accepted 234 raw evidence is hash-verified unchanged. The parent independently compared
the full corpus rows and confirmed exact inventory and only the two expected old-cell transitions.
Independent parent acceptance also verified 689 unique compact evidence references, every final
source/artifact/input hash, complete retained/resolved failure histories and all raw replay buffers.
Full checkpoint 235 is accepted and implementation cadence is zero.

The evidence gate script was accidentally edited while bash was reading it, shifting its read offset
and stopping after the passing units. That shell interruption is retained separately; it is not a
failed or passing test. A separate resume script ran only aliases and the outstanding gates with
identical final source/artifact identities. No successful suite was needlessly repeated, and no
unexecuted or incomplete gate was counted as success.

Source revision and commit are the actual base `2cf7d42e0c259c05bc0fd7ab38e9490d05f0155e` plus the
recorded patch. Final compiler-library SHA-256 is
`ca34db1a349ae8716785032a0a3b01b3e6cf8455f3137e9358e9d1ad4eca63cf`; provider stays
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36. Native Ubuntu/L4 SM89,
driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86 and LLVM 14 are unchanged. No device loss,
system change, worker commit or push. Material runtime still lacks application bindings,
textures/LUT/input and an output oracle; six material compile/assembly cells cannot establish runtime
correctness or performance. Other arithmetic, matrix-prefix, ordinary aggregate and resource
boundaries remain independent; this slice stops after the two quad helpers.
