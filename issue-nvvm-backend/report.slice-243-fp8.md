# Slice243: qualify FP8 scalar contracts before admission

## Motivation

Accepted242 leaves frozen `hlsl-intrinsic/substandard-fp-folding.slang#cuda-1` stopped at helper
parameter `FloatE4M3` in directO0/O3. Its exact1.25 literals flow through `pack8` to a UInt8 bitcast.
`compute/dynamic-dispatch-substandard-float.slang#cuda-1` stops at helper result `A`; its fields and
any-value marshalling require separate aggregate support. These measured opportunities rank ahead
of texture GetDimensions, whose NVRTC cell already mismatches, and exact BF16 integer construction,
whose double-rounding counterexample was retained in research238. No existing oracle was normalized.

Material runtime was reconsidered. Bindings, textures/LUTs, inputs and output oracle remain absent;
this support/correctness research uses the explicit cadence override. No material execution or
performance claim is made.

## Proposed solution

Keep production unchanged and qualify format-distinct i8 scalar transport and exact CUDA12.9
Float32 conversion semantics using dynamic buffers. Freeze independently derived rational oracles
before accepting raw recipes. Separate bit transport from casts, and separate runtime casts from
shared constant folding. The result is evidence for future work, not direct-Slang admission.

## Change summary

Only this report, the completed plan, compact semantic evidence, STATUS and the new FP8 design
contract change. Ignored `build/nvvm-loop/slice-243-fp8` retains oracle/scripts, source controls,
LLVM/PTX/cubins, complete input/expected/output buffers, failures, source hashes and command records.
No compiler/library/frontend/provider/ABI/test-runner source, registered test, manifest or existing
runtime input changes. No compiler build, commit or push by the worker.

## Concepts and vocabulary

- **Physical i8** carries bits; a separate semantic format distinguishes E4M3 and E5M2.
- **SATFINITE** maps overflow and infinity to signed finite maximum; it is the CUDA constructor policy.
- **Runtime narrowing** consumes dynamic Float32; **literal folding** uses shared host helpers before
  CUDA/NVVM emission and has a distinct producer contract.
- **Raw LLVM control** bypasses production Slang NVVM lowering. Its O0/O3 labels do not claim new
  public direct-backend support.

## Process report

The worker verified all40 accepted source hashes,12 artifacts and560 inputs unchanged, then passed
smoke4 before research GPU work. Full242 evidence remains inherited:1692cells,1651correct,
41unresolved and16resolved histories. Latest targeted233, full242 and implementation cadence0 do
not change. No full replay was needed for this research-only slice.

`core.meta.slang` produces canonical FP8 types and `FloatCast`; CUDA emission maps them to installed
`__nv_fp8_*` constructors. Source bitcasts remain scalar same-size bitcasts. The public source
controls retain these exact shapes; current direct modes reject UInt8→FP8 bitcast type. The frozen
folding source's `pack8` parameter rejection is separately reproduced. A standalone dynamic-object
compile without render-test conformance registration is only a partial source trace, not a replay of
its registered frozen cell; accepted242 supplies that exact resultA diagnostic and identity.

The input freeze has1536 E4M3 and1500 E5M2 records, each eight words plus a count header. It covers
all256 encodings with both branch choices; every finite representable value and adjacent Float32
values; every adjacent finite midpoint and neighbors; overflow midpoint and neighbors; both signs,
zeros, Float32 subnormals, infinities and signaling/quiet NaNs. Independent integer/rational expected
buffers were frozen before recipe execution. The parent independently reconstructed all24,290
combined expected words. NaN conversion outputs are classification-only; bit transport is exact.

Four public CUDA NVRTC controls and eight raw LLVM controls compile and assemble with SM80 ptxas.
They execute assembled cubins sequentially on L4SM89. Separate transport kernels test i8 internal
helper parameter/result, branch/phi and UInt8 bit transport; separate casts test widening and narrowing.
All12 pass145,740 complete words (36,432 active/109,308 preserved). Two additional actual public
Slang NVRTC casts pass24,290 words (6072active/18,218preserved), qualifying the source lowering too.
Total qualified runtime evidence:14 launches,170,030words,42,504active/127,526preserved,zero mismatches.
All input and sentinel fields are checked. The raw recipe uses ordinary integer/Float32 operations;
all PTX targets SM80 and no SM89 FP8 instruction is used. A first raw compilation omitted target
data layout; all eight verifier failures and scripts were retained before correcting only the
standalone harness. No GPU loss occurred.

For narrowing, bit guards establish valid finite input and bounded shifts before RNE logic. The
retained significand plus half-way tie parity gives exact target rounding; saturation uses the
format's finite maximum. Widening derives bits from the valid canonical encoding. The independent
oracle enumerates rational representable values and chooses nearest/even, rather than duplicating
this bit recipe. The parent separately reviewed shift bounds and the finite recipe. Promotion still
requires a proper production descriptor and full acceptance, not copying a diagnostic workaround.

Consider this pre-existing source difference:

```slang
uint a = bit_cast<uint8_t>(FloatE4M3(256.0f));
uint b = bit_cast<uint8_t>(FloatE4M3(asfloat(inputBits))); // inputBits encodes 256.0f.
```

The unchanged compiler outputs126 for a and120 for b. Five small literal/dynamic examples were
compiled, assembled and executed separately (41 observational words; not counted as correctness
passes). E4M3 minimum subnormal also folds to448; E5M2 minimum subnormal folds to infinity, which
CUDA construction saturates to byte123 while separately folded Float32 widening remains infinity.
`SCCPContext::evalCast`→`IRBuilder::getFloatValue`→`FloatToFloatE4M3/FloatToFloatE5M2` and the reverse
helpers is the producer path. E4 exponent15 is over-clamped, subnormal narrowing is invalid, and
both shared widening helpers use the wrong scale. A small host executable calling the actual shared
helpers independently records byte1 widening as0x37000000/0x34800000 instead of0x3b000000/0x37800000.
These finite/subnormal defects warrant producer-side repair before broader backend admission.

Overflow is a separate policy issue: `unit-test-math.cpp` explicitly expects shared E4 overflow to
NaN and E5 overflow to infinity. CUDA runtime SATFINITE differs. This research changes neither
policy. NaN payload/sign differences are also not reclassified as failures. The design contract
records these distinctions, avoiding a backend repair of malformed/incorrectly folded semantics.

Self-review inventory: no new production helpers, fallback or special case. Research helpers are
format-specific conversion, internal transport selection, independent oracle and evidence plumbing;
all survive only as ignored proof artifacts. Valid runtime FP8 inputs are canonical; incorrect
literal values are produced upstream and must be fixed there. No syntax or semantic values are
reconstructed to force compiler support. Dynamic-object/aggregate/BF-vector storage, exact integer,
Half/double constructors, vector operations and external ABI remain excluded. The next measured
action is bounded finite/subnormal shared-producer correctness work with existing overflow policy
preserved, followed by reconsideration of scalar admission. Independent parent acceptance is complete; parent owns the authorized local commit.

Independent parent acceptance checks180 indexed artifacts,19 primary sources, all baseline identities
and prior indices,66 canonicalized compact references before its own nine references, the complete
frozen grid and170,030 qualified runtime words. All41 observational words are verified separately;
pre-existing literal errors remain recorded as failures. See the nine parent artifacts in the compact
semantic evidence.
