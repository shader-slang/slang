# Establish the CUDA BF16 contract for direct NVVM

## Motivation

The frozen `hlsl-intrinsic/scalar-bf16.slang#cuda-1` remains correct under NVRTC and rejects
both direct modes with `helper function result type: BFloat16`. Consider its relevant code:

```slang
uint64_t bitsBF16 = 0x4080404040003F80;
let values = bit_cast<vector<BFloat16, 4>>(bitsBF16);
float4 expanded = float4(values);
BFloat16 three = BFloat16(3.0);
vector<BFloat16, 4> weights = vector<BFloat16, 4>(
    BFloat16(0.5), BFloat16(0.5), BFloat16(1.0), BFloat16(1.0));
BFloat16 result = dot(values, weights);
```

The source expects two-byte scalar storage, exact 1/2/3/4 expansion, bits 16448 for three, and
BF16 dot 8.5. Merely admitting the first helper type would not implement these semantics.
This research establishes the contract before a compiler change. BF16 ranks first for two measured
frozen failures and reusable scalar storage/conversion support. FP8, arbitrary RequirePrelude,
textures and the documented CUDA target-wide ignored column-major layout are separate candidates.
Material's six accepted 237 compile/assembly cells were reconsidered; missing binding, textures/LUT,
input and output oracle still prevent material runtime or performance work.

## Proposed solution

Implement a distinct semantic BF16 format at the NVVM compiler/provider boundary, with physical
i16 storage and format-specific conversions. Preserve the canonical Slang BF16 type. Existing
`FLOATING_POINT,16` is IEEE half, and the type-lowering cache infers `isFloat16` from width 16;
expanding that classifier would silently select half operations and layout. Native LLVM `bfloat`
is not usable through this installed libNVVM dialect: retained O0/O3 probes return 6 with
`parse expected type`. Isolated physical i16 helper/storage/conversion probes pass both modes.

This is a research handoff, not an implemented representation change. A next slice should bound
scalar BF16 constants, exact bit transport and Float32 conversions, explicitly negotiate the
semantic format through the provider ABI, and keep unimplemented constructor/dot/storage roles
rejected. Integer construction needs exact RNE, not ordinary int→FP32→BF16. Full frozen workload
acceptance additionally needs vector bit transport/casts and canonical `_slang_vector_dot`
recognition with the existing CUDA rounding order. Neither is claimed fixed here.

## Change summary

Only the completed plan, this report, [compact evidence](semantic-evidence.slice-238.json), STATUS
and appended design note change. Generated controls, LLVM, PTX, cubins, complete binary inputs,
expectations and outputs, independent rational oracle and audit/index remain under
`build/nvvm-loop/slice-238-bf16/`. No compiler/provider/ABI/frontend/library/test/corpus/runner edits,
compiler builds, commits, pushes or system changes were made.

| Fresh evidence                            | Result                                                                          |
| ----------------------------------------- | ------------------------------------------------------------------------------- |
| Identity                                  | 35 source, 12 artifacts, 557 inputs unchanged before/after against accepted 237 |
| GPU smoke                                 | 4/4                                                                             |
| Frozen scalar-bf16                        | 3 exact cells: NVRTC correct; direct O0/O3 unchanged preflight                  |
| CUDA and public Slang conversion controls | 73,190 records each; 365,950 checked outputs each                               |
| CUDA add/sub/mul/div control              | 676 selected pairs; 2,704 checked outputs                                       |
| Public Slang BF16 dot4                    | 679 cases; 679 checked outputs                                                  |
| Raw physical i16 NVVM controls            | 73,190 records each at O0/O3; 219,570 checked outputs each                      |
| Assembly / layout                         | 6/6 SM80 PTX assemblies; 10 scalar/vector size/alignment assertions             |
| Native bfloat feasibility                 | O0/O3 parser rejection retained, no execution                                   |

Six launches check 1,174,423 output words and preserve 3,531,423 input/sentinel words, for 4,705,846
complete returned words. All comparisons pass. Conversion inputs cover every BF16 encoding,
7,654 additional Float32 midpoint/extreme/NaN patterns, and 75 distinct signed integer cases.
The two constructor paths differ on 26 distinct integer inputs as independently predicted.

Full 237 remains immutable: 1,683 cells / 1,640 correct / 43 unresolved / 14 resolved histories. Only
three corpus cells are fresh here; the other 1,680 and units 481 plus one skip, toolkit 18,
runner 6, material 6 and prior clock replay are explicitly inherited. No support delta, new corpus
identity, full checkpoint or implementation-cadence increment. Latest targeted 233, full 237, zero
implementation slices since full. Native Ubuntu 24.04 / L4 SM89 / target 80 / driver 580.126.09,
CUDA 12.9.2 / NVRTC 12.9.86 / LLVM 14 / ABI 37 and matching RelWithDebInfo remain unchanged.

## Concepts and vocabulary

- **Semantic format:** BF16 has 8 exponent / 7 fraction bits; IEEE half has 5/10. Equal storage
  width does not make their arithmetic or conversion descriptors interchangeable.
- **Physical i16 transport:** exact 16-bit storage/register payload, still governed by BF16
  semantic operations; it does not turn BF16 values into source-language integers.
- **Double rounding:** an intermediate nearest-even FP32 result may land on a BF16 midpoint
  even when the exact integer was on one side of it.
- **Oracle:** integer/rational expected results established independently of CUDA execution.
  NaN expectations check classification; observed payloads are recorded separately.

## Process report

The helper/fallback inventory is empty for production code. The research contains a rational
oracle, bounded generated CUDA/Slang controls, raw LLVM controls and output checkers. Each survives
as evidence only. An initial oracle exponent-offset error failed its hand-derived assertion before
input generation or GPU use; the original is retained in `attempts/`, corrected before expectations
were written. There were no production fallback proposals, changed source oracles or moved
unsupported diagnostics counted as correctness.

`core.meta.slang` intentionally declares `BFloat16Type`, with `CastIntToFloat` for the integer
constructor and `FloatCast` for floating construction/expansion. `hlsl.meta.slang::dot` selects the
CUDA GenericAsm `_slang_vector_dot` for its BF16 vector overload. Final frozen IR retains BF16
constants, scalar bitcasts, makeVector, vector FloatCast and a BF16-returning canonical helper.
These are valid producer shapes. `slang-emit-nvvm.cpp` helper preflight rejects their result type;
`isNVVMSupportedFloatingPointScalarType`, `_getNVVMSemanticType`, type-lowering roles and the
provider's semantic type construction lack the distinct format. The producer needs no patch.

Reuse the current role-based type caches, storage traversal and descriptor validation, but audit
each format-dependent branch. `_emitNVVMHalfHelperABIReinterpretation` already separates semantic
Half from its physical i16 helper ABI; BF16 should share appropriate transport mechanics without
being classified as Half. CUDA header/static assertions give scalar size/alignment 2/2, BF2 = 4/4,
custom BF3 = 6/2 and custom BF4 = 8/2. The Half compact chunk path is not a valid blanket BF policy.
The raw prototype exercises noinline scalar i16 helper arguments/results and volatile 2-byte local
storage. It does not prove Slang aggregate/resource layout, which remains future acceptance work.

Installed `cuda_bf16.h/.hpp` is the concrete CUDA 12.9 implementation source. On target 80,
`__float2bfloat16_rn` emits `cvt.rn.bf16.f32`; expansion places the BF bits in Float32's upper word.
The exhaustive 65,536 transport/extension cases include all signaling/quiet NaNs, signed zeros,
subnormals, normals and infinities. Float32 narrowing checks both sides of nearest-even ties,
normal/subnormal transitions, overflow, underflow, signed zero and NaNs. CUDA's public conversion
contract promises nearest-even, signed zero/infinity preservation and NaN classification, not a
specific payload. All measured narrowing NaNs are 0x7fff; this remains an observation. Target 80
expansion preserves the exact NaN bits by construction; do not extend that payload claim to
SM90 `cvt.f32.bf16`, whose NaN result is unspecified.
[CUDA conversion contract](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-math-api/cuda_math_api/group__CUDA__MATH____BFLOAT16__MISC.html).

For integer 16842753 (`2^24+2^16+1`), direct nearest BF16 is 0x4b81. Nearest FP32 first yields
0x4b808000, and subsequent BF rounding gives 0x4b80. Both signed directions reproduce in the public
Slang control. CUDA's SM80 header deliberately converts int with directed rounding and adds a
sticky low bit when inexact, then narrows. The exact integer constructor is therefore a distinct
contract. Other source widths, double/half conversions and general BF-to-integer casts are not
runtime-qualified by this grid and must not be admitted through an unproven generic recipe.

CUDA operators delegate to `__hadd`, `__hsub`, `__hmul` and `__hdiv`. SM80 add/sub/mul use BF16
FMA with 1, -1 and negative zero respectively. This preserves rounded multiplication's signed zero;
using positive zero would not. The 26×26 arithmetic grid independently checks finite rounding,
subnormal preservation, cancellation, zeros, infinities and NaN classification. Division uses the
CUDA header's approximate FP32 division with scaling to avoid flushing subnormal results; the bounded grid is not exhaustive division proof.
The public dot4 control checks the frozen 8.5 case, rounding/cancellation counterexamples and 676
pairs of special patterns. Source prelude starts BF +0 and rounds every product and addition in lane
order. Generated PTX shows four product and four add BF16 FMA instructions. The two cancellation
cases return BF 0; Float32 accumulation or a fused product/add can give nonzero results. Do not replace
this with an unrestricted LLVM dot reduction. CUDA prelude owns only 2/3/4 overloads; source N0/N1
have separate branches.
[CUDA arithmetic contract](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-math-api/cuda_math_api/group__CUDA__MATH____BFLOAT16__ARITHMETIC.html).

BF16 FMA and Float32 narrowing are available at SM80; native BF16 add/sub/mul and broad
integer/BF16 conversions require SM90. Thus the actual L4 SM89 and target 80 cannot use the latter
instructions. The physical i16 prototype uses only the established SM80 narrowing instruction and
exact upper-word expansion; it assembles and matches the independent oracle at O0/O3. No universal
software NaN constant is introduced. `FloatToBFloat16` in `source/core/slang-math.h` has a useful
finite RNE path but retains/quietens NaN payload bits, unlike observed CUDA narrowing; reuse needs
an explicit format/target contract.
[PTX ISA](https://docs.nvidia.com/cuda/archive/12.9.1/parallel-thread-execution/index.html).

The related dynamic-dispatch source combines struct A's FP8 values and struct B's BF2. Its accepted 237
first direct diagnostic is `helper function result type: A`; source inspection confirms BF support
alone cannot resolve the workload. The CLI source-trace attempt lacks the runtime harness type conformances and fails with
E50100 (`no type conformances found`); that retained attempt is only early IR/source evidence,
not a fresh dynamic-dispatch corpus result. The independent FP8 boundary is recorded, not implemented
or freshly replayed. Material, texture and arbitrary prelude work remain separate. The research stops
at a precise provider-format/storage/conversion handoff, with full frozen BF16 acceptance still
requiring vector and dot support and a full checkpoint for shared type/provider changes.

Independent parent acceptance verified 125 unique evidence references, including every indexed
raw artifact, all 35 source/12 artifact/557 input hashes and 15 primary source hashes. The three
frozen cells retain exact outcomes. A separate oracle uses upper-word carry for FP32 rounding,
integer quotient/remainder, and exact rational nearest-neighbor search for arithmetic/dot. It
agrees with every stored expectation and all 4,705,846 returned words across six launches.
Research238 is accepted; latest full checkpoint remains 237 and implementation cadence is zero.
