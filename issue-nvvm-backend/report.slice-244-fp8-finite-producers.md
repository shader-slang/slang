# Correct shared finite FP8 literal conversion

## Motivation

Consider the ordinary constant expressions:

```slang
uint large = asuint(float(FloatE4M3(256.0f)));
uint smallE4 = asuint(float(FloatE4M3(0.001953125f)));
uint smallE5 = asuint(float(FloatE5M2(0.0000152587890625f)));
```

The accepted compiler folds these to 448, 448 and infinity instead of 256, 2^-9 and 2^-16.
The 52-word focused fixture produces 32 incorrect words in each of NVRTC O3 and direct NVVM O0/O3.
An independently enumerated 3548-case host-helper grid finds 220 mismatches: 20 widening and 200
narrowing cases. The defects precede this slice; research 243 also records the actual shared-helper
and compiler observations on the same accepted artifacts.

Correctness ranks ahead of FP8 backend admission and independent texture/dynamic-object boundaries.
Material runtime was reconsidered, but bindings, textures/LUTs, inputs and output oracle remain
absent. This is the explicit correctness cadence override, with no material runtime/performance claim.

## Proposed solution

Repair the four existing shared conversion helpers in `source/core/slang-math.h`. Subnormal
narrowing rounds on the destination's exact minimum-subnormal grid; a carry naturally becomes the
smallest normal encoding. Normal E4M3 exponent 15 is finite, so remove its erroneous clamp.
Subnormal widening uses the correct minimum-normal scale. Preserve all existing overflow, infinity,
NaN and sign policy outside these finite corrections. No NVVM backend support is admitted.

## Change summary

- The existing E4M3/E5M2 narrow/widen helpers now implement the finite representable-value contract.
- Three math units enumerate all 256 encodings per format, every finite value and every adjacent
  midpoint with neighboring Float32 inputs and both signs, plus explicit overflow-policy boundaries.
- One distinct discovery fixture tests 52 completely folded literal outputs in three modes.
- Plan, report, design, compact validation and census/addition artifacts retain the evidence.

The parent-reviewed source built successfully (758 steps), followed by the mandatory full checkpoint:

| Final-source gate               | Measured result                                                                             |
| ------------------------------- | ------------------------------------------------------------------------------------------- |
| Smoke / focused literals        | 4/4 and 3/3 pass; 52 exact output words per literal mode                                    |
| Actual shared-helper executable | 3548 cases; 220 before mismatches → 0 after                                                 |
| Dynamic Slang NVRTC replay      | 2 launches, 3036 records, 24290 complete words; zero mismatches                             |
| Units                           | 511 passed, one pre-existing Windows skip; includes all 27 math tests and 3 new FP8 tests   |
| Toolkit / runner contracts      | 18/18 and 6/6                                                                               |
| Frozen                          | 452 identities/1356 cells; 1345 correct, 11 unchanged unresolved                            |
| Discovery                       | 112 old identities/336 cells plus fixture 113/3 cells; 309 correct, 30 unchanged unresolved |
| Material                        | 6/6 compile/assembly cells; no runtime/performance claim                                    |

Every old classification, return_code, complete execution_counts, diagnostic and canonical_shape
matches accepted 242 exactly. Total 1695 fresh cells: 1654 correct and 41 unresolved; all 1651 old correct
cells and 16 resolved histories survive. The 3 new correct cells are additions, not a baseline reset.
The established FP8 backend rejections remain unchanged. All 560 old inputs are byte-identical.

The final identity includes 43 source paths, 12 artifacts and 561 runtime inputs, captured before every
gate and afterward. Compiler executable SHA256 `b9e87811c263f131cf1372b307bd60acdcc51993aaab2af91a0cfd03462d10a1`;
provider SHA256 `c0522674424c86dbc9444b2abc202c97146a6b41e3a9179d95d34ec9fe1b0773`. Seven artifacts change
through the shared-header rebuild; the provider binary remains identical. ABI 40, native Ubuntu 24.04,
L4 SM89 driver 580.126.09, target SM80, CUDA 12.9 and matching RelWithDebInfo remain unchanged.

See [validation244](runtime-validation.slice-244.json), [plan244](plan.slice-244-fp8-finite-producers.md),
[frozen census](census.slice-244.tsv) and [discovery census](discovery-census.slice-244.tsv).
Raw evidence remains under `build/nvvm-loop/slice-244-before` and `slice-244-after`.
Full checkpoint 244 is independently accepted, resetting the implementation cadence to zero.
The parent independently checked all 3548 helper cases, 156 fixture GPU words and 24290 dynamic
replay words using a rational oracle. Metadata review verified 716 compact / 1275 total evidence
references before its seven own references, all 58 indexed artifacts, exact old outcomes and
failure histories, and all 19 historical primary-source snapshots.

## Concepts and vocabulary

- **Subnormal grid:** E4M3 values are integer multiples of 2^-9, E5M2 of 2^-16 below the minimum normal.
- **Nearest/even:** select the closest representable value; an exact midpoint selects an even byte.
- **Producer:** `IRBuilder::getFloatValue` constructs the canonical rounded floating literal consumed
  by optimization and emitters. The shared host helpers determine its numeric value.
- **SATFINITE:** CUDA runtime overflow saturation differs from the shared helpers' established
  E4 NaN/E5 infinity policy. This change deliberately preserves that distinction.

## Process report

The production inventory contains two subnormal branches in existing narrowing helpers, removal of
one invalid E4 exponent clamp, and two corrected widening scales. There are no new production
helpers, fallbacks, descriptors, admission rules or alternate value representations. Test-only
`getFiniteFloat8Value` derives dyadic values from the format definition;
`checkFloat8FiniteConversions` searches nearest values by distance. This independently checks the
bit-based implementation instead of copying its arithmetic.

`SCCPContext::evalCast` receives canonical finite Float32 literal inputs and calls
`IRBuilder::getFloatValue`. For FP8 types, the builder calls FloatToFloatE4M3/FloatToFloatE5M2 followed
by FloatE4M3ToFloat/FloatE5M2ToFloat, preserving a target-rounded value in the existing IR literal.
That semantic source of truth is valid; no syntax needs reconstruction. Previously negative biased
subnormal exponents became unsigned, yielding invalid results, and exponent 15 E4 values all became 448.
Widening also used the wrong powers of two. SPIR-V emission and reflection call these same narrow
helpers, making shared producer repair necessary rather than an NVVM special case.

The E4 half-minimum guard handles inputs at or below 2^-10 as signed zero; the remaining subnormal
interval bounds the significand shift to 21..24. E5 uses half-minimum 2^-17 and shifts 22..24. Retained
significand, remainder and half-way parity implement RNE. For example, 15*2^-10 lies halfway between
E4 bytes 7 and 8, so the even byte 8 is naturally the minimum normal; E5's 7*2^-17 similarly rounds to 4.
Each widening uses its minimum normal, 2^-6 or 2^-14, divided by its fraction count 8 or 4. These are
exact Float32 operations, far from Float32 underflow. The early E4 range guard proves normal
rounding cannot produce a code beyond 126, so the exponent 15 clamp is unnecessary and wrong.

The existing E4 abs(input)>448 guard remains exact, including the immediate Float32 neighbor above 448.
E5 still rounds overflow to infinity at 61440 and retains its exact infinity/NaN encoding behavior.
CUDA runtime SATFINITE is neither imported nor used as the host-helper oracle. Questions about
out-of-range policy remain separate. Signed zero and existing widening NaN sign/payload mapping
are checked exactly. The host oracle uses enumeration and exact dyadic reasoning rather than the
production shift algorithm; its before/after cases and expected values are byte-identical.

The focused source cast pairs fold before direct preflight, leaving ordinary Float/UInt output
values. Therefore the fixture tests the actual shared producer through all three backends without
admitting FP8 transport, bitcasts or runtime casts. Current FP8 unsupported corpus boundaries must
remain unchanged. All old frozen/discovery identities and oracles remain preservation obligations.

The tested source base is 41f070c689cb46e91b3939ca038654b91ef96944; accepted 242 was tested from
base d6c26eb4cf5960ac07feff8158d15163cc2757fc. Intervening research 243 changed no production source.
Historical 243 raw LLVM controls remain inherited with their original artifact hashes. Final source
NVRTC dynamic controls were replayed against unchanged 1536/1500-record research grids. Historical
primary-source live hashes intentionally differ for the edited math header and math unit file; all 19 immutable
snapshots and accepted indices 238/240/241/242/243 are checked without altering their references.

No worker commit, push, driver/system change or reboot is authorized. Parent owns independent
acceptance and the local commit. Full 244 is now authoritative after successful independent review.
